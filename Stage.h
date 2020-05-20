#ifndef _STAGE_H_
#define _STAGE_H_
#include <Windows.h>
#include "Collider.h"
#include "VecRect.h"

struct Stage {
public:
	VecRect Box;
	bool Intersects(Vec2 pos, Collider col, int& direction, float& offset);
};

#endif // !_STAGE_H_
