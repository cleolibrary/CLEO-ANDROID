#include "libres.h"
#include "utils.h"

using namespace core;

// resolves main game library symbols
namespace libres
{
	void *loadAddress = NULL;
	void *getLoadAddress()
	{
		return loadAddress;
	}	

	std::vector<section_addr_space_t> executableSections;
	const std::vector<section_addr_space_t> &getExecutableSections()
	{
		return executableSections;
	}

	std::string libFileName;
	const char *getLibFileName()
	{
		return libFileName.c_str();
	}

#ifdef ANDROID

	bool init(e_game &game, int32_t &image_base)
	{
		// get game lib handle
		LPCSTR libs[] = {"libR1.so", "libGTAVC.so", "libGTASA.so", "libGTALcs.so"};

		// Android4/5 -- works
		// Android6/7 -- doesn't work
		//for (int32_t i = 0; i < sizeof(libs) / sizeof(libs[0]) && !lib; i++)
		//	lib = dlopen(libs[i], RTLD_LAZY);
		//if (lib)
		//	return true;

		// if lib handle can't be aquired using dlopen then use custom resolver that parses /proc/%pid%/maps
		// works on all versions of Android
		char str[4096];
		FILE *f = fopen("/proc/self/maps", "rt");
		while(fgets(str, sizeof(str), f) && !loadAddress)
		{
			uint32_t memstart, memend, unk1, unk2, unk3, unk4;
			char attr[1024], path[1024];
			int count = sscanf(str, "%x-%x %s %x %x:%x %d %s", &memstart, &memend, attr, &unk1, &unk2, &unk3, &unk4, path);
			if (count == 8)
				for (int32_t i = 0; i < sizeof(libs) / sizeof(libs[0]); i++)
					if (strstr(path, libs[i]))
					{
						loadAddress = cast<void *>(memstart);
						libFileName = path;
						game = (e_game)i;
						image_base = memstart;
						break;
					}
		}
		fclose(f);
		if (!loadAddress)
			return false;
		extern bool loadElfInfo(const char *fname);
		return loadElfInfo(libFileName.c_str());
	}

	std::map<std::string, uint32_t> exports;

	void *getsym(LPCSTR name)
	{
		// use exports from custom resolver
		if (exports.find(name) != exports.end())
			return cast<void *>(cast<uint32_t>(loadAddress) + exports[name]);
		return NULL;
	}

	typedef uint16_t Elf_Half;
	typedef uint32_t Elf_Word;
	typedef int32_t  Elf_Sword;
	typedef uint64_t Elf_Xword;
	typedef int64_t  Elf_Sxword;

	typedef uint32_t Elf32_Addr;
	typedef uint32_t Elf32_Off;
	typedef uint64_t Elf64_Addr;
	typedef uint64_t Elf64_Off;

	const int EI_NIDENT = 16;
	const int SHT_SYMTAB = 2;
	const int SHT_DYNSYM = 11;
	const int SHF_EXECINSTR = 4;

	#pragma pack(push, 1)

	struct Elf32_Ehdr
	{
		unsigned char e_ident[EI_NIDENT];
		Elf_Half e_type;
		Elf_Half e_machine;
		Elf_Word e_version;
		Elf32_Addr e_entry;
		Elf32_Off e_phoff;
		Elf32_Off e_shoff;
		Elf_Word e_flags;
		Elf_Half e_ehsize;
		Elf_Half e_phentsize;
		Elf_Half e_phnum;
		Elf_Half e_shentsize;
		Elf_Half e_shnum;
		Elf_Half e_shstrndx;
	};

	struct Elf32_Shdr
	{
		Elf_Word sh_name;
		Elf_Word sh_type;
		Elf_Word sh_flags;
		Elf32_Addr sh_addr;
		Elf32_Off sh_offset;
		Elf_Word sh_size;
		Elf_Word sh_link;
		Elf_Word sh_info;
		Elf_Word sh_addralign;
		Elf_Word sh_entsize;
	};

	struct Elf32_Sym
	{
		Elf_Word st_name;
		Elf32_Addr st_value;
		Elf_Word st_size;
		unsigned char st_info;
		unsigned char st_other;
		Elf_Half st_shndx;
	};

	#pragma pack(pop)

	bool loadElfInfo(const char *fname)
	{
		// load file
		uint32_t size;
		void *buf = utils::load_binary_file(fname, size);
		if (!buf)
			return false;
		// custom resolver using load address
		uint32_t addr = cast<uint32_t>(buf);
		// header
		Elf32_Ehdr header;
		memcpy(&header, cast<void *>(addr), sizeof(header));
		// sections
		Elf32_Shdr sections[header.e_shnum];
		//LPCSTR sectionNames[header.e_shnum];
        int entry_size = header.e_shentsize;
        int num        = header.e_shnum;
        int offset     = header.e_shoff;
        for (int i = 0; i < num; i++)
        	memcpy(&sections[i], cast<void *>(addr + offset + i * entry_size), sizeof(Elf32_Shdr));
        //int shstrndx = header.e_shstrndx;
        //for (int i = 0; i < num; i++)
        //	utils::log(cast<LPCSTR>(addr + sections[shstrndx].sh_offset + sections[i].sh_name));
		// executable sections
		for (int i = 0; i < num; i++)
			if (sections[i].sh_flags & SHF_EXECINSTR)
			{
				section_addr_space_t addr_space;
				addr_space.addr = sections[i].sh_addr;
				addr_space.size = sections[i].sh_size;
				executableSections.push_back(addr_space);
			}
        // symbols
        for (int i = 0; i < num; i++)
        {
        	Elf32_Shdr &section = sections[i];
        	if (section.sh_type == SHT_SYMTAB || section.sh_type == SHT_DYNSYM)
        	{
        		Elf32_Shdr &str_section = sections[section.sh_link];
        		int sym_count = (section.sh_entsize > 0) ? section.sh_size / section.sh_entsize : 0;
        		for (int j = 0; j < sym_count; j++)
        		{
        			Elf32_Sym *sym = cast<Elf32_Sym *>(addr + section.sh_offset + j * section.sh_entsize);
        			LPCSTR symname = cast<LPCSTR>(addr + str_section.sh_offset + sym->st_name);
					if (strlen(symname))
						exports[symname] = sym->st_value;
        		}
        	}
        }
		free(buf);
        return exports.size() > 0;
	}

#else

	uint32_t gpValue = 0;
	uint32_t getGpValue()
	{
		return gpValue;
	}	

	std::string disc_id;
	const char *getDiscId()
	{
		return disc_id.c_str();
	}

	std::string disc_version;
	const char *getDiscVersion()
	{
		return disc_version.c_str();
	}
	
	uint32_t disc_version_code;
	uint32_t getDiscVersionCode()
	{
		return disc_version_code;
	}

	struct SFO_Header
	{
		u32 magic; /* Always PSF */
		u32 version; /* Usually 1.1 */
		u32 key_table_start; /* Start position of key_table */
		u32 data_table_start; /* Start position of data_table */
		u32 index_table_entries; /* Number of entries in index_table*/
	};

	struct SFO_IndexTable
	{
		u16 key_table_offset; /* Offset of the param_key from start of key_table */
		u16 param_fmt; /* Type of data of param_data in the data_table */
		u32 param_len; /* Used Bytes by param_data in the data_table */
		u32 param_max_len; /* Total bytes reserved for param_data in the data_table */
		u32 data_table_offset; /* Offset of the param_data from start of data_table */
	};

	bool init(e_game &game, int32_t &image_base)
	{		
		SceUID modules[10];
		int count = 0;
		if (sceKernelGetModuleIdList(modules, sizeof(modules), &count) >= 0)
		{
			SceKernelModuleInfo info;
			for (int i = 0; i < count; ++i)
			{
				info.size = sizeof(SceKernelModuleInfo);
				if (sceKernelQueryModuleInfo(modules[i], &info) < 0)
					continue;

				if (strcmp(info.name, "GTA3") == 0)
				{
					loadAddress = cast<void *>(info.text_addr);					
					image_base = info.text_addr;
					section_addr_space_t addr_space;
					addr_space.addr = 0;
					addr_space.size = info.text_size;
					executableSections.push_back(addr_space);
					gpValue = info.gp_value;
					break;
				}
			}
		}
		if (!loadAddress)
			return false;

		// load file
		uint32_t size;
		void *buf = utils::load_binary_file("disc0:/PSP_GAME/PARAM.SFO", size);
		if (!buf || size < 8)
			return false;

		uint32_t addr = cast<uint32_t>(buf);
		SFO_Header *header = cast<SFO_Header *>(addr);
		if (header->magic != 0x46535000 || header->version != 0x00000101)
			return false;
		SFO_IndexTable *index = cast<SFO_IndexTable *>(addr + sizeof(SFO_Header));

		std::string title;
		for (int i = 0; i < header->index_table_entries; i++)
		{
			if (index[i].param_fmt != 0x0204)
				continue;
			std::string name = cast<char *>(addr + header->key_table_start + index[i].key_table_offset);
			char *val = cast<char *>(addr + header->data_table_start + index[i].data_table_offset);
			if (name == "DISC_ID") disc_id = val; else
			if (name == "DISC_VERSION") disc_version = val; else
			if (name == "TITLE") title = val;
		}		
		free(buf);
		if (disc_id.empty() || disc_version.empty())
			return false;

		uint32_t disk_ver[2] = { 0, 0 };
		sscanf(disc_version.c_str(), "%d.%d", &disk_ver[0], &disk_ver[1]);
		disc_version_code = ((disk_ver[0] & 0xFF) << 8) | (disk_ver[1] & 0xFF);
		if (!disc_version_code)
			return false;
		
		if (title == "GTA: Liberty City Stories" || title == "Grand Theft Auto: Liberty City Stories") game = GTALCS; else
		if (title == "Grand Theft Auto: Vice City Stories") game = GTAVCS; else
			return false;

		libFileName = "disc0:/PSP_GAME/SYSDIR/EBOOT.BIN";

		return true;
	}

#endif
}

























