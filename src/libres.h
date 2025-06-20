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
	
	void *getsym(LPCSTR name);
	
	const char *getLibFileName();
	void *getLoadAddress();
	const std::vector<section_addr_space_t> &getExecutableSections();
}

template <typename T> inline T getsym(LPCSTR name)
{
	return cast<T>(libres::getsym(name));
}