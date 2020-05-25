#ifndef _GIRAFFE_H_
#define _GIRAFFE_H_

#include "ArrayQueue.h"
#include "ArrayList.h"
#include "Collider.h"
#include "giraffewar.h"
#include "Stage.h"
#include <vector>
#include <array>
#include "MoveSet.h"
#include "Projectile.h"
#include "SimpleMath.h"

using namespace DirectX::SimpleMath;

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
	STATE_CROUCH = (1 << 18),
	STATE_LEDGEHOG = (1 << 19),
	STATE_TECHATTEMPT = (1 << 20),
	STATE_TECHLAG = (1 << 21),
	STATE_TECHING = (1 << 22),
	STATE_KNOCKDOWN = (1 << 23),
	STATE_KNOCKDOWNLAG = (1 << 24),
	STATE_ROLLING = (1 << 25),
	STATE_INTANGIBLE = (1 << 26),
	STATE_GETUPATTACK = (1 << 27),
	STATE_GRABBING = (1 << 28),
	STATE_GRABBED = (1 << 29),
	STATE_THROW = (1 << 30),
};


struct HitWID {
	HitCollider hit;
	int ID;
};

class Giraffe {
public:
	virtual ~Giraffe() { };
	virtual void Update(std::array<Giraffe*, 4> giraffes, const int num_giraffes, const int i, const int inputs, const int frameNumber, Stage& stage) = 0;
	virtual void Draw(HDC hdc, Vector2 Scale) = 0;
	virtual void Move(Stage& stage, const int frameNumber, std::array<Giraffe*, 4> giraffes) = 0;
	bool AddHit(HitCollider hit, int ID, Vector2 facing, Vector2 position);
	bool ProjectileHit(Projectile p);
	bool GrabHit(Collider col, Vector2 _Facing, int frameNumber);
	static POINT VecToPoint(Vector2 vec, Vector2 scale);

	Vector2 Position;
	Vector2 Velocity;
	int State;
	const std::array<HurtCollider, 6>* Hurtboxes;
	const std::vector<HitCollider>* Hitboxes;
	float Knockback;

	friend struct NormProjFuncs;

protected:
	//Movement
	
	float MaxGroundSpeed;
	Vector2 MaxAirSpeed;
	float RunAccel;
	float AirAccel;
	float Gravity;
	Vector2 Facing;

	//Jumping
	bool HasDoubleJump;
	float JumpSpeed;

	//Air Dash
	bool HasAirDash;
	float DashSpeed;

	//Misc
	int Stocks;
	float Mass;
	int LedgeID;
	ArrayList<Projectile> Projectiles;
	HINSTANCE hInst;


	//State Management
	int JumpDelay;
	int AttackDelay;
	int TechDelay;
	int MaxJumpDelay;
	int MaxShieldDelay;
	int AttackNum;
	MoveSet* Moves;

	//Collision
	Collider Fullbody;
	Collider StageCollider;
	HitWID IncomingHits[GGPO_MAX_PLAYERS - 1];
	ArrayQueue<int> PrevHitQueue;
	int LastAttackID;
	int numIncoming;
	int numHitboxes;

	//Grabs
	bool incomingGrab;
	//int GrabPointer;

	//Animation
	int AnimFrame;
	HPEN GiraffePen;
	HPEN IntangiblePen;
	HBRUSH ShieldBrush;
};

#endif // !_GIRAFFE_H_
