#ifndef _GIRAFFE_H_
#define _GIRAFFE_H_

#include "ArrayQueue.h"
#include "Collider.h"
#include "Vec2.h"
#include "giraffewar.h"
#include "Stage.h"
#include <vector>
#include <array>

class Giraffe;
enum GiraffeStates {
	STATE_UP = (1 << 0),
	STATE_BACK = (1 << 1),
	STATE_DOWN = (1 << 2),
	STATE_FORWARD = (1 << 3),
	STATE_WEAK = (1 << 4),
	STATE_HEAVY = (1 << 5),
	STATE_SHIELDING = (1 << 6),
	STATE_DROPSHIELD = (1 << 7),
	STATE_SHIELDSTUN = (1 << 8),
	STATE_JUMPSQUAT = (1 << 9),
	STATE_JUMPLAND = (1 << 10),
	STATE_JUMPING = (1 << 11),
	STATE_HITSTUN = (1 << 12),
	STATE_FASTFALL = (1 << 13),
	STATE_DOUBLEJUMPWAIT = (1 << 14),
	STATE_WAVEDASH = (1 << 15),
	STATE_RUNNING = (1 << 16),
	STATE_SHORTHOP = (1 << 17),
	STATE_ATTACKSTUN = (1 << 18),
};


struct HitWID {
	HitCollider hit;
	int ID;
};

class Giraffe {
public:
	virtual ~Giraffe() { };
	virtual void Update(std::array<Giraffe*, 4> giraffes, const int num_giraffes, const int i, const int inputs, const int frameNumber) = 0;
	virtual void Draw(HDC hdc, Vec2 Scale) = 0;
	virtual void Move(Stage stage, const int frameNumber) = 0;
	void AddHit(HitCollider hit, int ID, Vec2 facing, Vec2 position);

	Vec2 Position;
	int State;
	const std::array<HurtCollider, 6>* Hurtboxes;
	const std::vector<HitCollider>* Hitboxes;
	HBRUSH ShieldBrush;
	float Knockback;

protected:
	//Movement
	Vec2 Velocity;
	float MaxGroundSpeed;
	Vec2 MaxAirSpeed;
	float RunAccel;
	float AirAccel;
	float Gravity;
	Vec2 Facing;

	//Jumping
	bool HasDoubleJump;
	float JumpSpeed;

	//Air Dash
	bool HasAirDash;
	float DashSpeed;

	//Misc
	int Stocks;
	float Mass;
	


	//State Management
	int JumpDelay;
	int AttackDelay;
	int MaxJumpDelay;
	int MaxShieldDelay;
	int AttackNum;
	


	//Collision
	//Head, Neck1, Neck2, Body, FrontLegs, BackLegs
	Collider Fullbody;
	Collider StageCollider;
	//HitCollider Hitboxes[6]; //Ranked by priority
	
	HitWID IncomingHits[GGPO_MAX_PLAYERS - 1];
	ArrayQueue PrevHitQueue;
	//int AttackID;
	int LastAttackID;
	int numIncoming;
	int numHitboxes;

	//Animation
	int AnimFrame;
};

#endif // !_GIRAFFE_H_
