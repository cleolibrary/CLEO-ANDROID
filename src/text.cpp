#include "text.h"
#include "core.h"
#include "utils.h"
#include "strutils.h"

using namespace strutils;

namespace text
{
	const char CSI_ENTRY[] = "CSI_%d";

	std::map<uint32_t, uint16_t *> gxt_entries;
	
	void init()
	{
		for (std::map<uint32_t, uint16_t *>::iterator i = gxt_entries.begin(); i != gxt_entries.end(); i++)
			delete[] i->second;
		gxt_entries.clear();
		// startup string replacement shown in GTA3 and GTAVC
		add_gxt_entry("SPLASH", VERSION_STR);
		// script menu
		add_gxt_entry("CLDSC", VERSION_STR "~n~" COPYRIGHT "~n~" WEBPAGE); // SA, LCS, VCS PSP
		add_gxt_entry("CLDSCL", core::GetGame() == core::GTA3
			? CLEO_ANDROID "               " VERSION_DATE "                   " COPYRIGHT "          " WWW_WEBPAGE	// III
			: CLEO_ANDROID "~n~" VERSION_DATE "~n~" COPYRIGHT "~n~" WWW_WEBPAGE);	// VC
		add_gxt_entry("CLMNU", "cleo menu");
		add_gxt_entry("CLMNUD", "Touch screen up or down to select a script, center to start it");
		add_gxt_entry("CLMNUN", "You have no scripts for menu, are you sure the game has STORAGE access permission?");
		add_gxt_entry("CLMNU0", "No such script !");
		add_gxt_entry("CLMNU1", "Script is already running !");
		add_gxt_entry("CLMNU2", "<CLEO MENU > Touch screen left or right to select script, center to start, any other screen part to dismiss");
		add_gxt_entry("CLMNUL", "LEFT");
		add_gxt_entry("CLMNUT", "UP");
		add_gxt_entry("CLMNUB", "DOWN");
		add_gxt_entry("CLMNUR", "RIGHT");
		add_gxt_entry("CLMNUS", "DISMISS");
		for (int32_t i = 0; i < 108; i++)
		{
			char str[16];
			sprintf(str, CSI_ENTRY, i);
			add_gxt_entry(str, "-");
		}
	}

	void add_gxt_entry(std::string name, std::string str)
	{
	    uint16_t *w = new uint16_t[str.length() + 1];
	    uint32_t hash = str_hash(name);
		wstr_from_ansi(w, str.c_str());
		gxt_entries[hash] = w;
	}

	void set_gxt_invokable_script_name(uint32_t num, std::string str)
	{
		if (gxt_entries.size())
		{
			char name[64];
			sprintf(name, CSI_ENTRY, num);
			uint32_t hash = str_hash(name);
			if (gxt_entries.find(hash) != gxt_entries.end())
			{
				str = str_to_lower(str);
				str = str_replace(str, "gta3", "");	
				str = str_replace(str, "gtavcs", "");
				str = str_replace(str, "gtavc", "");
				str = str_replace(str, "gtasa", "");
				str = str_replace(str, "gtalcs", "");				
				str = str_replace(str, "csi", "");
				str = str_replace(str, ".", "");
				str = str_replace(str, "_", " ");
				uint16_t *w = new uint16_t[str.length() + 1];
				wstr_from_ansi(w, str.c_str());
				delete[] gxt_entries[hash];
				gxt_entries[hash] = w;
			}
		}
	}

	uint16_t *get_gxt_entry(std::string name)
	{
		uint32_t hash = str_hash(name);
		if (gxt_entries.find(hash) != gxt_entries.end())
			return gxt_entries[hash];
		return NULL;
	}

	void load_gxt_entries_from_text(LPSTR text, uint32_t size)
	{
		// read all lines
		std::string line;
		int32_t i = 0;
		while (i < size)
		{
			if (text[i] == '\r' || text[i] == '\n' || i == size - 1)
			{
				if (i == size - 1)
					line += text[i];
				if (line.size() > 3)
				{
					char *str = (char *)line.c_str();
					if (str[0] >= 'A' && str[0] <= 'z')
					{
						char *space = strstr(str, "\x20");
						if (space != NULL && str < space)
						{
							std::string e_name;
							while (str < space)
							{
								e_name += *str;
								str++;
							}
							str++;
							if (line.size() > e_name.size() + 1)
							{
								//utils::log("'%s' '%s'", e_name.c_str(), str);
								add_gxt_entry(e_name, std::string(str));
							}
						}
					}
					line.clear();
				}
			} else
			{
				line += text[i];
			}
			i++;
		}
	}
}