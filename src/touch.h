#pragma once

#include "common.h"

#ifdef ANDROID

enum e_touch
{
	TOUCH_NONE,
	TOUCH_UP,
	TOUCH_DOWN,
	TOUCH_HOLD,
	TOUCH_MAX
};

#else

enum ePspControl
{
	CTRL_SELECT,
	CTRL_UP,
	CTRL_RIGHT,
	CTRL_DOWN,
	CTRL_LEFT,
	CTRL_LTRIGGER,
	CTRL_RTRIGGER,
	CTRL_TRIANGLE,
	CTRL_CIRCLE,  
	CTRL_CROSS,   
	CTRL_SQUARE,
	CTRL_HOLD,

	CTRL_STICK_UP,
	CTRL_STICK_DOWN,
	CTRL_STICK_RIGHT,
	CTRL_STICK_LEFT,

	CTRL_SIZE
};

#endif

namespace touch
{
#ifdef ANDROID
	void touch_event(e_touch type, uint32_t num, uint32_t x, uint32_t y, uint32_t w, uint32_t h);
	bool point_touched(uint32_t p);
	bool point_touched_timed(uint32_t p, uint32_t mintime);
	bool point_slide_done(uint32_t p_from, uint32_t p_to, uint32_t mintime, uint32_t maxtime);
#else
	void psp_input_event(SceCtrlData *pad_data);
	bool psp_control_pressed(ePspControl control);
	bool psp_control_pressed_timed(ePspControl control, uint32_t mintime);
	void psp_control_disable(ePspControl control, uint32_t mintime);
#endif
	void menu_button_event(bool state);
	bool menu_button_pressed();
	bool menu_button_pressed_timed(uint32_t mintime);
}
