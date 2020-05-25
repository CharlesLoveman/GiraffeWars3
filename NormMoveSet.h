#ifndef _NORM_MOVESET_H_
#define _NORM_MOVESET_H_

#include "MoveSet.h"

struct NormMoveSet : public MoveSet {
public:
	NormMoveSet();
	~NormMoveSet();
	std::vector<HitCollider>* GetHitboxes(int MoveId, int FrameNum);
	std::array<HurtCollider, 6>* GetHurtboxes(int MoveId, int FrameNum);
	std::array<Vector2, NUM_POINTS>* GetSkelPoints(int MoveId, int FrameNum);
	int GetLandingLag(int MoveId);
	int GetMoveLength(int MoveId);

private:
	std::array<std::vector<std::vector<HitCollider>>, NUM_MOVES> Hitboxes;
	std::array<std::vector<std::array<HurtCollider, 6>>, NUM_MOVES> Hurtboxes;
	std::array<std::vector<std::array<Vector2, NUM_POINTS>>, NUM_MOVES> SkelPoints;
	std::array<int, 10> LandingLag;
};

#endif // !_NORM_MOVESET_H_

