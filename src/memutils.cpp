#include "memutils.h"

namespace memutils
{
	void mem_write_arr(uint8_t *addr, uint8_t *arr, uint32_t size, bool protect)
	{
		if (protect)
		{
			uint32_t a = ( cast<uint32_t>(addr) ) & ( ~(PAGE_SIZE - 1) );
			uint32_t pageCount = 0;
			for (int i = 0; i < size; i++)
				if (cast<uint32_t>(addr) + i >= a + PAGE_SIZE * pageCount)
					pageCount++;
			mprotect(cast<void *>(a), PAGE_SIZE * pageCount, PROT_READ | PROT_WRITE | PROT_EXEC);
		}
		for (int i = 0; i < size; i++)
			addr[i] = arr[i];
	}

	// regNum - R0..R7
	void mem_write_thumb_jmp(uint8_t *addrFrom, uint8_t *addrTo, uint8_t regNum)
	{
		// if this is thumb addr then make it normal
		addrFrom = cast<ptr>(cast<uint32_t>(addrFrom) & 0xFFFFFFFE);
		addrTo = cast<ptr>(cast<uint32_t>(addrTo) & 0xFFFFFFFE);

		uint8_t code[8];
		code[0] = 0x00;
		code[1] = 0x48 + regNum;
		code[2] = 0x87 + regNum * 8;
		code[3] = 0x46;
		*(uint8_t **)(&code[4]) = addrTo;
		mem_write_arr(addrFrom, code, sizeof(code));
	}

	void mem_write_thumb_rel_jmp(uint8_t *addrFrom, uint8_t *addrTo)
	{
		// if this is thumb addr then make it normal
		addrFrom = cast<ptr>(cast<uint32_t>(addrFrom) & 0xFFFFFFFE);

		int32_t offset = cast<int32_t>(addrTo) - cast<int32_t>(addrFrom) - 4;
		uint16_t lowPart = 0xF000 | ((offset >> 12) & 0x07FF);
		uint16_t highPart = 0xB800 | ((offset >> 1) & 0x07FF);
		uint32_t result = (highPart << 16) | lowPart;
		mem_write_arr(addrFrom, (uint8_t *)&result, sizeof(result));
	}

	void mem_write_thumb_rel_call(uint8_t *addrFrom, uint8_t *addrTo)
	{
		// if this is thumb addr then make it normal
		addrFrom = cast<ptr>(cast<uint32_t>(addrFrom) & 0xFFFFFFFE);

		int32_t offset = cast<int32_t>(addrTo) - cast<int32_t>(addrFrom) - 4;
		uint16_t lowPart = 0xF000 | ((offset >> 12) & 0x07FF);
		uint16_t highPart = 0xF800 | ((offset >> 1) & 0x07FF);
		uint32_t result = (highPart << 16) | lowPart;
		mem_write_arr(addrFrom, (uint8_t *)&result, sizeof(result));
	}

	void mem_write_thumb_long_jmp(uint8_t *addrFrom, uint8_t *addrTo)
	{
		// if addrFrom is thumb then make it normal
		addrFrom = cast<ptr>(cast<uint32_t>(addrFrom) & 0xFFFFFFFE);
		// make addrTo thumb, POP PC requires thumb
		addrTo = cast<ptr>(cast<uint32_t>(addrTo) | 1);

		uint8_t code[16] = {0x01, 0xB4, 0x01, 0xB4, 0x01, 0x48, 0x01, 0x90, 0x01, 0xBD, 0x00, 0xBF, 0, 0, 0, 0};
		*cast<uint8_t **>(&code[12]) = addrTo;
		mem_write_arr(addrFrom, code, sizeof(code));
	}

	void mem_write_arm_rel_jmp(uint8_t *addrFrom, uint8_t *addrTo)
	{
		int32_t offset = cast<int32_t>(addrTo) - cast<int32_t>(addrFrom) - 4;
		uint32_t result = (0xEA << 24) | ((offset / 4 - 1) & 0x00FFFFFF);
		mem_write_arr(addrFrom, (uint8_t *)&result, sizeof(result));
	}

	void mem_write_arm_rel_call(uint8_t *addrFrom, uint8_t *addrTo)
	{
		int32_t offset = cast<int32_t>(addrTo) - cast<int32_t>(addrFrom) - 4;
		uint32_t result = (0xEB << 24) | ((offset / 4 - 1) & 0x00FFFFFF);
		mem_write_arr(addrFrom, (uint8_t *)&result, sizeof(result));
	}

	void mem_write_arm_long_jmp(uint8_t *addrFrom, uint8_t *addrTo, bool toThumb)
	{
		// make addrTo thumb, POP PC requires thumb
		addrTo = cast<ptr>(toThumb ? (cast<uint32_t>(addrTo) | 1) : (cast<uint32_t>(addrTo) & ~1));

		uint8_t code[8] = {0x04, 0xF0, 0x1F, 0xE5, 0, 0, 0, 0};
		*cast<uint8_t **>(&code[4]) = addrTo;
		mem_write_arr(addrFrom, code, sizeof(code));
	}
}