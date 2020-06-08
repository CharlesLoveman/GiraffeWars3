#ifndef _MOVESET_H_
#define _MOVESET_H_

#include "Collider.h"
#include <vector>
#include <array>

constexpr int NUM_MOVES = 34;
constexpr int NUM_POINTS = 38;

struct MoveSet {
public:
	virtual ~MoveSet() { };
	virtual std::vector<HitCollider>* GetHitboxes(int MoveId, int FrameNum) = 0;
	virtual std::array<HurtCollider, 6>* GetHurtboxes(int MoveId, int FrameNum) = 0;
	virtual std::vector<Vector2>* GetSkelPoints(int MoveId, int FrameNum) = 0;
	virtual int GetLandingLag(int MoveId) = 0;
	virtual int GetMoveLength(int MoveId) = 0;
	virtual int GetAttackSoundLength(int AttackId) = 0;
	virtual int GetMoveSoundLength(int MoveId) = 0;
	virtual void InitMoves() = 0;
	virtual void InitThrows() = 0;
	virtual void InitTilts() = 0;
	virtual void InitSmashes() = 0;
	virtual void InitAerials() = 0;
	virtual void InitSpecials() = 0;
};

#endif // !_MOVESET_H_
