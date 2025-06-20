#pragma once

#include "common.h"
#include "core.h"

namespace libres
{
	struct section_addr_space_t
	{
		uint32_t	addr;
		uint32_t	size;
	};

	bool init(core::e_game &game, int32_t &image_base);	

#ifdef ANDROID
	void *getsym(LPCSTR name);	
#else
	uint32_t getGpValue();

	const char *getDiscId();
	const char *getDiscVersion();
	uint32_t getDiscVersionCode();
#endif

	const char *getLibFileName();
	void *getLoadAddress();
	const std::vector<section_addr_space_t> &getExecutableSections();
}

#ifdef ANDROID
template <typename T> inline T getsym(LPCSTR name)
{
	return cast<T>(libres::getsym(name));
}
#endif