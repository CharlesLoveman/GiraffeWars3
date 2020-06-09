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
#include "movebankheader.h"
#include "attackbankheader.h"
#include "Line.h"

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

enum GiraffeSoundMoveStates {
	SOUND_RUN = (1 << 0),
	SOUND_JUMP = (1 << 1),
	SOUND_JUMPLAND = (1 << 2),
	SOUND_KNOCKDOWN = (1 << 3),
	SOUND_HITSTUN = (1 << 4),
	SOUND_WAVEDASH = (1 << 5),
	SOUND_ROLL = (1 << 6),
	SOUND_CROUCH = (1 << 7),
	SOUND_DOUBLEJUMP = (1 << 8),
	SOUND_AIRDASH = (1 << 9),
	SOUND_FASTFALL = (1 << 10),
	SOUND_TECH = (1 << 11),
	SOUND_BOUNCE = (1 << 12),
	SOUND_SHIELD = (1 << 13),
	SOUND_DEATH = (1 << 14),
};

enum GiraffeSoundAttackStates {
	SOUND_GRAB = (1 << 0),
	SOUND_FTHROW = (1 << 1),
	SOUND_UPTHROW = (1 << 2),
	SOUND_BACKTHROW = (1 << 3),
	SOUND_DOWNTHROW = (1 << 4),
	SOUND_GETUPATTACK = (1 << 5),
	SOUND_JAB = (1 << 6),
	SOUND_FTILT = (1 << 7),
	SOUND_UPTILT = (1 << 8),
	SOUND_DTILT = (1 << 9),
	SOUND_FSMASH = (1 << 10),
	SOUND_UPSMASH = (1 << 11),
	SOUND_DSMASH = (1 << 12),
	SOUND_NAIR = (1 << 13),
	SOUND_FAIR = (1 << 14),
	SOUND_UPAIR = (1 << 15),
	SOUND_DAIR = (1 << 16),
	SOUND_BAIR = (1 << 17),
	SOUND_NEUTRALB = (1 << 18),
	SOUND_SIDEB = (1 << 19),
	SOUND_UPB = (1 << 20),
	SOUND_DOWNB = (1 << 21),
	SOUND_WEAK = (1 << 22),
	SOUND_MEDIUM = (1 << 23),
	SOUND_HEAVY = (1 << 24),
};

class Giraffe {
public:
	virtual ~Giraffe() { };
	virtual void Update(std::array<Giraffe*, GGPO_MAX_PLAYERS> giraffes, const int num_giraffes, const int i, const int inputs, const int frameNumber, Stage& stage);
	virtual void Draw(HDC hdc, Vector2 Scale, int frameNumber) = 0;
	virtual void Move(Stage& stage, const int frameNumber, std::array<Giraffe*, GGPO_MAX_PLAYERS> giraffes, std::vector<Line>& lines);
	bool AddHit(HitCollider hit, int ID, Vector2 facing, Vector2 position);
	bool ProjectileHit(Projectile p);
	bool GrabHit(Collider col, Vector2 _Facing, int frameNumber);
	static POINT VecToPoint(Vector2 vec, Vector2 scale);

	virtual int Size() = 0;

	Vector2 Position;
	Vector2 Velocity;
	int State;
	int SoundMoveState;
	int SoundAttackState;
	int SoundMoveDelay[XACT_WAVEBANK_MOVEBANK_ENTRY_COUNT];
	int SoundAttackDelay[XACT_WAVEBANK_ATTACKBANK_ENTRY_COUNT];
	const std::array<HurtCollider, 6>* Hurtboxes;
	const std::vector<HitCollider>* Hitboxes;
	float Knockback;

	friend struct NormProjFuncs;
	friend struct RobotProjFuncs;
	friend struct CoolProjFuncs;
	friend struct PoshProjFuncs;

protected:
	virtual void UniqueChanges(std::array<Giraffe*, GGPO_MAX_PLAYERS> giraffes, const int num_giraffes, const int i, const int inputs, const int frameNumber, Stage& stage) = 0;
	virtual void TransitionStates(const int frameNumber);
	virtual void ParseInputs(const int inputs, const int frameNumber, Stage& stage);
	virtual void ApplyChanges(std::array<Giraffe*, GGPO_MAX_PLAYERS> giraffes, const int num_giraffes, const int frameNumber, const int i);

	virtual void ParseWalk(const int inputs, const int frameNumber, Stage& stage);
	virtual void ParseWeak(const int inputs, const int frameNumber, Stage& stage);
	virtual void ParseHeavy(const int inputs, const int frameNumber, Stage& stage);
	virtual void ParseJump(const int inputs, const int frameNumber, Stage& stage);
	virtual void ParseShield(const int inputs, const int frameNumber, Stage& stage);

	virtual void GiveHits(std::array<Giraffe*, GGPO_MAX_PLAYERS> giraffes, const int num_giraffes, const int frameNumber, const int i);
	virtual void RecieveHits(Stage& stage, const int frameNumber);
	virtual void StageIntersection(Stage& stage, const int frameNumber, std::array<Giraffe*, GGPO_MAX_PLAYERS> giraffes, Vector2 offset, bool hogging, bool bounced, bool landed);
	virtual void Hogging(Stage& stage, const int frameNumber, std::array<Giraffe*, GGPO_MAX_PLAYERS> giraffes);
	virtual void Bouncing(Stage& stage, const int frameNumber, std::array<Giraffe*, GGPO_MAX_PLAYERS> giraffes);
	virtual void Landing(Stage& stage, const int frameNumber, std::array<Giraffe*, GGPO_MAX_PLAYERS> giraffes);
	virtual void Die(Stage& stage, const int frameNumber, std::vector<Line>& lines);
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
	HitCollider IncomingHits[8];
	ArrayQueue<int> PrevHitQueue;
	int LastAttackID;
	int numIncoming;
	int numHitboxes;

	//Grabs
	bool incomingGrab;
	int CommandGrabPointer;

	//Animation
	int AnimFrame;
	HPEN GiraffePen;
	HPEN IntangiblePen;
	HBRUSH ShieldBrush;
};

#endif // !_GIRAFFE_H_
