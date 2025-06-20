#pragma once

#include "common.h"

namespace plugins
{
	void init(std::string storage_dir, std::string load_dir);
	bool handle_opcode(void *scriptHandle, uint32_t *ip, uint16_t opcode);
}
