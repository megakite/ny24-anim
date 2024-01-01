#ifndef _ANIM_INTERP_
#define _ANIM_INTERP_

#include <math.h>

static inline double interp_linear(const double v)
{
	return v;
}
static inline double interp_cubic(const double v)
{
	return 1 - pow(1 - v, 3);
}

#endif//_ANIM_INTERP_

