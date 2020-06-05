#ifndef _RPBOT_MOVESET_H_
#define _ROBOT_MOVESET_H_

#include "MoveSet.h"
#include "attackbankheader.h"
#include "movebankheader.h"

constexpr int NUM_POINTS_ROBOT = 34;

struct RobotMoveSet : public MoveSet {
public:
	RobotMoveSet();
	~RobotMoveSet();
	std::vector<HitCollider>* GetHitboxes(int MoveId, int FrameNum);
	std::array<HurtCollider, 6>* GetHurtboxes(int MoveId, int FrameNum);
	std::vector<Vector2>* GetSkelPoints(int MoveId, int FrameNum);
	int GetLandingLag(int MoveId);
	int GetMoveLength(int MoveId);
	int GetAttackSoundLength(int AttackId);
	int GetMoveSoundLength(int MoveId);
	void InitMoves();
	void InitThrows();
	void InitTilts();
	void InitSmashes();
	void InitAerials();
	void InitSpecials();

private:
	std::array<std::vector<std::vector<HitCollider>>, NUM_MOVES> Hitboxes;
	std::array<std::vector<std::array<HurtCollider, 6>>, NUM_MOVES> Hurtboxes;
	std::array<std::vector<std::vector<Vector2>>, NUM_MOVES> SkelPoints;
	std::array<int, 10> LandingLag;
	std::array<int, XACT_WAVEBANK_ATTACKBANK_ENTRY_COUNT> AttackSoundLengths;
	std::array<int, XACT_WAVEBANK_MOVEBANK_ENTRY_COUNT> MoveSoundLengths;
};

#endif // !_NORM_MOVESET_H_