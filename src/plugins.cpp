#include "plugins.h"
#include "core.h"
#include "libres.h"
#include "armhook.h"
#include "utils.h"
#include "strutils.h"
#include "pattern.h"
#include "mutex.h"
#include "memutils.h"

namespace plugins
{
	std::string storage_dir;
	std::string load_dir;

	std::map<uint16_t, void *>		opcodes;
	std::map<std::string, std::pair<std::string, void *> >	opcodeFuncs;

	const uint16_t OPCODE_FUNC = 0x1000;

	enum eGameIdent
	{
		GTA3,
		GTAVC,
		GTASA,
		GTALCS,
		GTAVCS
	};

	enum eGameVerInternal
	{
		VER_NONE,
		VER_GTA3_1_4,
		VER_GTAVC_1_03,
		VER_GTASA_1_00,
		VER_GTASA_1_01,
		VER_GTASA_1_02,
		VER_GTASA_1_03,
		VER_GTASA_1_05,
		VER_GTASA_1_06,
		VER_GTASA_1_05_GER,
		VER_GTASA_1_07,
		VER_GTA3_1_6,
		VER_GTAVC_1_06,
		VER_GTASA_1_08,
		VER_GTALCS_2_2,
		VER_GTA3_1_8_OR_HIGHER,
		VER_GTAVC_1_09_OR_HIGHER,
		VER_GTASA_2_00_OR_HIGHER,
		VER_GTALCS_2_4_OR_HIGHER,
		VER_GTALCS_PSP_1_05_OR_HIGHER,
		VER_GTAVCS_PSP_1_02_OR_HIGHER
	};

	struct cleo_ifs_t
	{
		// interface v1

		uint32_t (*GetInterfaceVersion)();	// currently = 1

		eGameIdent (*GetGameIdentifier)();
		eGameVerInternal (*GetGameVersionInternal)();

		const char *(*GetPackageName)();			// PSP: DiscId
		const char *(*GetPackageVersionStr)();		// PSP: DiscVersion
		uint32_t (*GetPackageVersionCode)();		// PSP: DiscVersionCode

		const char *(*GetCleoStorageDir)();			 
		const char *(*GetCleoPluginLoadDir)();		// PSP: same as StorageDir

		void (*PrintToCleoLog)(const char *str);

		const char *(*GetMainLibraryFileName)();	
		void *(*GetMainLibraryLoadAddress)();

		typedef struct { uint32_t rva, size; } section_t;

		uint32_t (*GetMainLibraryExecutableSections)(section_t *sections, uint32_t size);
		void *(*FindExecutablePattern)(const char *pattern, uint32_t index);

#ifdef ANDROID
		void *(*GetMainLibrarySymbol)(const char *name);
#endif

		void (*MemWriteArr)(void *addr, uint8_t *arr, uint32_t size, bool protect);

#ifdef ANDROID
		void (*ReplaceThumbCall)(void *addr, void *func_to);
		void (*HookThumbFunc)(void *func, uint32_t startSize, void *func_to, void **func_orig);
		void (*ReplaceArmCall)(void *addr, void *func_to);
		void (*HookArmFunc)(void *func, uint32_t startSize, void *func_to, void **func_orig);
#else
		void (*ReplaceMipsCall)(void *addr, void *func_to);
		void (*HookMipsFunc)(void *func, uint32_t startSize, void *func_to, void **func_orig);
#endif

		// ip is a pointer inside scriptHandle structure, the structure is different in all games
		typedef void (*opcode_handler_t)(void *scriptHandle, uint32_t *ip, uint16_t opcode, const char *name);

		bool (*RegisterOpcode)(uint16_t opcode, opcode_handler_t handler);
		bool (*RegisterOpcodeFunction)(const char *name, opcode_handler_t handler);

		typedef struct { union { int32_t i; uint32_t u; float f; }; } data_t;

		data_t *(*ReadParam)(void *scriptHandle); // returned pointer has the data until next param read call (points into game's ScriptParams)
		data_t *(*GetPointerToScriptVar)(void *scriptHandle); // returned pointer is valid all the time (points into game's script handle or into global seg)
		bool (*ReadString8byte)(void *scriptHandle, char *buf, uint32_t size);
		bool (*ReadStringLong)(void *scriptHandle, char *buf, uint32_t size);	// supported only in III/VC/SA (so far)

		bool (*IsParamListEnd)(uint32_t ip);
		void (*SkipParamListEnd)(uint32_t *ip);

		void *(*GetRealCodePointer)(uint32_t ip);	// script ip to ptr, script ip has to be gained from handle->ip as *ip (handler param)
		uint32_t (*GetIpUsingRealCodePointer)(void *realPtr);	// ptr to script ip
		void *(*GetRealLabelPointer)(void *scriptHandle, uint32_t offset);	// offset has to be represented in the raw way i.e. opcode label param encoding

		static uint32_t _GetInterfaceVersion()
		{
			return 1;
		}

		static eGameIdent _GetGameIdentifier()
		{
			return (eGameIdent)core::GetGame();
		}

		static eGameVerInternal _GetGameVersionInternal()
		{
			return (eGameVerInternal)core::GetGameVersion();
		}

		static const char *_GetPackageName()
		{
			return core::GetPackageName();
		}

		static const char *_GetPackageVersionStr()
		{
			return core::GetPackageVersionStr();
		}

		static uint32_t _GetPackageVersionCode()
		{
			return core::GetPackageVersionCode();
		}

		static const char *_GetCleoStorageDir()
		{
			return storage_dir.c_str();
		}

		static const char *_GetCleoPluginLoadDir()
		{
			return load_dir.c_str();
		}

		static void _PrintToCleoLog(const char *str)
		{
			utils::log((std::string("@ ") + str).c_str());
		}

		static const char *_GetMainLibraryFileName()
		{
			return libres::getLibFileName();
		}

		static void *_GetMainLibraryLoadAddress()
		{
			return libres::getLoadAddress();
		}

		static uint32_t _GetMainLibraryExecutableSections(section_t *sections, uint32_t size)
		{
			const std::vector<libres::section_addr_space_t> vec = libres::getExecutableSections();
			uint32_t res = 0;
			for (uint32_t i = 0; i < size && i < vec.size(); i++)
			{
				sections[i].rva = vec[i].addr;
				sections[i].size = vec[i].size;
				res++;
			}
			return res;
		}

		static void *_FindExecutablePattern(const char *pattern, uint32_t index)
		{
			void *res;
			return __FindPatternAddressCompact(res, pattern, index) ? res : NULL;
		}

#ifdef ANDROID
		static void *_GetMainLibrarySymbol(const char *name)
		{
			return libres::getsym(name);
		}
#endif

		static void _MemWriteArr(void *addr, uint8_t *arr, uint32_t size, bool protect)
		{
			memutils::mem_write_arr(cast<uint8_t *>(addr), arr, size, protect);
		}

#ifdef ANDROID
		static void _ReplaceThumbCall(void *addr, void *func_to)
		{
			armhook::replace_thumb_call(cast<ptr>(addr), cast<ptr>(func_to));
		}
			 
		static void _HookThumbFunc(void *func, uint32_t startSize, void *func_to, void **func_orig)
		{
			armhook::hook_thumb_func(func, startSize, func_to, func_orig);
		}
			 
		static void _ReplaceArmCall(void *addr, void *func_to)
		{
			armhook::replace_arm_call(cast<ptr>(addr), cast<ptr>(func_to));
		}
			 
		static void _HookArmFunc(void *func, uint32_t startSize, void *func_to, void **func_orig)
		{
			armhook::hook_arm_func(func, startSize, func_to, func_orig);
		}
#else
		static void _ReplaceMipsCall(void *addr, void *func_to)
		{
			armhook::replace_mips_call(cast<ptr>(addr), cast<ptr>(func_to));
		}
			 
		static void _HookMipsFunc(void *func, uint32_t startSize, void *func_to, void **func_orig)
		{
			armhook::hook_mips_func(func, startSize, func_to, func_orig);
		}
#endif

		static bool _RegisterOpcode(uint16_t opcode, opcode_handler_t handler)
		{
			CS_SCOPE(mutex::mplugin);

			if (!handler || opcodes.find(opcode) != opcodes.end())
				return false;
			opcodes[opcode] = cast<void *>(handler);
			return true;
		}

		static bool _RegisterOpcodeFunction(const char *name, opcode_handler_t handler)
		{
			CS_SCOPE(mutex::mplugin);

			if (!name || !strlen(name) || !handler)
				return false;

			std::string str = strutils::str_to_lower(name);
			if (opcodeFuncs.find(str) != opcodeFuncs.end())
				return false;

			opcodeFuncs[str] = std::pair<std::string, void *>(name, cast<void *>(handler));
			return true;
		}

		static data_t *_ReadParam(void *scriptHandle)
		{
			core::CRunningScript__CollectParameters(cast<ptr>(scriptHandle), 1);
			return cast<data_t *>(&core::GetScriptParams()[0]);
		}

		static data_t *_GetPointerToScriptVar(void *scriptHandle)
		{
			return cast<data_t *>(core::CRunningScript__GetPointerToScriptVariable(cast<ptr>(scriptHandle), 0));
		}

		static bool _ReadString8byte(void *scriptHandle, char *buf, uint32_t size)
		{
			if (!buf || !size)
				return false;
			memset(buf, 0, size);
			std::string str;
			if (!core::read_str_8byte(cast<ptr>(scriptHandle), str))
				return false;
			const char *pstr = str.c_str();
			int i = 0;
			while (*pstr && i < size - 1)
			{
				buf[i] = *pstr;
				i++;
				pstr++;
			}
			return true;
		}

		static bool _ReadStringLong(void *scriptHandle, char *buf, uint32_t size)
		{
			if (!buf || !size)
				return false;
			memset(buf, 0, size);
			std::string str;
			if (!core::read_str_long(cast<ptr>(scriptHandle), str))
				return false;
			const char *pstr = str.c_str();
			int i = 0;
			while (*pstr && i < size - 1)
			{
				buf[i] = *pstr;
				i++;
				pstr++;
			}
			return true;
		}

		static bool _IsParamListEnd(uint32_t ip)
		{
			return *core::get_real_code_ptr(ip) == 0;
		}

		static void _SkipParamListEnd(uint32_t *ip)
		{
			if (*core::get_real_code_ptr(*ip) == 0)
				*ip += 1;
		}

		static void *_GetRealCodePointer(uint32_t ip)
		{
			return core::get_real_code_ptr(ip);
		}

		static uint32_t _GetIpUsingRealCodePointer(void *realPtr)
		{
			return core::get_ip_using_real_code_ptr(cast<ptr>(realPtr));
		}

		static void *_GetRealLabelPointer(void *scriptHandle, uint32_t offset)
		{
			return core::get_real_label_ptr(cast<ptr>(scriptHandle), offset);
		}

		#define func(x) x(_##x)
		cleo_ifs_t() :
			func(GetInterfaceVersion),
			func(GetGameIdentifier),
			func(GetGameVersionInternal),
			func(GetPackageName),
			func(GetPackageVersionStr),
			func(GetPackageVersionCode),
			func(GetCleoStorageDir),
			func(GetCleoPluginLoadDir),
			func(PrintToCleoLog),
			func(GetMainLibraryFileName),
			func(GetMainLibraryLoadAddress),
			func(GetMainLibraryExecutableSections),
			func(FindExecutablePattern),
#ifdef ANDROID
			func(GetMainLibrarySymbol),
#endif
			func(MemWriteArr),
#ifdef ANDROID
			func(ReplaceThumbCall),
			func(HookThumbFunc),
			func(ReplaceArmCall),
			func(HookArmFunc),
#else
			func(ReplaceMipsCall),
			func(HookMipsFunc),
#endif
			func(RegisterOpcode),
			func(RegisterOpcodeFunction),
			func(ReadParam),
			func(GetPointerToScriptVar),
			func(ReadString8byte),
			func(ReadStringLong),
			func(IsParamListEnd),
			func(SkipParamListEnd),
			func(GetRealCodePointer),
			func(GetIpUsingRealCodePointer),
			func(GetRealLabelPointer)
		{}
		
	} plugin_ifs;

	bool handle_opcode(void *scriptHandle, uint32_t *ip, uint16_t opcode)
	{
		if (opcodes.find(opcode) != opcodes.end())
		{
			cast<cleo_ifs_t::opcode_handler_t>(opcodes[opcode])(scriptHandle, ip, opcode, "");
			return true;
		}
		if (opcode == OPCODE_FUNC)
		{
			std::string str;
			core::e_game game = core::GetGame();
			if (game == core::GTA3 || game == core::GTAVC || game == core::GTASA)
			{
				if (!core::read_str_long(cast<ptr>(scriptHandle), str))
				{
					utils::log("1000: func name isn't defined");
					exit(1);
				}
			} else
			{
				if (!core::read_str_long(cast<ptr>(scriptHandle), str))
				{
					bool read = true;
					while (read)
					{
						std::string str_short;
						if (!core::read_str_8byte(cast<ptr>(scriptHandle), str_short))
						{
							utils::log("1000: func name has to be a set of 8byte strings"); // works only in VCS
							exit(1);
						}						
						for (int i = 0; i < str_short.size(); i++)
						{
							if (str_short[i] == '(')
							{
								read = false;
								break;
							} else
							{
								str += str_short[i];
								if (str.size() > 64)
								{
									utils::log("1000: func name has more than 64 chars (limit while using 8byte strings) '%s'", str.c_str());
									exit(1);
								}
							}
						}
					}
				}
			}
			size_t pos = str.find('(');
			if (pos != -1)
				str.erase(pos);
			std::string str_lower = strutils::str_to_lower(str);
			if (opcodeFuncs.find(str_lower) != opcodeFuncs.end())
			{
				std::pair<std::string, void *> func = opcodeFuncs[str_lower];
				cast<cleo_ifs_t::opcode_handler_t>(func.second)(scriptHandle, ip, opcode, func.first.c_str());
			} else
			{
				utils::log("1000: func '%s' not found", str.c_str());
				exit(1);
			}
			return true;
		}
		return false;
	}

#ifdef ANDROID

	void for_each_plugin(std::string dir, void (*file_cb)(std::string, std::string))
	{
		std::vector<std::string> file_names;
		if (utils::list_files_in_dir(dir, file_names))
			for (int i = 0; i < (int)file_names.size(); i++)
				if (strutils::get_ext(file_names[i]) == "so")
					file_cb(dir, file_names[i]);
	}

	void remove_plugin(std::string dir, std::string fname)
	{
		utils::log("deleting %s", (dir + fname).c_str());
		remove((dir + fname).c_str());
	}

	void copy_plugin(std::string dir, std::string fname)
	{
		std::string file_from = dir + fname;
		std::string file_to = load_dir + fname;
		uint32_t size;
		if (uint8_t *buf = utils::load_binary_file(file_from, size))
		{
			if (size)
			{
				if (FILE *f = fopen(file_to.c_str(), "wb"))
				{
					utils::log("copying %s -> %s", file_from.c_str(), file_to.c_str());
					fwrite(buf, 1, size, f);
					fclose(f);
				}
			}
			free(buf);
		}
	}

	void load_plugin(std::string dir, std::string fname)
	{
		utils::log("plugin is loading '%s'", (dir + fname).c_str());
		void *lib = dlopen((dir + fname).c_str(), RTLD_NOW);
		utils::log("handle %08x", lib);
		#pragma message "todo"
		if (lib)
			if (void (*plugin_init)(void *) = (void (*)(void *))dlsym(lib, "plugin_init"))
			{
				plugin_init(&plugin_ifs);
				utils::log("plugin loaded successfully");
				return;
			}
		utils::log("plugin loading failed");
	}

	void init(std::string storage_dir, std::string load_dir)
	{
		plugins::storage_dir = strutils::path_normalize(storage_dir, strutils::tspMake);
		plugins::load_dir = strutils::path_normalize(load_dir, strutils::tspMake);

		for_each_plugin(plugins::load_dir, remove_plugin);
		for_each_plugin(plugins::storage_dir, copy_plugin);
		for_each_plugin(plugins::load_dir, load_plugin);
	}

#else

	char plugin_ifs_as_arg[32];
	char *args[] = { plugin_ifs_as_arg };

	void load_plugin(std::string dir, std::string fname)
	{
		utils::log("plugin is loading '%s'", (dir + fname).c_str());
		bool loaded = pspSdkLoadStartModuleWithArgs((dir + fname).c_str(), 1, 1, args) >= 0;
		utils::log(loaded ? "plugin loaded successfully" : "plugin loading failed");
	}

	void init(std::string storage_dir, std::string load_dir)
	{
		plugins::storage_dir = storage_dir;
		plugins::load_dir = load_dir;

		sprintf(plugin_ifs_as_arg, "%08X", cast<uint32_t>(&plugin_ifs));

		std::vector<std::string> file_names;
		if (utils::list_files_in_dir(storage_dir, file_names))
			for (int i = 0; i < (int)file_names.size(); i++)
				if (strutils::get_ext(file_names[i]) == "prx")
					load_plugin(storage_dir, file_names[i]);
	}

#endif

}
