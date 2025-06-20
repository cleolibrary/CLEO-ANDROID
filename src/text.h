#pragma once

#include "common.h"

namespace text
{
	void init();
	void add_gxt_entry(std::string name, std::string str);
	void set_gxt_invokable_script_name(uint32_t num, std::string str);
	uint16_t *get_gxt_entry(std::string name);
	void load_gxt_entries_from_text(LPSTR text, uint32_t size);
}
