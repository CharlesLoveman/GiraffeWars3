#ifndef _STAGE_H_
#define _STAGE_H_
#include <Windows.h>
#include "Collider.h"
#include "Projectile.h"
#include <vector>
#include "SimpleMath.h"

using namespace DirectX::SimpleMath;

//struct Platform {
//	Vector2 Position;
//	float Width;
//};

struct Ledge {
	Collider Col;
	bool Hogged;
	bool Facing;
};

struct Stage {
public:
	DirectX::SimpleMath::Rectangle Box;
	std::vector<DirectX::SimpleMath::Rectangle> Platforms;
	std::vector<Ledge> Ledges;
	bool Intersects(Vector2 pos, Collider col, bool down, bool jumping, bool falling, bool hitstun, bool& landed, bool& bounced, Vector2& facing, Vector2& offset, Vector2& deltaV, bool& hogging, int& ledgeID);
	bool KillProjectile(Projectile p);
	void Draw(HDC hdc, Vector2 Scale, HBRUSH Brush);
};

#endif // !_STAGE_H_
