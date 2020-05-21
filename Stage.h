#ifndef _STAGE_H_
#define _STAGE_H_
#include <Windows.h>
#include "Collider.h"
#include "VecRect.h"

//struct Platform {
//	Vec2 Position;
//	float Width;
//};

struct Stage {
public:
	VecRect Box;
	int NumPlats;
	VecRect Plats[3];
	bool Intersects(Vec2 pos, Collider col, bool down, bool falling, int& direction, float& offset);
};

#endif // !_STAGE_H_
