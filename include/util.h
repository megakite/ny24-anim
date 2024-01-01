#ifndef _UTIL_H_
#define _UTIL_H_

#include <stddef.h>

static inline size_t ceil_byte(size_t size)
{
	return size / 8 + (size % 8 != 0);
}

#endif//_UTIL_H_
