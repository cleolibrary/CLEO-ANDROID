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
}