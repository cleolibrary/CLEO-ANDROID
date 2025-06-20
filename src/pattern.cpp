#define FIND_PATTERN_ADDR_COMPACT

#include "pattern.h"

bool __FindPatternAddressCompact(void *&result, const char *lpszPattern, int index)
{
	return __FindPatternAddress(result, lpszPattern, index);
}