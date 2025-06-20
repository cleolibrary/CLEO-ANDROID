#pragma once

#include "common.h"

enum e_touch
{
	TOUCH_NONE,
	TOUCH_UP,
	TOUCH_DOWN,
	TOUCH_HOLD,
	TOUCH_MAX
};

namespace touch
{
    void touch_event(e_touch type, uint32_t num, uint32_t x, uint32_t y, uint32_t w, uint32_t h);
	bool point_touched(uint32_t p);
	bool point_touched_timed(uint32_t p, uint32_t mintime);
	bool point_slide_done(uint32_t p_from, uint32_t p_to, uint32_t mintime, uint32_t maxtime);
	void menu_button_event(bool state);
	bool menu_button_pressed();
	bool menu_button_pressed_timed(uint32_t mintime);
}