#include "armhook.h"
#include "libres.h"
#include "memutils.h"
#include "utils.h"

namespace armhook
{
#ifdef ANDROID

	#define	STEP_NEAR 16
	#define STEP_FAR 32

	#define SIZE_FAR 4096

	ptr g_near_space, g_near_space_limit, g_far_space, g_far_space_limit;

	void check_space()
	{
		if (g_near_space + STEP_NEAR > g_near_space_limit || g_far_space + STEP_FAR > g_far_space_limit)
		{
			utils::log("SPACE LIMIT REACHED");
			exit(1);
		}
	}

	void init(ptr nearSpace, uint32_t nearSize)
	{
		g_near_space = nearSpace;
		g_near_space_limit = nearSpace + nearSize;
		g_far_space = cast<ptr>(mmap(NULL, SIZE_FAR, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0));
		g_far_space_limit = g_far_space + SIZE_FAR;
	}

	void replace_thumb_call(ptr addr, ptr func_to)
	{
		check_space();
		memutils::mem_write_thumb_rel_call(addr, g_near_space);
		memutils::mem_write_thumb_long_jmp(g_near_space, func_to);
		g_near_space += STEP_NEAR;
	}

	void hook_thumb_func(ptr func, uint32_t startSize, ptr func_to, ptr *func_orig)
	{
		check_space();

		// if this is thumb addr then make it normal
		func = cast<ptr>(cast<uint32_t>(func) & 0xFFFFFFFE);

		// build orig func (copied start + jmp to the func code after start)
		uint8_t *space = g_far_space;
		memutils::mem_write_arr(space, func, startSize);
		space += startSize;
		// align by 4 using nop
		if (startSize % 4 != 0)
		{
			*cast<uint16_t *>(space) = 0x46C0;
			space += 2;
		}
		memutils::mem_write_thumb_long_jmp(space, func + startSize);
		*func_orig = g_far_space + 1; // set func_orig as thumb
		g_far_space += STEP_FAR;

		// write rel jmp from func to near and long from near to func_to
		memutils::mem_write_thumb_rel_jmp(func, g_near_space);
		memutils::mem_write_thumb_long_jmp(g_near_space, func_to);
		g_near_space += STEP_NEAR;
	}

	// func_to is always thumb
	void replace_arm_call(ptr addr, ptr func_to)
	{
		check_space();
		memutils::mem_write_arm_rel_call(addr, g_near_space);
		memutils::mem_write_arm_long_jmp(g_near_space, func_to, true);
		g_near_space += STEP_NEAR;
	}

	// func_to is always thumb
	void hook_arm_func(ptr func, uint32_t startSize, ptr func_to, ptr *func_orig)
	{
		check_space();

		// build orig func (copied start + jmp to the func code after start)
		uint8_t *space = g_far_space;
		memutils::mem_write_arr(space, func, startSize);
		space += startSize;
		// align by 4 using nop
		if (startSize % 4 != 0)
		{
			*cast<uint16_t *>(space) = 0x46C0;
			space += 2;
		}
		memutils::mem_write_arm_long_jmp(space, func + startSize, false);
		*func_orig = g_far_space;
		g_far_space += STEP_FAR;

		// write rel jmp from func to near and long from near to func_to
		memutils::mem_write_arm_rel_jmp(func, g_near_space);
		memutils::mem_write_arm_long_jmp(g_near_space, func_to, true);
		g_near_space += STEP_NEAR;
	}

#else

	void init()
	{
	}

	void replace_mips_call(ptr addr, ptr func_to)
	{
		memutils::mem_write_mips_call(addr, func_to, false);
	}

	void hook_mips_func(ptr func, uint32_t startSize, ptr func_to, ptr *func_orig)
	{
		uint8_t *space = cast<uint8_t *>(malloc(startSize + 32));
		while ((cast<uint32_t>(space) & 0x0F) != (cast<uint32_t>(func) & 0x0F))
			space++;
		memutils::mem_write_arr(space, func, startSize);
		memutils::mem_write_mips_jmp(space + startSize, func + startSize, true);
		*func_orig = space;
		memutils::mem_write_mips_jmp(func, func_to, true);
	}

	std::vector<ptr> find_mips_func_calls(ptr func)
	{
		uint32_t code = 0x0C000000 | ((cast<uint32_t>(func) >> 2) & 0x03FFFFFF);
		std::vector<ptr> res;

		void *imageBase = libres::getLoadAddress();
		const std::vector<libres::section_addr_space_t> &executable_sections = libres::getExecutableSections();

		for (size_t i = 0; i < executable_sections.size(); i++)
		{
			uint8_t *addrStart = (uint8_t *)imageBase + executable_sections[i].addr;
			uint32_t secSize = executable_sections[i].size;
			uint8_t *addrMax = addrStart + secSize - 4;
			for (uint8_t *addr = addrStart; addr < addrMax; addr += 4)
				if (*cast<uint32_t *>(addr) == code)
					res.push_back(addr);
		}

		return res;
	}

	std::vector<ptr> find_mips_func_calls_in_func(ptr func, ptr func_in)
	{
		uint32_t code = 0x0C000000 | ((cast<uint32_t>(func) >> 2) & 0x03FFFFFF);
		std::vector<ptr> res;

		// let's assume first jr $ra is func end
		for (uint8_t *addr = func_in; *cast<uint32_t *>(addr) != 0x03E00008; addr += 4)
			if (*cast<uint32_t *>(addr) == code)
				res.push_back(addr);

		return res;
	}

#endif
}
