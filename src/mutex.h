#pragma once

#include "common.h"

namespace mutex
{
	enum eMutex
	{
		mlog,
		mplugin,

		msize
	};

	void init();
	void enter(eMutex m);
	void leave(eMutex m);
}

#define CS_SCOPE(m) \
	struct cs_t \
	{ \
		cs_t() { mutex::enter(m); } \
		~cs_t() { mutex::leave(m); } \
	} __cs;