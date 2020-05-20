#ifndef _VECRECT_H_
#define _VECRECT_H_

#include "Vec2.h"

struct VecRect {
public:
	float left;
	float top;
	float right;
	float bottom;

	RECT ToRect(Vec2 Scale);
};

#endif // !_VECRECT_H_

