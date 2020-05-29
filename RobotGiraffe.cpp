#include "RobotGiraffe.h"
#include "RobotProjFuncs.h"

RobotGiraffe::RobotGiraffe(Vector2 _Position, MoveSet* _Moves, HPEN _GiraffePen)
{
	//Movement
	Position = _Position;
	Velocity = Vector2(0, 0);
	MaxGroundSpeed = 0.3f;
	MaxAirSpeed = Vector2(0.2f, 0.7f);
	RunAccel = 0.1f;
	AirAccel = 0.03f;
	Gravity = 0.025f;
	Facing = { 1.0f, 1.0f };

	//Jumping
	JumpSpeed = 0.5f;
	HasDoubleJump = false;
	DashSpeed = 0.4f;
	HasAirDash = true;

	//Collision
	Fullbody = { Vector2(0.0f,0.0f), 2.5f };
	StageCollider = { Vector2(0.0f,0.7f), 1.0f };
	Hitboxes = nullptr;
	Hurtboxes = nullptr;
	numHitboxes = 0;
	numIncoming = 0;
	PrevHitQueue = ArrayQueue<int>();
	LastAttackID = 0;
	Projectiles = ArrayList<Projectile>();
	incomingGrab = false;

	//State
	State = 0;
	SoundMoveState = 0;
	SoundAttackState = 0;
	JumpDelay = 0;
	MaxJumpDelay = 4;
	AttackDelay = 0;
	AttackNum = 0;
	MaxShieldDelay = 5;
	TechDelay = 0;
	Moves = _Moves;

	//Misc
	Stocks = 3;
	Knockback = 0;
	Mass = 150;
	hInst = GetModuleHandle(NULL);
	CommandGrabPointer = 0;

	//Animation
	AnimFrame = 0;
	GiraffePen = _GiraffePen;
	IntangiblePen = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
	ShieldBrush = CreateHatchBrush(HS_BDIAGONAL, RGB(0, 255, 127));
	SpitBrush = CreateSolidBrush(RGB(90, 210, 180));
	ShineBrush = CreateSolidBrush(RGB(0, 255, 255));
}

RobotGiraffe::~RobotGiraffe()
{
}

void RobotGiraffe::Update(std::array<Giraffe*, 4> giraffes, const int num_giraffes, const int i, const int inputs, const int frameNumber, Stage& stage)
{
	++AnimFrame;

	//Transition States Based On Frame Number
	if (State & STATE_JUMPSQUAT && frameNumber >= JumpDelay) {
		if (State & STATE_SHORTHOP) {
			Velocity.y = -0.5f * JumpSpeed;
		}
		else {
			Velocity.y = -JumpSpeed;
		}
		State &= ~(STATE_JUMPSQUAT | STATE_SHORTHOP);
		State |= STATE_JUMPING | STATE_DOUBLEJUMPWAIT;
		JumpDelay = frameNumber + MaxJumpDelay * 2;
		SoundMoveState |= SOUND_JUMP;
		SoundMoveDelay[XACT_WAVEBANK_MOVEBANK_JUMP] = frameNumber + Moves->GetMoveSoundLength(XACT_WAVEBANK_MOVEBANK_JUMP);
	}
	else if (State & STATE_JUMPLAND && frameNumber >= JumpDelay) {
		State &= ~STATE_JUMPLAND;
	}
	else if (State & STATE_DOUBLEJUMPWAIT && frameNumber >= JumpDelay) {
		HasDoubleJump = true;
		State &= ~STATE_DOUBLEJUMPWAIT;
	}
	else if (State & STATE_WAVEDASH && frameNumber >= JumpDelay) {
		State &= ~STATE_WAVEDASH;
	}
	if (State & (STATE_WEAK | STATE_HEAVY | STATE_THROW) && frameNumber >= AttackDelay) {
		State &= ~(STATE_UP | STATE_BACK | STATE_DOWN | STATE_FORWARD | STATE_WEAK | STATE_HEAVY | STATE_THROW | STATE_GETUPATTACK);
		numHitboxes = 0;
	}
	else if (State & STATE_HITSTUN && frameNumber >= AttackDelay) {
		State &= ~STATE_HITSTUN;
		SoundMoveState &= ~SOUND_HITSTUN;
	}
	else if (State & STATE_SHIELDSTUN && frameNumber >= AttackDelay) {
		State &= ~STATE_SHIELDSTUN;
	}
	else if (State & STATE_DROPSHIELD && frameNumber >= AttackDelay) {
		State &= ~STATE_DROPSHIELD;
	}
	else if (State & STATE_ROLLING && frameNumber >= AttackDelay) {
		State &= ~STATE_ROLLING;
	}
	if (State & STATE_TECHATTEMPT && frameNumber >= TechDelay) {
		State &= ~STATE_TECHATTEMPT;
		State |= STATE_TECHLAG;
		TechDelay = frameNumber + 40;
	}
	else if (State & STATE_TECHLAG && frameNumber >= TechDelay) {
		State &= ~STATE_TECHLAG;
	}
	else if (State & STATE_TECHING && frameNumber >= TechDelay) {
		State &= ~STATE_TECHING;
		State &= ~STATE_INTANGIBLE;
	}
	else if (State & STATE_KNOCKDOWNLAG && frameNumber >= TechDelay) {
		State &= ~STATE_KNOCKDOWNLAG;
		State |= STATE_KNOCKDOWN;
	}
	else if (State & (STATE_GRABBING | STATE_GRABBED) && frameNumber >= TechDelay) {
		State &= ~(STATE_GRABBED | STATE_GRABBING);
	}

	for (int i = 0; i < XACT_WAVEBANK_MOVEBANK_ENTRY_COUNT; ++i) {
		if (SoundMoveState & (1 << i) && frameNumber >= SoundMoveDelay[i]) {
			SoundMoveState &= ~(1 << i);
		}
	}
	for (int i = 0; i < XACT_WAVEBANK_ATTACKBANK_ENTRY_COUNT; ++i) {
		if (SoundAttackState & (1 << i) && frameNumber >= SoundAttackDelay[i]) {
			SoundAttackState &= ~(1 << i);
		}
	}


	//Read Inputs
	if (!(State & (STATE_WEAK | STATE_HEAVY | STATE_JUMPSQUAT | STATE_JUMPLAND | STATE_DROPSHIELD | STATE_SHIELDSTUN | STATE_HITSTUN | STATE_KNOCKDOWNLAG | STATE_ROLLING | STATE_GRABBED | STATE_THROW))) {
		if (State & STATE_LEDGEHOG) {
			if (inputs & INPUT_DOWN) {
				State &= ~STATE_LEDGEHOG;
				stage.Ledges[LedgeID].Hogged = false;
				State |= STATE_JUMPING | STATE_DOUBLEJUMPWAIT;
				JumpDelay = frameNumber + MaxJumpDelay * 2;
				SoundMoveState &= ~SOUND_HITSTUN;
			}
		}
		else if (State & STATE_GRABBING) {
			if ((inputs & INPUT_RIGHT && Facing.x == 1) || (inputs & INPUT_LEFT && Facing.x == -1)) {
				State &= ~STATE_GRABBING;
				State |= STATE_FORWARD | STATE_THROW;
				AttackNum = 13;
				AttackDelay = frameNumber + Moves->GetMoveLength(AttackNum);
				AnimFrame = 0;
				SoundAttackState |= SOUND_FTHROW;
				SoundAttackDelay[XACT_WAVEBANK_ATTACKBANK_FTHROW] = frameNumber + Moves->GetAttackSoundLength(XACT_WAVEBANK_ATTACKBANK_FTHROW);
			}
			else if (inputs & INPUT_UP) {
				State &= ~STATE_GRABBING;
				State |= STATE_UP | STATE_THROW;
				AttackNum = 14;
				AttackDelay = frameNumber + Moves->GetMoveLength(AttackNum);
				AnimFrame = 0;
				SoundAttackState |= SOUND_UPTHROW;
				SoundAttackDelay[XACT_WAVEBANK_ATTACKBANK_UPTHROW] = frameNumber + Moves->GetAttackSoundLength(XACT_WAVEBANK_ATTACKBANK_UPTHROW);
			}
			else if (inputs & INPUT_DOWN) {
				State &= ~STATE_GRABBING;
				State |= STATE_DOWN | STATE_THROW;
				AttackNum = 15;
				AttackDelay = frameNumber + Moves->GetMoveLength(AttackNum);
				AnimFrame = 0;
				SoundAttackState |= SOUND_DOWNTHROW;
				SoundAttackDelay[XACT_WAVEBANK_ATTACKBANK_DOWNTHROW] = frameNumber + Moves->GetAttackSoundLength(XACT_WAVEBANK_ATTACKBANK_DOWNTHROW);
			}
			else if ((inputs & INPUT_LEFT && Facing.x == 1) || (inputs & INPUT_RIGHT && Facing.x == -1)) {
				State &= ~STATE_GRABBING;
				State |= STATE_BACK | STATE_THROW;
				AttackNum = 16;
				AttackDelay = frameNumber + Moves->GetMoveLength(AttackNum);
				AnimFrame = 0;
				SoundAttackState |= SOUND_BACKTHROW;
				SoundAttackDelay[XACT_WAVEBANK_ATTACKBANK_BACKTHROW] = frameNumber + Moves->GetAttackSoundLength(XACT_WAVEBANK_ATTACKBANK_BACKTHROW);
			}
		}
		else if (State & STATE_JUMPING) {
			if (inputs & INPUT_LEFT) {
				if (Velocity.x > -MaxAirSpeed.x) {
					Velocity.x -= AirAccel;
				}
			}
			else if (inputs & INPUT_RIGHT) {
				if (Velocity.x < MaxAirSpeed.x) {
					Velocity.x += AirAccel;
				}
			}
			if (inputs & INPUT_DOWN && !(inputs & (INPUT_WEAK | INPUT_HEAVY)) && !(State & STATE_FASTFALL)) {
				State |= STATE_FASTFALL;
				SoundMoveState |= SOUND_FASTFALL;
				SoundMoveDelay[XACT_WAVEBANK_MOVEBANK_FASTFALL] = frameNumber + Moves->GetMoveSoundLength(XACT_WAVEBANK_MOVEBANK_FASTFALL);
			}
		}
		else {
			if (inputs & INPUT_LEFT) {
				if (State & (STATE_TECHING | STATE_SHIELDING | STATE_KNOCKDOWN)) {
					State &= ~(STATE_TECHING | STATE_SHIELDING | STATE_KNOCKDOWN | STATE_CROUCH);
					State |= STATE_ROLLING | STATE_INTANGIBLE;
					AttackDelay = frameNumber + 16;
					AnimFrame = 0;
					Facing = { -1, 1 };
					SoundMoveState |= SOUND_ROLL;
					SoundMoveState &= ~SOUND_SHIELD;
					SoundMoveDelay[XACT_WAVEBANK_MOVEBANK_ROLL] = frameNumber + Moves->GetMoveSoundLength(XACT_WAVEBANK_MOVEBANK_ROLL);
				}
				else if (Velocity.x > -MaxGroundSpeed) {
					Velocity.x -= RunAccel;
					State &= ~STATE_CROUCH;
					State |= STATE_RUNNING;
					Facing = { -1, 1 };
					SoundMoveState |= SOUND_RUN;
					SoundMoveDelay[XACT_WAVEBANK_MOVEBANK_RUN] = frameNumber + Moves->GetMoveSoundLength(XACT_WAVEBANK_MOVEBANK_RUN);
				}
			}
			else if (inputs & INPUT_RIGHT) {
				if (State & (STATE_TECHING | STATE_SHIELDING | STATE_KNOCKDOWN)) {
					State &= ~(STATE_TECHING | STATE_SHIELDING | STATE_KNOCKDOWN | STATE_CROUCH);
					State |= STATE_ROLLING | STATE_INTANGIBLE;
					AttackDelay = frameNumber + 16;
					AnimFrame = 0;
					Facing = { 1, 1 };
					SoundMoveState |= SOUND_ROLL;
					SoundMoveState &= ~SOUND_SHIELD;
					SoundMoveDelay[XACT_WAVEBANK_MOVEBANK_ROLL] = frameNumber + Moves->GetMoveSoundLength(XACT_WAVEBANK_MOVEBANK_ROLL);
				}
				else if (Velocity.x < MaxGroundSpeed) {
					Velocity.x += RunAccel;
					State &= ~STATE_CROUCH;
					State |= STATE_RUNNING;
					Facing = { 1, 1 };
					SoundMoveState |= SOUND_RUN;
					SoundMoveDelay[XACT_WAVEBANK_MOVEBANK_RUN] = frameNumber + Moves->GetMoveSoundLength(XACT_WAVEBANK_MOVEBANK_RUN);
				}
			}
			else {
				State &= ~STATE_RUNNING;
				SoundMoveState &= ~SOUND_RUN;
			}

			if (inputs & INPUT_DOWN && !(inputs & (INPUT_WEAK | INPUT_HEAVY)) && !(State & (STATE_CROUCH | STATE_ROLLING))) {
				State |= STATE_CROUCH;
				SoundMoveState |= SOUND_CROUCH;
				SoundMoveDelay[XACT_WAVEBANK_MOVEBANK_CROUCH] = frameNumber + Moves->GetMoveSoundLength(XACT_WAVEBANK_MOVEBANK_CROUCH);
			}
			else if (!(inputs & INPUT_DOWN) && State & STATE_CROUCH) {
				State &= ~STATE_CROUCH;
			}
		}
	}
	if (inputs & INPUT_WEAK && !(State & (STATE_WEAK | STATE_HEAVY | STATE_SHIELDSTUN | STATE_DROPSHIELD | STATE_JUMPSQUAT | STATE_JUMPLAND | STATE_HITSTUN | STATE_WAVEDASH | STATE_KNOCKDOWNLAG | STATE_TECHING | STATE_ROLLING | STATE_GRABBING | STATE_GRABBED | STATE_THROW))) {
		State |= STATE_WEAK;
		State &= ~STATE_CROUCH;
		if (State & STATE_KNOCKDOWN) {
			State &= ~STATE_KNOCKDOWN;
			State |= STATE_GETUPATTACK;
			AttackNum = 17;
		}
		else if (State & STATE_LEDGEHOG) {
			State &= ~STATE_LEDGEHOG;
			stage.Ledges[LedgeID].Hogged = false;
			SoundMoveState &= ~SOUND_HITSTUN;
			State |= STATE_GETUPATTACK;
			AttackNum = 17;
			Position += Vector2(2.5f, -2.5f) * Facing;
		}
		else if ((inputs & INPUT_SHIELD || State & STATE_SHIELDING) && !(State & (STATE_JUMPING))) {
			State &= ~STATE_SHIELDING;
			State |= STATE_HEAVY;//Code grabs as weak and heavy
			AttackNum = 12;
		}
		else if ((inputs & INPUT_RIGHT && Facing.x == 1) || (inputs & INPUT_LEFT && Facing.x == -1)) {
			State |= STATE_FORWARD;
			AttackNum = 19;
		}
		else if (inputs & INPUT_UP) {
			State |= STATE_UP;
			AttackNum = 20;
		}
		else if (inputs & INPUT_DOWN) {
			State |= STATE_DOWN;
			AttackNum = 21;
		}
		else {//Neutral
			AttackNum = 18;
		}
		if (State & STATE_JUMPING) {
			if ((inputs & INPUT_RIGHT && Facing.x == -1) || (inputs & INPUT_LEFT && Facing.x == 1)) {
				//Bair
				State |= STATE_BACK;
				AttackNum = 29;
			}
			else {
				AttackNum += 7;
			}
		}
		AttackDelay = frameNumber + Moves->GetMoveLength(AttackNum);
		SoundAttackState |= (1 << (AttackNum - 12));
		SoundAttackDelay[AttackNum - 12] = frameNumber + Moves->GetAttackSoundLength(AttackNum - 12);
		++LastAttackID;
		AnimFrame = 0;
	}
	if (inputs & INPUT_HEAVY && !(State & (STATE_WEAK | STATE_HEAVY | STATE_SHIELDING | STATE_DROPSHIELD | STATE_JUMPSQUAT | STATE_JUMPLAND | STATE_HITSTUN | STATE_WAVEDASH | STATE_LEDGEHOG | STATE_KNOCKDOWN | STATE_KNOCKDOWNLAG | STATE_TECHING | STATE_ROLLING | STATE_GRABBING | STATE_GRABBED | STATE_THROW))) {
		State &= ~STATE_CROUCH;
		State |= STATE_HEAVY;
		if ((inputs & INPUT_RIGHT && Facing.x == 1) || (inputs & INPUT_LEFT && Facing.x == -1)) {
			//Fsmash
			State |= STATE_FORWARD;
			AttackNum = 22;
		}
		else if (inputs & INPUT_UP) {
			//Upsmash
			State |= STATE_UP;
			AttackNum = 23;
		}
		else if (inputs & INPUT_DOWN)
		{
			//Dsmash
			State |= STATE_DOWN;
			AttackNum = 24;
		}
		else
		{
			//Neutral B
			AttackNum = 30;
		}
		if (State & STATE_JUMPING) {
			if (State & (STATE_UP | STATE_DOWN | STATE_FORWARD)) {
				AttackNum += 9;
			}
		}
		AttackDelay = frameNumber + Moves->GetMoveLength(AttackNum);
		SoundAttackState |= (1 << (AttackNum - 12));
		SoundAttackDelay[AttackNum - 12] = frameNumber + Moves->GetAttackSoundLength(AttackNum - 12);
		++LastAttackID;
		AnimFrame = 0;
	}

	if (inputs & INPUT_JUMP && !(State & (STATE_WEAK | STATE_HEAVY | STATE_JUMPSQUAT | STATE_JUMPLAND | STATE_HITSTUN | STATE_SHIELDSTUN | STATE_WAVEDASH | STATE_KNOCKDOWNLAG | STATE_TECHING | STATE_ROLLING | STATE_GRABBING | STATE_GRABBED | STATE_THROW))) {
		if (State & STATE_LEDGEHOG) {
			State &= ~STATE_LEDGEHOG;
			stage.Ledges[LedgeID].Hogged = false;
			State |= STATE_JUMPING | STATE_DOUBLEJUMPWAIT;
			JumpDelay = frameNumber + MaxJumpDelay;
			Velocity.y = -JumpSpeed;
			AnimFrame = 0;
			SoundMoveState |= SOUND_JUMP;
			SoundMoveState &= ~SOUND_HITSTUN;
			SoundMoveDelay[XACT_WAVEBANK_MOVEBANK_JUMP] = frameNumber + Moves->GetMoveSoundLength(XACT_WAVEBANK_MOVEBANK_JUMP);
		}
		else if (!(State & STATE_JUMPING)) {
			State |= STATE_JUMPSQUAT;
			JumpDelay = frameNumber + MaxJumpDelay;
			AnimFrame = 0;
			State &= ~(STATE_SHIELDING | STATE_DROPSHIELD | STATE_RUNNING | STATE_CROUCH | STATE_KNOCKDOWN);
			SoundMoveState &= ~(SOUND_SHIELD | SOUND_RUN);
		}
		else if (HasDoubleJump) {
			HasDoubleJump = false;
			Velocity.y = -JumpSpeed;
			State &= ~STATE_FASTFALL;
			SoundMoveState |= SOUND_DOUBLEJUMP;
			SoundMoveDelay[XACT_WAVEBANK_MOVEBANK_DOUBLEJUMP] = frameNumber + Moves->GetMoveSoundLength(XACT_WAVEBANK_MOVEBANK_DOUBLEJUMP);
		}
	}
	else if (!(inputs & INPUT_JUMP) && (State & STATE_JUMPSQUAT)) {
		State |= STATE_SHORTHOP;
	}
	else if (inputs & INPUT_JUMP && State & STATE_HEAVY && State & STATE_DOWN && State & STATE_JUMPING && HasDoubleJump) {
		State &= ~(STATE_HEAVY | STATE_DOWN);
		HasDoubleJump = false;
		Velocity.y = -JumpSpeed;
		State &= ~STATE_FASTFALL;
		SoundAttackState &= ~SOUND_DOWNB;
		SoundMoveState |= SOUND_DOUBLEJUMP;
		SoundMoveDelay[XACT_WAVEBANK_MOVEBANK_DOUBLEJUMP] = frameNumber + Moves->GetMoveSoundLength(XACT_WAVEBANK_MOVEBANK_DOUBLEJUMP);
	}

	if (inputs & INPUT_SHIELD) {
		if ((State & STATE_JUMPING) && !(State & (STATE_TECHLAG | STATE_TECHATTEMPT | STATE_GRABBING | STATE_GRABBED))) {
			State |= STATE_TECHATTEMPT;
			TechDelay = frameNumber + 20;
		}

		if (inputs & INPUT_DOWN && State & STATE_JUMPSQUAT && HasAirDash) {
			HasAirDash = false;
			State &= ~(STATE_JUMPSQUAT | STATE_SHORTHOP);
			State |= STATE_WAVEDASH;
			JumpDelay = frameNumber + 2 * MaxJumpDelay;
			SoundMoveState |= SOUND_WAVEDASH;
			SoundMoveDelay[XACT_WAVEBANK_MOVEBANK_WAVEDASH] = frameNumber + Moves->GetMoveSoundLength(XACT_WAVEBANK_MOVEBANK_WAVEDASH);

			Velocity.y += DashSpeed;
			if (inputs & INPUT_LEFT) {
				Velocity.x -= DashSpeed;
			}
			else if (inputs & INPUT_RIGHT) {
				Velocity.x += DashSpeed;
			}
		}
		else if (State & STATE_JUMPING && !(State & (STATE_WEAK | STATE_HEAVY | STATE_HITSTUN)) && HasAirDash) {
			HasAirDash = false;
			State &= ~STATE_FASTFALL;
			SoundMoveState |= SOUND_AIRDASH;
			SoundMoveDelay[XACT_WAVEBANK_MOVEBANK_AIRDASH] = frameNumber + Moves->GetMoveSoundLength(XACT_WAVEBANK_MOVEBANK_AIRDASH);
			if (inputs & INPUT_LEFT) {
				Velocity.x -= DashSpeed;
			}
			else if (inputs & INPUT_RIGHT) {
				Velocity.x += DashSpeed;
			}
			if (inputs & INPUT_UP) {
				Velocity.y -= DashSpeed;
			}
			else if (inputs & INPUT_DOWN) {
				Velocity.y += DashSpeed;
			}
		}
		else if (!(State & (STATE_WEAK | STATE_HEAVY | STATE_JUMPSQUAT | STATE_JUMPLAND | STATE_JUMPING | STATE_HITSTUN | STATE_SHIELDSTUN | STATE_WAVEDASH | STATE_LEDGEHOG | STATE_KNOCKDOWN | STATE_KNOCKDOWNLAG | STATE_ROLLING | STATE_GRABBING | STATE_GRABBED | STATE_THROW))) {
			State &= ~(STATE_DROPSHIELD | STATE_WAVEDASH | STATE_RUNNING);
			State |= STATE_SHIELDING;
			Velocity.x = 0;
			SoundMoveState |= SOUND_SHIELD;
			SoundMoveDelay[XACT_WAVEBANK_MOVEBANK_SHIELD] = 1000000000;
		}
	}
	else if (State & STATE_SHIELDING) {
		State &= ~STATE_SHIELDING;
		State |= STATE_DROPSHIELD;
		AttackDelay = frameNumber + MaxShieldDelay;
		SoundMoveState &= ~SOUND_SHIELD;
	}

	if (State & STATE_ROLLING) {
		Velocity.x = MaxGroundSpeed * Facing.x;
		if (AnimFrame == 10) {
			State &= ~STATE_INTANGIBLE;
		}
	}

	//Grab
	if (State & STATE_WEAK && State & STATE_HEAVY && AnimFrame == 11) {
		Projectiles.Append(Projectile(Position + Vector2(Facing.x * 0.5f, 0), { Facing.x * 0.6f, 0 }, 0.5f, { 0,0 }, 0, 0, 0, true, LastAttackID, frameNumber + 15, RobotProjFuncs::GrabberOnHit, RobotProjFuncs::GrabberUpdate, RobotProjFuncs::GrabberDraw, GiraffePen, ShieldBrush));
		//Collider grabCol = { {Position.x + 1.885000f * Facing.x, Position.y - 0.650000f}, 0.855862f };
		//for (int j = 0; j < num_giraffes; ++j) {
		//	if (j != i) {
		//		if (giraffes[j]->GrabHit(grabCol, Facing, frameNumber)) {
		//			State &= ~(STATE_WEAK | STATE_HEAVY | STATE_RUNNING);
		//			State |= STATE_GRABBING;
		//			TechDelay = frameNumber + 30;
		//			//GrabPointer = j;
		//			break;
		//		}
		//	}
		//}
	}
	//B-Reverse
	if ((State & STATE_HEAVY) && (State & STATE_JUMPING) && AnimFrame == 1 && (((inputs & INPUT_LEFT) && Facing.x == 1) || ((inputs & INPUT_RIGHT) && Facing.x == -1))) {
		Facing.x *= -1;
	}

	//Character Move-Specific Updates
	if ((State & STATE_JUMPING) && (State & STATE_HEAVY) && (State & STATE_FORWARD) && AnimFrame == 7) {
		Collider grabCol = { {Position.x + 1.885000f * Facing.x, Position.y - 0.650000f}, 0.855862f };
		for (int j = 0; j < num_giraffes; ++j) {
			if (j != i) {
				if (giraffes[j]->GrabHit(grabCol, Facing, frameNumber)) {
					State &= ~(STATE_WEAK | STATE_HEAVY | STATE_RUNNING);
					State |= STATE_GRABBING;
					TechDelay = frameNumber + 30;
					giraffes[j]->Velocity = Velocity;
					giraffes[j]->State |= STATE_JUMPING;
					CommandGrabPointer = j;
					break;
				}
			}
		}
	}



	//if ((State & STATE_HEAVY) && (State & STATE_UP)) {
	//	//Fire neck
	//	if (State & STATE_JUMPING) {
	//		if (AnimFrame == 10) {
	//			Projectiles.Append(Projectile(Position + Vector2(0.2f, -1.2f), { Facing.x * 0.65f, -0.65f }, 0.3f, { 0.0f, 0.0f }, 0.1f, 0.1f, 1.0f, true, LastAttackID, AttackDelay - 10, RobotProjFuncs::NeckGrabOnHit, RobotProjFuncs::NeckGrabUpdate, RobotProjFuncs::NeckGrabDraw, GiraffePen, SpitBrush));
	//		}
	//	}
	//	//Main hit of upsmash
	//	else if (AnimFrame == 17) {
	//		++LastAttackID;
	//	}
	//}
	////Fire spit
	//else if ((State & STATE_HEAVY) && !(State & (STATE_WEAK | STATE_UP | STATE_FORWARD | STATE_BACK | STATE_DOWN)) && AnimFrame == 13) {
	//	Projectiles.Append(Projectile(Position + Vector2(0.2f, -1.2f), { Facing.x * 0.5f, 0.0f }, 0.3f, { Facing.x, 0.0f }, 0.1f, 0.1f, 1.0f, true, LastAttackID, frameNumber + 100, RobotProjFuncs::SpitOnHit, RobotProjFuncs::SpitUpdate, RobotProjFuncs::SpitDraw, GiraffePen, SpitBrush));
	//}
	////Reverse direction in bair
	//else if ((State & STATE_JUMPING) && (State & STATE_WEAK) && (State & STATE_BACK) && AnimFrame == 7) {
	//	Facing.x *= -1;
	//}
	////Spin like a maniac in get-up attack
	//else if ((State & STATE_GETUPATTACK) && (7 <= AnimFrame) && (AnimFrame <= 10)) {
	//	Facing.x *= -1;
	//}


	//Update State
	if (State & (STATE_WEAK | STATE_HEAVY | STATE_THROW)) {
		Hitboxes = Moves->GetHitboxes(AttackNum, AnimFrame);
		Hurtboxes = Moves->GetHurtboxes(AttackNum, AnimFrame);
		numHitboxes = (int)(*Hitboxes).size();
	}
	else {
		Hitboxes = nullptr;
		numHitboxes = 0;
		if (State & STATE_HITSTUN) {
			Hurtboxes = Moves->GetHurtboxes(6, AnimFrame % 8);
		}
		else if (State & STATE_RUNNING) {
			Hurtboxes = Moves->GetHurtboxes(1, AnimFrame % 2);
		}
		else if (State & STATE_JUMPING) {
			Hurtboxes = Moves->GetHurtboxes(2, 0);
		}
		else if (State & STATE_JUMPSQUAT) {
			Hurtboxes = Moves->GetHurtboxes(3, AnimFrame);
		}
		else if (State & STATE_JUMPLAND) {
			Hurtboxes = Moves->GetHurtboxes(4, AnimFrame);
		}
		else if (State & (STATE_WAVEDASH | STATE_CROUCH)) {
			Hurtboxes = Moves->GetHurtboxes(7, 0);
		}
		else if (State & STATE_ROLLING) {
			Hurtboxes = Moves->GetHurtboxes(8, 0);
		}
		else if (State & STATE_LEDGEHOG) {
			Hurtboxes = Moves->GetHurtboxes(6, AnimFrame % 8);
		}
		else if (State & STATE_GRABBING) {
			Hurtboxes = Moves->GetHurtboxes(12, 7);
		}
		else { //Idle
			Hurtboxes = Moves->GetHurtboxes(0, 0);
		}
	}



	//Adding Gravity
	if (State & STATE_JUMPING) {
		if (Velocity.y < MaxAirSpeed.y) {
			Velocity.y += Gravity;
		}
		if (State & STATE_FASTFALL) {
			Velocity.y += 3 * Gravity;
		}
	}
	for (int p = Projectiles.Size() - 1; p >= 0; --p) {
		if (Projectiles[p].Update(Projectiles[p], *this, frameNumber)) {
			Projectiles.Remove(p);
		}
	}

	//Adding Friction/Air resitance
	if (!(State & STATE_JUMPING)) {
		Velocity *= 0.9f;
	}
	else {
		if (fabs(Velocity.y) > MaxAirSpeed.y) {
			Velocity.y *= 0.8f;
		}
		if (fabs(Velocity.x) > MaxAirSpeed.x) {
			Velocity.x *= 0.95f;
		}
	}




	//Apply hits to other giraffes
	for (int p = Projectiles.Size() - 1; p >= 0; --p) {
		for (int j = 0; j < num_giraffes; ++j) {
			if (j != i) {
				if ((*giraffes[j]).ProjectileHit(Projectiles[p])) {
					Projectiles[p].OnHit(Projectiles[p], *this, giraffes[j], frameNumber);
					Projectiles.Remove(p);
					SoundAttackState |= SOUND_WEAK;
					SoundAttackDelay[XACT_WAVEBANK_ATTACKBANK_WEAK] = frameNumber + Moves->GetAttackSoundLength(XACT_WAVEBANK_ATTACKBANK_WEAK);
					break;
				}
			}
		}
	}


	if (!(Hitboxes == nullptr)) {
		for (int j = 0; j < num_giraffes; ++j) {
			if (j != i) {
				for (int h = 0; h < numHitboxes; ++h) {
					if ((*giraffes[j]).AddHit((*Hitboxes)[h], LastAttackID, Facing, Position)) {
						if ((*Hitboxes)[h].Damage > 1) {
							SoundAttackState |= SOUND_HEAVY;
							SoundAttackDelay[XACT_WAVEBANK_ATTACKBANK_HEAVY] = frameNumber + Moves->GetAttackSoundLength(XACT_WAVEBANK_ATTACKBANK_HEAVY);
						}
						else if ((*Hitboxes)[h].Damage > 0.5f) {
							SoundAttackState |= SOUND_MEDIUM;
							SoundAttackDelay[XACT_WAVEBANK_ATTACKBANK_MEDIUM] = frameNumber + Moves->GetAttackSoundLength(XACT_WAVEBANK_ATTACKBANK_MEDIUM);
						}
						else {
							SoundAttackState |= SOUND_WEAK;
							SoundAttackDelay[XACT_WAVEBANK_ATTACKBANK_WEAK] = frameNumber + Moves->GetAttackSoundLength(XACT_WAVEBANK_ATTACKBANK_WEAK);
						}
					}
				}
			}
		}
	}

}

void RobotGiraffe::Move(Stage& stage, const int frameNumber, std::array<Giraffe*, 4> giraffes)
{
	//Recieve incoming hits
	if (!(State & STATE_INTANGIBLE)) {
		if (incomingGrab) {
			State &= ~(STATE_UP | STATE_BACK | STATE_DOWN | STATE_FORWARD | STATE_WEAK | STATE_HEAVY | STATE_SHIELDING | STATE_DROPSHIELD | STATE_SHIELDSTUN | STATE_JUMPSQUAT | STATE_JUMPLAND | STATE_HITSTUN | STATE_FASTFALL | STATE_DOUBLEJUMPWAIT | STATE_WAVEDASH | STATE_RUNNING | STATE_SHORTHOP | STATE_CROUCH | STATE_TECHATTEMPT | STATE_TECHLAG | STATE_TECHING | STATE_KNOCKDOWN | STATE_KNOCKDOWNLAG | STATE_ROLLING | STATE_GETUPATTACK | STATE_GRABBING | STATE_THROW);
			State |= STATE_GRABBED;
			SoundMoveState &= ~(SOUND_SHIELD | SOUND_RUN);
			incomingGrab = false;
		}
		if (State & STATE_SHIELDING) {
			for (int j = 0; j < numIncoming; ++j) {
				if (!PrevHitQueue.Contains(IncomingHits[j].ID)) {
					State |= STATE_SHIELDSTUN;
					AttackDelay = (int)max(AttackDelay, frameNumber + IncomingHits[j].hit.Damage * 50);
					PrevHitQueue.Push(IncomingHits[j].ID);
				}
			}
		}
		else {
			for (int j = 0; j < numIncoming; ++j) {
				if (!PrevHitQueue.Contains(IncomingHits[j].ID)) {
					Knockback += IncomingHits[j].hit.Damage;
					float KnockbackApplied;
					if (IncomingHits[j].hit.Fixed) {
						KnockbackApplied = IncomingHits[j].hit.Knockback;
					}
					else {
						KnockbackApplied = ((((Knockback / 10 + (Knockback * IncomingHits[j].hit.Damage / 20)) * (200 / (Mass + 100)) * 1.4f + 0.18f) * IncomingHits[j].hit.Scale) + IncomingHits[j].hit.Knockback);
					}
					Velocity += KnockbackApplied * IncomingHits[j].hit.Force;
					if (State & STATE_LEDGEHOG) {
						stage.Ledges[LedgeID].Hogged = false;
						State &= ~STATE_LEDGEHOG;
					}
					State &= ~(STATE_UP | STATE_BACK | STATE_DOWN | STATE_FORWARD | STATE_WEAK | STATE_HEAVY | STATE_JUMPSQUAT | STATE_JUMPLAND | STATE_SHORTHOP | STATE_KNOCKDOWN | STATE_KNOCKDOWNLAG | STATE_GETUPATTACK | STATE_GRABBING | STATE_GRABBED | STATE_THROW);
					State |= STATE_HITSTUN;
					SoundMoveState |= SOUND_HITSTUN;
					SoundMoveDelay[XACT_WAVEBANK_MOVEBANK_HITSTUN] = 100000000;
					AttackDelay = (int)max(AttackDelay, frameNumber + min(KnockbackApplied * 40, 100));
					PrevHitQueue.Push(IncomingHits[j].ID);
				}
			}
		}
	}
	numIncoming = 0;

	//Test projectile intersection with stage
	for (int i = Projectiles.Size() - 1; i >= 0; --i) {
		Projectiles[i].Position += Projectiles[i].Velocity;
		if (stage.KillProjectile(Projectiles[i])) {
			Projectiles[i].OnHit(Projectiles[i], *this, nullptr, frameNumber);
			Projectiles.Remove(i);
		};
	}

	Position += Velocity;
	//Correct intersection with the stage
	bool landed = false;
	bool hogging = false;
	bool bounced = false;
	Vector2 offset;
	if (!(State & STATE_LEDGEHOG)) {
		if (stage.Intersects(Position, StageCollider, State & (STATE_CROUCH | STATE_FASTFALL), State & STATE_JUMPING, Velocity.y > -0.00001, State & STATE_HITSTUN, landed, bounced, Facing, offset, Velocity, hogging, LedgeID)) {
			Position += offset;
			if (hogging) {
				State &= ~(STATE_UP | STATE_BACK | STATE_DOWN | STATE_FORWARD | STATE_WEAK | STATE_HEAVY | STATE_JUMPING | STATE_FASTFALL | STATE_GETUPATTACK | STATE_GRABBING | STATE_GRABBED | STATE_THROW);
				State |= STATE_LEDGEHOG;
				SoundMoveState |= SOUND_HITSTUN;
				SoundMoveDelay[XACT_WAVEBANK_MOVEBANK_HITSTUN] = 1000000000;
			}
			else if (bounced) {
				if (State & STATE_TECHATTEMPT) {
					Velocity = { 0,0 };
					State &= ~(STATE_TECHATTEMPT | STATE_HITSTUN);
					State |= STATE_TECHING | STATE_INTANGIBLE;
					TechDelay = frameNumber + 5;
					SoundMoveState |= SOUND_TECH;
					SoundMoveState &= ~SOUND_HITSTUN;
				}
			}
			else if (landed && (State & STATE_JUMPING)) {
				SoundMoveState |= SOUND_JUMPLAND;
				SoundMoveDelay[XACT_WAVEBANK_MOVEBANK_JUMPLAND] = frameNumber + Moves->GetMoveSoundLength(XACT_WAVEBANK_MOVEBANK_JUMPLAND);
				if (State & STATE_HITSTUN && !(State & STATE_TECHATTEMPT)) {
					Velocity = { 0,0 };
					State |= STATE_KNOCKDOWNLAG;
					AnimFrame = 0;
					TechDelay = frameNumber + 30;
					SoundMoveState |= SOUND_KNOCKDOWN;
					SoundMoveState &= ~SOUND_HITSTUN;
					SoundMoveDelay[XACT_WAVEBANK_MOVEBANK_KNOCKDOWN] = frameNumber + Moves->GetMoveSoundLength(XACT_WAVEBANK_MOVEBANK_KNOCKDOWN);
				}
				else if (State & STATE_GRABBING) {
					State &= ~STATE_GRABBING;
					giraffes[CommandGrabPointer]->State &= ~STATE_GRABBED;
				}
				else {
					if (State & (STATE_WEAK | STATE_HEAVY)) {
						State |= STATE_HITSTUN;
						AttackDelay = frameNumber + Moves->GetLandingLag(max(0, (AttackNum - 25)));
						SoundMoveState |= SOUND_HITSTUN;
						SoundMoveDelay[XACT_WAVEBANK_MOVEBANK_HITSTUN] = AttackDelay;
					}
					else if ((State & STATE_TECHATTEMPT) && (State & STATE_HITSTUN)) {
						State &= ~STATE_TECHATTEMPT;
						State |= STATE_TECHING | STATE_INTANGIBLE;
						TechDelay = frameNumber + 20;
						SoundMoveState &= ~SOUND_HITSTUN;
						SoundMoveState |= SOUND_TECH;
						SoundMoveDelay[XACT_WAVEBANK_MOVEBANK_TECH] = frameNumber + Moves->GetMoveSoundLength(XACT_WAVEBANK_MOVEBANK_TECH);
					}

					State |= STATE_JUMPLAND;
					HasAirDash = true;
					HasDoubleJump = false;
					JumpDelay = frameNumber + MaxJumpDelay / 2;
					AnimFrame = 0;
				}
				SoundAttackState &= ~(SOUND_DOWNB | SOUND_UPB);
				State &= ~(STATE_UP | STATE_BACK | STATE_DOWN | STATE_FORWARD | STATE_WEAK | STATE_HEAVY | STATE_JUMPING | STATE_FASTFALL | STATE_HITSTUN | STATE_TECHLAG | STATE_THROW | STATE_GRABBED | STATE_GRABBING);
			}
		}
		else if (!(State & (STATE_JUMPING | STATE_GRABBED))) {
			State &= ~(STATE_UP | STATE_BACK | STATE_DOWN | STATE_FORWARD | STATE_WEAK | STATE_HEAVY | STATE_SHIELDING | STATE_DROPSHIELD | STATE_SHIELDSTUN | STATE_JUMPSQUAT | STATE_SHORTHOP | STATE_JUMPLAND | STATE_WAVEDASH | STATE_CROUCH | STATE_GRABBING | STATE_THROW | STATE_KNOCKDOWN);
			State |= STATE_JUMPING | STATE_DOUBLEJUMPWAIT;
			JumpDelay = frameNumber + MaxJumpDelay * 2;
		}
	}


	//Make giraffes loop around like pac-man
	//Remove this later
	if (Position.y > 60) {
		Position.y = 0;
		Knockback = 0;
		SoundMoveState |= SOUND_DEATH;
		SoundMoveDelay[XACT_WAVEBANK_MOVEBANK_DEATH] = frameNumber + Moves->GetMoveSoundLength(XACT_WAVEBANK_MOVEBANK_DEATH);
	}
	else if (Position.y < 0) {
		Position.y = 60;
		Knockback = 0;
		SoundMoveState |= SOUND_DEATH;
		SoundMoveDelay[XACT_WAVEBANK_MOVEBANK_DEATH] = frameNumber + Moves->GetMoveSoundLength(XACT_WAVEBANK_MOVEBANK_DEATH);
	}
	if (Position.x > 60) {
		Position.x = 0;
		Knockback = 0;
		SoundMoveState |= SOUND_DEATH;
		SoundMoveDelay[XACT_WAVEBANK_MOVEBANK_DEATH] = frameNumber + Moves->GetMoveSoundLength(XACT_WAVEBANK_MOVEBANK_DEATH);
	}
	else if (Position.x < 0) {
		Position.x = 60;
		Knockback = 0;
		SoundMoveState |= SOUND_DEATH;
		SoundMoveDelay[XACT_WAVEBANK_MOVEBANK_DEATH] = frameNumber + Moves->GetMoveSoundLength(XACT_WAVEBANK_MOVEBANK_DEATH);
	}


	//This is the last stateful part of the update loop

}

void RobotGiraffe::Draw(HDC hdc, Vector2 Scale)
{
	SelectObject(hdc, GiraffePen);
	for (int i = 0; i < Projectiles.Size(); ++i) {
		Projectiles[i].Draw(Projectiles[i], *this, hdc, Scale);
	}

	int CurrentAnim = 0;
	int CurrentFrame = 0;


	if (State & STATE_INTANGIBLE) {

		SelectObject(hdc, IntangiblePen);
	}


	if ((State & STATE_HEAVY) && (State & STATE_JUMPING)) {
		if (State & STATE_UP) {
			if (AnimFrame >= 10 && AnimFrame <= 30) {
				POINT points[NUM_POINTS];
				for (int i = 0; i < NUM_POINTS; ++i) {
					points[i] = Giraffe::VecToPoint(Position + Facing * (*Moves->GetSkelPoints(CurrentAnim, CurrentFrame))[i], Scale);
				}
				Polyline(hdc, points, 27);
				Polyline(hdc, &points[36], 2);
			}
			else {
				DrawSelf(hdc, Scale, AnimFrame, AttackNum);
			}
		}
		else if (State & STATE_DOWN) {
			DrawSelf(hdc, Scale, AnimFrame, AttackNum);
			SelectObject(hdc, ShineBrush);
			POINT points[12];
			float r = 2.5f * cosf(AnimFrame / 15.0f * 3.1415f);
			float r1 = 0.5f * r;
			float r2 = 0.866025f * r;
			points[0] = Giraffe::VecToPoint(Position + Vector2(r, 0), Scale);
			points[1] = Giraffe::VecToPoint(Position + Vector2(r1, r2), Scale);
			points[2] = Giraffe::VecToPoint(Position + Vector2(-r1, r2), Scale);
			points[3] = Giraffe::VecToPoint(Position + Vector2(-r, 0), Scale);
			points[4] = Giraffe::VecToPoint(Position + Vector2(-r1, -r2), Scale);
			points[5] = Giraffe::VecToPoint(Position + Vector2(r1, -r2), Scale);

			r *= 0.6f;
			r1 *= 0.6f;
			r2 *= 0.6f;
			points[6] = Giraffe::VecToPoint(Position + Vector2(r, 0), Scale);
			points[7] = Giraffe::VecToPoint(Position + Vector2(r1, r2), Scale);
			points[8] = Giraffe::VecToPoint(Position + Vector2(-r1, r2), Scale);
			points[9] = Giraffe::VecToPoint(Position + Vector2(-r, 0), Scale);
			points[10] = Giraffe::VecToPoint(Position + Vector2(-r1, -r2), Scale);
			points[11] = Giraffe::VecToPoint(Position + Vector2(r1, -r2), Scale);

			Polygon(hdc, points, 12);
		}
		else {
			DrawSelf(hdc, Scale, AnimFrame, AttackNum);
		}
		return;
	}

	if (State & STATE_WEAK && !(State & STATE_JUMPING)) {
		DrawSelf(hdc, Scale, AnimFrame, AttackNum);
		if (State & STATE_FORWARD && (AnimFrame >= 5 && AnimFrame <= 20)) {
			DrawAxe(hdc, Scale, ((*Moves->GetSkelPoints(19, AnimFrame))[25] + (*Moves->GetSkelPoints(19, AnimFrame))[30]) * 0.5f, ((*Moves->GetSkelPoints(19, AnimFrame))[27] + (*Moves->GetSkelPoints(19, AnimFrame))[28]) * 0.5f);
			return;
		}
		else if (State & STATE_UP && (AnimFrame >= 14)) {
			DrawMace(hdc, Scale, ((*Moves->GetSkelPoints(20, AnimFrame))[25] + (*Moves->GetSkelPoints(20, AnimFrame))[30]) * 0.5f, ((*Moves->GetSkelPoints(20, AnimFrame))[27] + (*Moves->GetSkelPoints(20, AnimFrame))[28]) * 0.5f);
			return;
		}
		else if (State & STATE_DOWN && (AnimFrame >= 5 && AnimFrame <= 20)) {
			DrawSword(hdc, Scale, ((*Moves->GetSkelPoints(21, AnimFrame))[25] + (*Moves->GetSkelPoints(21, AnimFrame))[30]) * 0.5f, ((*Moves->GetSkelPoints(21, AnimFrame))[27] + (*Moves->GetSkelPoints(21, AnimFrame))[28]) * 0.5f);
			return;
		}
		else {
			return;
		}
	}

	if (State & (STATE_WEAK | STATE_HEAVY | STATE_THROW)) {
		CurrentAnim = AttackNum;
		CurrentFrame = AnimFrame;
		/*for (int i = 0; i < numHitboxes; ++i) {
			DrawHitbox(hdc, Scale, (*Hitboxes)[i].Position, (*Hitboxes)[i].Radius);
		}*/
	}
	else {
		if (State & STATE_HITSTUN) {
			CurrentAnim = 6;
			CurrentFrame = AnimFrame % 8;
		}
		else if (State & STATE_SHIELDSTUN) {
			SelectObject(hdc, ShieldBrush);
			DrawHitbox(hdc, Scale, { 0,0 }, 2.5f);
			//Ellipse(hdc, (Position.x - 2.5f) * Scale.x, (Position.y - 2.5f) * Scale.y, (Position.x + 2.5f) * Scale.x, (Position.y + 2.5f) * Scale.y);
			CurrentAnim = 6;
			CurrentFrame = AnimFrame % 8;
		}
		else if (State & STATE_SHIELDING) {
			SelectObject(hdc, ShieldBrush);
			DrawHitbox(hdc, Scale, { 0,0 }, 2.5f);
			//Ellipse(hdc, (Position.x - 2.5f) * Scale.x, (Position.y - 2.5f) * Scale.y, (Position.x + 2.5f) * Scale.x, (Position.y + 2.5f) * Scale.y);
			CurrentAnim = 0;
			CurrentFrame = 0;
		}
		else if (State & STATE_RUNNING) {
			CurrentAnim = 1;
			CurrentFrame = AnimFrame % 2;
		}
		else if (State & STATE_JUMPING) {
			CurrentAnim = 2;
			CurrentFrame = 0;
		}
		else if (State & STATE_JUMPSQUAT) {
			CurrentAnim = 3;
			CurrentFrame = AnimFrame;
		}
		else if (State & STATE_JUMPLAND) {
			CurrentAnim = 4;
			CurrentFrame = AnimFrame;
		}
		else if (State & STATE_KNOCKDOWNLAG) {
			CurrentAnim = 5;
			CurrentFrame = AnimFrame;
		}
		else if (State & STATE_KNOCKDOWN) {
			CurrentAnim = 5;
			CurrentFrame = 30;
		}
		else if (State & STATE_LEDGEHOG) {
			CurrentAnim = 6;
			CurrentFrame = AnimFrame % 8;
		}
		else if (State & (STATE_WAVEDASH | STATE_CROUCH)) {
			CurrentAnim = 7;
			CurrentFrame = 0;
		}
		else if (State & STATE_ROLLING) {
			CurrentAnim = 8;
			CurrentFrame = AnimFrame;
		}
		else if (State & STATE_GRABBED) {
			CurrentAnim = 6;
			CurrentFrame = AnimFrame % 8;
		}
		else if (State & STATE_GRABBING) {
			CurrentAnim = 12;
			CurrentFrame = 7;
		}
	}
	DrawSelf(hdc, Scale, CurrentFrame, CurrentAnim);
}

void RobotGiraffe::DrawSelf(HDC hdc, Vector2 Scale, int CurrentFrame, int CurrentAnim)
{
	std::vector<POINT> points;
	std::vector<Vector2> vPoints = (*Moves->GetSkelPoints(CurrentAnim, CurrentFrame));

	for (int i = 0; i < vPoints.size(); ++i) {
		points.push_back(Giraffe::VecToPoint(Position + Facing * vPoints[i], Scale));
	}
	Polyline(hdc, &points[0], points.size());
}

void RobotGiraffe::DrawHitbox(HDC hdc, Vector2 Scale, Vector2 Pos, float Rad)
{
	Ellipse(hdc, (int)(Scale.x * (Position.x + Facing.x * Pos.x - Rad)), (int)(Scale.y * (Position.y + Facing.y * Pos.y - Rad)), (int)(Scale.x * (Position.x + Facing.x * Pos.x + Rad)), (int)(Scale.y * (Position.y + Facing.y * Pos.y + Rad)));
}

void RobotGiraffe::DrawAxe(HDC hdc, Vector2 Scale, Vector2 Neck, Vector2 Head)
{
	Vector2 dir = Head - Neck;
	dir.Normalize();
	Vector2 perp = { -dir.y, dir.x };
	Vector2 Pos = Head + dir;

	POINT points[8];
	points[0] = Giraffe::VecToPoint(Position + Facing * Head, Scale);
	points[1] = Giraffe::VecToPoint(Position + Facing * Pos, Scale);
	points[2] = Giraffe::VecToPoint(Position + Facing * (Pos + 0.3f * perp + -0.3f * dir), Scale);
	points[3] = Giraffe::VecToPoint(Position + Facing * (Pos + 0.3f * perp + 0.4f * dir), Scale);
	points[4] = Giraffe::VecToPoint(Position + Facing * (Pos + 0.1f * dir), Scale);
	points[5] = Giraffe::VecToPoint(Position + Facing * (Pos + -0.3f * perp + 0.4f * dir), Scale);
	points[6] = Giraffe::VecToPoint(Position + Facing * (Pos + -0.3f * perp + -0.3f * dir), Scale);
	points[7] = Giraffe::VecToPoint(Position + Facing * Pos, Scale);

	Polyline(hdc, points, 8);
}

void RobotGiraffe::DrawSword(HDC hdc, Vector2 Scale, Vector2 Neck, Vector2 Head)
{
	Vector2 dir = Head - Neck;
	dir.Normalize();
	Vector2 perp = { -dir.y, dir.x };
	Vector2 Pos = Head + dir * 0.3f;

	POINT points[6];
	points[0] = Giraffe::VecToPoint(Position + Facing * Head, Scale);
	points[1] = Giraffe::VecToPoint(Position + Facing * Pos, Scale);
	points[2] = Giraffe::VecToPoint(Position + Facing * (Pos + perp * 0.2f), Scale);
	points[3] = Giraffe::VecToPoint(Position + Facing * (Pos - perp * 0.2f), Scale);
	points[4] = Giraffe::VecToPoint(Position + Facing * Pos, Scale);
	points[5] = Giraffe::VecToPoint(Position + Facing * (Head + dir * 1.5f), Scale);

	Polyline(hdc, points, 6);
}

void RobotGiraffe::DrawMace(HDC hdc, Vector2 Scale, Vector2 Neck, Vector2 Head)
{
	Vector2 dir = Head - Neck;
	dir.Normalize();
	Vector2 perp = { -dir.y, dir.x };
	Vector2 Pos = Head + dir;

	POINT points[11];

	points[0] = Giraffe::VecToPoint(Position + Facing * Head, Scale);
	points[1] = Giraffe::VecToPoint(Position + Facing * Pos, Scale);
	points[2] = Giraffe::VecToPoint(Position + Facing * (Pos + perp * 0.3f), Scale);
	points[3] = Giraffe::VecToPoint(Position + Facing * (Pos + perp * 0.3f + dir * 0.6f), Scale);
	points[4] = Giraffe::VecToPoint(Position + Facing * (Pos + perp * -0.3f + dir * 0.6f), Scale);
	points[5] = Giraffe::VecToPoint(Position + Facing * (Pos + perp * -0.3f), Scale);
	points[6] = Giraffe::VecToPoint(Position + Facing * Pos, Scale);
	points[7] = Giraffe::VecToPoint(Position + Facing * (Pos + perp * 0.3f + dir * 0.3f), Scale);
	points[8] = Giraffe::VecToPoint(Position + Facing * (Pos + dir * 0.6f), Scale);
	points[9] = Giraffe::VecToPoint(Position + Facing * (Pos + perp * -0.3f + dir * 0.3f), Scale);
	points[10] = Giraffe::VecToPoint(Position + Facing * Pos, Scale);

	Polyline(hdc, points, 11);
}
