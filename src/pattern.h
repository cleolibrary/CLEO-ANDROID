#pragma once

#include "common.h"
#include "libres.h"

template <typename T>
#ifndef __INTELLISENSE__
__attribute__((__always_inline__))
#endif
inline bool __FindPatternAddress(T &result, const char *lpszPattern, int index = 0)
{
	__STATIC_ASSERT(sizeof(T) == sizeof(uint32_t));

	struct pattern_byte_t
	{
		uint8_t index;
		uint8_t value;
	};

	pattern_byte_t pattern_bytes[32];
	int pattern_byte_count = 0;
	uint8_t byte_index = 0;

	int str_current_char_index = 0;

#define ACH																			\
	if (!lpszPattern || !lpszPattern[str_current_char_index])						\
		break;																		\
	else																			\
	while (true)																	\
	{																				\
		char c1 = lpszPattern[str_current_char_index];								\
																					\
		if (c1 == ' ')																\
		{																			\
			str_current_char_index++;												\
			break;																	\
		}																			\
																					\
		char c2 = lpszPattern[str_current_char_index + 1];							\
																					\
		if (!c2 || ((c1 == '?' || c2 == '?') && c1 != c2))							\
			return false;															\
																					\
		if (c1 == '?')																\
		{																			\
			byte_index++;															\
			str_current_char_index += 2;											\
			break;																	\
		}																			\
																					\
		/* to upper */																\
		if (c1 >= 'a' && c1 <= 'f')	c1 -= 'a' - 'A';								\
		if (c2 >= 'a' && c2 <= 'f')	c2 -= 'a' - 'A';								\
																					\
		bool c1_in_bounds = (c1 >= '0' && c1 <= '9') || (c1 >= 'A' && c1 <= 'F');	\
		bool c2_in_bounds = (c2 >= '0' && c2 <= '9') || (c2 >= 'A' && c2 <= 'F');	\
																					\
		if (!c1_in_bounds || !c2_in_bounds)											\
			return false;															\
																					\
		uint8_t c1_value = c1 >= '0' && c1 <= '9' ? c1 - '0' : c1 - 'A' + 0xA;			\
		uint8_t c2_value = c2 >= '0' && c2 <= '9' ? c2 - '0' : c2 - 'A' + 0xA;			\
																					\
		uint8_t byte_value = (c1_value << 4) | c2_value;								\
																					\
		if (pattern_byte_count == sizeof(pattern_bytes) / sizeof(pattern_bytes[0]))	\
			return false;															\
																					\
		pattern_bytes[pattern_byte_count].index = byte_index;						\
		pattern_bytes[pattern_byte_count].value = byte_value;						\
																					\
		pattern_byte_count++;														\
																					\
		byte_index++;																\
		str_current_char_index += 2;												\
																					\
		break;																		\
	}

	while (true)
	{
#ifndef FIND_PATTERN_ADDR_COMPACT

		// pattern string is larger than the pattern !
		ACH; ACH; ACH; ACH; ACH; ACH; ACH; ACH;
		ACH; ACH; ACH; ACH; ACH; ACH; ACH; ACH;
		ACH; ACH; ACH; ACH; ACH; ACH; ACH; ACH;
		ACH; ACH; ACH; ACH; ACH; ACH; ACH; ACH;
		ACH; ACH; ACH; ACH; ACH; ACH; ACH; ACH;
		ACH; ACH; ACH; ACH; ACH; ACH; ACH; ACH;
		ACH; ACH; ACH; ACH; ACH; ACH; ACH; ACH;
		ACH; ACH; ACH; ACH; ACH; ACH; ACH; ACH;
		ACH; ACH; ACH; ACH; ACH; ACH; ACH; ACH;

#endif

		while (true)
			ACH;

		break;
	}

#undef ACH

	if (!pattern_byte_count)
		return false;

	void *imageBase = libres::getLoadAddress();
	const std::vector<libres::section_addr_space_t> &executable_sections = libres::getExecutableSections();

	for (size_t i = 0; i < executable_sections.size(); i++)
	{
		uint8_t *addrStart = (uint8_t *)imageBase + executable_sections[i].addr;
		uint32_t secSize = executable_sections[i].size;
		uint8_t *addrMax = addrStart + secSize - byte_index;
		for (uint8_t *addr = addrStart; addr < addrMax; addr++)
		{
			bool mismatch = false;

#define CMPB {																\
				if (i >= pattern_byte_count)								\
					break;													\
				if (addr[pattern_bytes[i].index] != pattern_bytes[i].value)	\
				{															\
					mismatch = true;										\
					break;													\
				}															\
				i++;														\
			}

			int i = 0;
			while (true)
			{
#ifndef FIND_PATTERN_ADDR_COMPACT

				CMPB; CMPB; CMPB; CMPB; CMPB; CMPB; CMPB; CMPB;
				CMPB; CMPB; CMPB; CMPB; CMPB; CMPB; CMPB; CMPB;
				CMPB; CMPB; CMPB; CMPB; CMPB; CMPB; CMPB; CMPB;
				CMPB; CMPB; CMPB; CMPB; CMPB; CMPB; CMPB; CMPB;

#endif

				while (true)
					CMPB;

				break;
			}

#undef CMPB

			if (!mismatch)
			{
				if (!index)
				{
					result = (T)addr;
					return true;
				}
				index--;
			}
		} // single section
	} // sections

	return false;
}

bool __FindPatternAddressCompact(void *&result, const char *lpszPattern, int index = 0);

template <typename T>
inline bool __FindPatternAddressCompact(T &result, const char *lpszPattern, int index = 0)
{
	return __FindPatternAddressCompact(cast<void *&>(result), lpszPattern, index);
}