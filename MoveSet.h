#ifndef _MOVESET_H_
#define _MOVESET_H_

#include "Collider.h"
#include <vector>
#include <array>

constexpr int NUM_MOVES = 24;
constexpr int NUM_POINTS = 38;

struct MoveSet {
public:
	virtual ~MoveSet() { };
	virtual std::vector<HitCollider>* GetHitboxes(int MoveId, int FrameNum) = 0;
	virtual std::array<HurtCollider, 6>* GetHurtboxes(int MoveId, int FrameNum) = 0;
	virtual std::array<Vec2, NUM_POINTS>* GetSkelPoints(int MoveId, int FrameNum) = 0;
	virtual int GetLandingLag(int MoveId) = 0;
	virtual int GetMoveLength(int MoveId) = 0;
};

#endif // !_MOVESET_H_
