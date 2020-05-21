#ifndef _STAGE_H_
#define _STAGE_H_
#include <Windows.h>
#include "Collider.h"
#include "VecRect.h"
#include <vector>

//struct Platform {
//	Vec2 Position;
//	float Width;
//};

struct Ledge {
	Collider Col;
	bool Hogged;
	bool Facing;
};

struct Stage {
public:
	VecRect Box;
	std::vector<VecRect> Platforms;
	std::vector<Ledge> Ledges;
	bool Intersects(Vec2 pos, Collider col, bool down, bool jumping, bool falling, bool& landed, Vec2& facing, Vec2& offset, Vec2& deltaV, bool& hogging, int& ledgeID);
	void Draw(HDC hdc, Vec2 Scale, HBRUSH Brush);
};

#endif // !_STAGE_H_
