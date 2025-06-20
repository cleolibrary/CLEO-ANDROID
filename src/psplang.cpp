#include "psplang.h"
#include "core.h"
#include "strutils.h"

namespace psplang
{
	bool bInit = false;	

	bool is_init()
	{
		return bInit;
	}

	bool bVcsRus1 = false;

	bool is_vcs_rus1()
	{
		return bVcsRus1;
	}

	bool bLcsRus1 = false;

	bool is_lcs_rus1()
	{
		return bLcsRus1;
	}

	void init()
	{
		if (bInit)
			return;

		uint16_t *lang_loading_str = core::GetGxtEntry("LOADING", false);

		if (!lang_loading_str)
			return;

		switch (core::GetGame())
		{
			case core::GTALCS:
			{
				if (*lang_loading_str == 0x0033)
					bLcsRus1 = true;
				break;
			}
			case core::GTAVCS:
			{
				if (*lang_loading_str == 0x0033)
					bVcsRus1 = true;
				break;
			}
		}

		bInit = true;
	}

	std::string localize(std::string str)
	{
		if (!bInit)
			return str;

		if (bVcsRus1 || bLcsRus1)
		{
			for (int i = 0; i < str.size(); i++)
			{
				char c = str[i];
				if (c >= 'A' && c <= 'Z')
					c += 0x20;
				str[i] = c;
			}

			str = strutils::str_replace(str, "cleo android", "Cleo Android");
			str = strutils::str_replace(str, "(c) alexander blade", "(C) Alexander Blade");
			if (bLcsRus1)
				str = strutils::str_replace(str, "(C)", "     (C)");
		}

		return str;
	}
}