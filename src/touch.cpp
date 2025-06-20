#include "touch.h"
#include "utils.h"
#include "ui.h"

namespace touch
{
    #define TOUCH_EXIST_TIME 150
    #define SLIDE_EXIST_TIME TOUCH_EXIST_TIME

	uint32_t touch_points[10] = {};
	struct _state_point {uint32_t p, t;} touch_point_using_state[2][4] = {};

	void touch_event(e_touch type, uint32_t num, uint32_t x, uint32_t y, uint32_t w, uint32_t h)
	{
		x = (float)x / w * 100;
		y = (float)y / h * 100;

		uint32_t p = 0;
		if (x >= 0 && x < 33)
		{
			if (y >= 0 && y < 33) p = 1; else
			if (y >= 33 && y <= 66) p = 2; else
				p = 3;
		} else
		if (x >= 33 && x <= 66)
		{
			if (y >= 0 && y < 33) p = 4; else
			if (y >= 33 && y <= 66) p = 5; else
				p = 6;
		} else
		{
			if (y >= 0 && y < 33) p = 7; else
			if (y >= 33 && y <= 66) p = 8; else
				p = 9;
		}

		uint32_t t = utils::get_tick_count();
		touch_points[p] = t;
		if (num < 2 && type < TOUCH_MAX)
		{
			touch_point_using_state[num][type].p = p;
			touch_point_using_state[num][type].t = t;
		}

		if (type == TOUCH_DOWN)
			ui::handle_touch((float)x / 100, (float)y / 100);
	}

	bool point_touched(uint32_t p)
	{
		return touch_points[p] + TOUCH_EXIST_TIME > utils::get_tick_count();
	}

	bool point_touched_timed(uint32_t p, uint32_t mintime)
	{
		uint32_t t = utils::get_tick_count();
		for (int32_t i = 0; i < 2; i++)
			if (touch_point_using_state[i][TOUCH_DOWN].p == p &&
				touch_point_using_state[i][TOUCH_UP].t < touch_point_using_state[i][TOUCH_DOWN].t)
			{
				if (touch_point_using_state[i][TOUCH_DOWN].t + mintime <= t)
					return true;
			}
		return false;
	}

	bool point_slide_done(uint32_t p_from, uint32_t p_to, uint32_t mintime, uint32_t maxtime)
	{
		uint32_t t = utils::get_tick_count();
		for (int32_t i = 0; i < 2; i++)
			if (touch_point_using_state[i][TOUCH_DOWN].p == p_from &&
				touch_point_using_state[i][TOUCH_UP].p == p_to &&
				touch_point_using_state[i][TOUCH_UP].t > touch_point_using_state[i][TOUCH_DOWN].t &&
				touch_point_using_state[i][TOUCH_UP].t + SLIDE_EXIST_TIME > t)
			{
				t = touch_point_using_state[i][TOUCH_UP].t - touch_point_using_state[i][TOUCH_DOWN].t;
				if (t >= mintime && t <= maxtime)
					return true;
			}
		return false;
	}

	bool menu_button_state = false;
	uint32_t menu_button_event_time = 0;

	void menu_button_event(bool state)
	{
		menu_button_event_time = utils::get_tick_count();
		menu_button_state = state;
	}

	bool menu_button_pressed()
	{
		return menu_button_state;
	}

	bool menu_button_pressed_timed(uint32_t mintime)
	{
		return menu_button_state && menu_button_event_time + mintime <= utils::get_tick_count();
	}
}