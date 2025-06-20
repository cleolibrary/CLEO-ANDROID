#pragma once

#include "common.h"

namespace utils
{
	uint32_t get_tick_count();

	void log(const char *fmt, ...);
	
	bool list_files_in_dir(std::string dir, std::vector<std::string> &files);
	uint8_t *load_binary_file(std::string filename, uint32_t &size);
}