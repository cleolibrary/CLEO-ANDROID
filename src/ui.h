#pragma once

#include "common.h"
#include "core.h"

namespace ui
{
	void init();

	void show_arrow();
	void hide_arrow();
	void create_menu(std::string name, std::string closename, std::vector<wide_string> &items);
	int32_t get_menu_touched_item_index(uint32_t maxtime);
	void delete_menu();
	void set_menu_active_item_index(int32_t index);
	int32_t get_menu_active_item_index();
	
	void handle_touch(float x, float y);
}