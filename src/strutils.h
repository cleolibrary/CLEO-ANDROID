#pragma once

#include "common.h"

namespace strutils
{
	std::string ansi_from_wstr(uint16_t *w);
	wide_string wstr_from_ansi(const char *a);
	void wstr_from_ansi(uint16_t *w, const char *a);

	std::string str_to_lower(std::string str);

	uint32_t str_hash(std::string astr);

	std::string str_replace(std::string subject, const std::string& search, const std::string& replace);
	std::string replace_slashes(std::string str);

	enum eTrailingSlashPolicy
	{
		tspDontCare,
		tspMake,
		tspClear
	};

	std::string path_normalize(std::string path, eTrailingSlashPolicy tsp = tspDontCare);

	std::string get_ext(std::string path, bool lower = true);
}