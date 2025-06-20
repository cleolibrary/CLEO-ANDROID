#pragma once

#include "common.h"

namespace memutils
{
	void mem_write_arr(uint8_t *addr, uint8_t *arr, uint32_t size, bool protect = true);

#ifdef ANDROID

	// R0-7 allowed, addrFrom must divide by 4 without remainder
	void mem_write_thumb_jmp(uint8_t *addrFrom, uint8_t *addrTo, uint8_t regNum);
	// relative thumb-2 jmp
	void mem_write_thumb_rel_jmp(uint8_t *addrFrom, uint8_t *addrTo);
	// relative thumb-2 call
	void mem_write_thumb_rel_call(uint8_t *addrFrom, uint8_t *addrTo);
	// long 16 byte jmp
	void mem_write_thumb_long_jmp(uint8_t *addr_from, uint8_t *addr_to);
	// relative arm jmp
	void mem_write_arm_rel_jmp(uint8_t *addrFrom, uint8_t *addrTo);
	// relative arm call
	void mem_write_arm_rel_call(uint8_t *addrFrom, uint8_t *addrTo);
	// long 8 byte arm jmp, can jump to thumb
	void mem_write_arm_long_jmp(uint8_t *addrFrom, uint8_t *addrTo, bool toThumb);

#else

	// read addr from mips jmp/call
	ptr mem_read_mips_jmp(uint8_t *addr);
	// addrFrom must divide by 4 without remainder
	void mem_write_mips_jmp(uint8_t *addrFrom, uint8_t *addrTo, bool withNop);
	// addrFrom must divide by 4 without remainder
	void mem_write_mips_call(uint8_t *addrFrom, uint8_t *addrTo, bool withNop);

#endif
}