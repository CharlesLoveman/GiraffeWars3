#include "NormGiraffe.h"
#include "NormProjFuncs.h"

NormGiraffe::NormGiraffe(Vec2 _Position, MoveSet* _Moves, HPEN _GiraffePen)
{
	//Movement
	Position = _Position;
	Velocity = Vec2(0, 0);
	MaxGroundSpeed = 0.4f;
	MaxAirSpeed = Vec2(0.3f, 0.7f);
	RunAccel = 0.1f;
	AirAccel = 0.03f;
	Gravity = 0.02f;
	Facing = { 1.0f, 1.0f };

	//Jumping
	JumpSpeed = 0.5f;
	HasDoubleJump = false;
	DashSpeed = 0.4f;
	HasAirDash = true;

	//Collision
	Fullbody = { Vec2(0.0f,0.0f), 2.5f };
	StageCollider = { Vec2(0.0f,0.7f), 1.0f };
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
	Mass = 100;

	//Animation
	AnimFrame = 0;
	GiraffePen = _GiraffePen;
	IntangiblePen = CreatePen(PS_SOLID, 1, RGB(255,255,255));
	ShieldBrush = CreateHatchBrush(HS_BDIAGONAL, RGB(0, 255, 127));
	SpitBrush = CreateSolidBrush(RGB(90, 210, 180));
	ShineBrush = CreateSolidBrush(RGB(0, 255, 255));
}

NormGiraffe::~NormGiraffe()
{
}

void NormGiraffe::Update(std::array<Giraffe*, 4> giraffes, const int num_giraffes, const int i, const int inputs, const int frameNumber, Stage& stage)
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
	
	//Read Inputs
	if (!(State & (STATE_WEAK | STATE_HEAVY | STATE_JUMPSQUAT | STATE_JUMPLAND | STATE_DROPSHIELD | STATE_SHIELDSTUN | STATE_HITSTUN | STATE_KNOCKDOWNLAG | STATE_ROLLING | STATE_GRABBED | STATE_THROW))) {
		if (State & STATE_LEDGEHOG) {
			if (inputs & INPUT_DOWN) {
				State &= ~STATE_LEDGEHOG;
				stage.Ledges[LedgeID].Hogged = false;
				State |= STATE_JUMPING | STATE_DOUBLEJUMPWAIT;
				JumpDelay = frameNumber + MaxJumpDelay * 2;
			}
		}
		else if (State & STATE_GRABBING) {
			if ((inputs & INPUT_RIGHT && Facing.x == 1) || (inputs & INPUT_LEFT && Facing.x == -1)) {
				State &= ~STATE_GRABBING;
				State |= STATE_FORWARD | STATE_THROW;
				AttackNum = 13;
				AttackDelay = frameNumber + Moves->GetMoveLength(AttackNum);
				AnimFrame = 0;
			}
			else if (inputs & INPUT_UP) {
				State &= ~STATE_GRABBING;
				State |= STATE_UP | STATE_THROW;
				AttackNum = 14;
				AttackDelay = frameNumber + Moves->GetMoveLength(AttackNum);
				AnimFrame = 0;
			}
			else if (inputs & INPUT_DOWN) {
				State &= ~STATE_GRABBING;
				State |= STATE_DOWN | STATE_THROW;
				AttackNum = 15;
				AttackDelay = frameNumber + Moves->GetMoveLength(AttackNum);
				AnimFrame = 0;
			}
			else if ((inputs & INPUT_LEFT && Facing.x == 1) || (inputs & INPUT_RIGHT && Facing.x == -1)) {
				State &= ~STATE_GRABBING;
				State |= STATE_BACK | STATE_THROW;
				AttackDelay = frameNumber + 18;//Length of back throw
				AttackNum = 18;
				AnimFrame = 0;
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
			if (inputs & INPUT_DOWN && !(inputs & (INPUT_WEAK | INPUT_HEAVY))) {
				State |= STATE_FASTFALL;
			}
		}
		else{
			if (inputs & INPUT_LEFT) {
				if (State & (STATE_TECHING | STATE_SHIELDING | STATE_KNOCKDOWN)) {
					State &= ~(STATE_TECHING | STATE_SHIELDING | STATE_KNOCKDOWN);
					State |= STATE_ROLLING | STATE_INTANGIBLE;
					AttackDelay = frameNumber + 20;
					AnimFrame = 0;
					Facing = { -1, 1 };
				}
				else if (Velocity.x > -MaxGroundSpeed) {
					Velocity.x -= RunAccel;
					State &= ~STATE_CROUCH;
					State |= STATE_RUNNING;
					Facing = { -1, 1 };
				}
			}
			else if (inputs & INPUT_RIGHT) {
				if (State & (STATE_TECHING | STATE_SHIELDING | STATE_KNOCKDOWN)) {
					State &= ~(STATE_TECHING | STATE_SHIELDING | STATE_KNOCKDOWN);
					State |= STATE_ROLLING | STATE_INTANGIBLE;
					AttackDelay = frameNumber + 20;
					AnimFrame = 0;
					Facing = { 1, 1 };
				}
				else if (Velocity.x < MaxGroundSpeed) {
					Velocity.x += RunAccel;
					State &= ~STATE_CROUCH;
					State |= STATE_RUNNING;
					Facing = { 1, 1 };
				}
			}
			else {
				State &= ~STATE_RUNNING;
			}

			if (inputs & INPUT_DOWN && !(inputs & (INPUT_WEAK | INPUT_HEAVY))) {
				State |= STATE_CROUCH;
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
			State |= STATE_GETUPATTACK;
			AttackNum = 17;
			Position += Vec2(2.5f, -2.5f) * Facing;
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
			if (State & (STATE_UP | STATE_DOWN)) {
				AttackNum += 9;
			}
		}
		AttackDelay = frameNumber + Moves->GetMoveLength(AttackNum);
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
		}
		else if (!(State & STATE_JUMPING)) {
			State |= STATE_JUMPSQUAT;
			JumpDelay = frameNumber + MaxJumpDelay;
			AnimFrame = 0;
			State &= ~(STATE_SHIELDING | STATE_DROPSHIELD | STATE_RUNNING | STATE_CROUCH | STATE_KNOCKDOWN);
		}
		else if (HasDoubleJump) {
			HasDoubleJump = false;
			Velocity.y = -JumpSpeed;
			State &= ~STATE_FASTFALL;
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
	}

	if (inputs & INPUT_SHIELD) {
		if ((State & STATE_JUMPING) && !(State & (STATE_TECHLAG | STATE_TECHATTEMPT))) {
			State |= STATE_TECHATTEMPT;
			TechDelay = frameNumber + 20;
		}
		
		if (inputs & INPUT_DOWN && State & STATE_JUMPSQUAT && HasAirDash) {
			HasAirDash = false;
			State &= ~(STATE_JUMPSQUAT | STATE_SHORTHOP);
			State |= STATE_WAVEDASH;
			JumpDelay = frameNumber + 2 * MaxJumpDelay;

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
		}
	}
	else if (State & STATE_SHIELDING) {
		State &= ~STATE_SHIELDING;
		State |= STATE_DROPSHIELD;
		AttackDelay = frameNumber + MaxShieldDelay;
	}

	if (State & STATE_ROLLING) {
		Velocity.x = MaxGroundSpeed * Facing.x;
		if (AnimFrame == 10) {
			State &= ~STATE_INTANGIBLE;
		}
	}

	//Grab
	if (State & STATE_WEAK && State & STATE_HEAVY && AnimFrame == 7) {
		Collider grabCol = { {Position.x + 1.885000f * Facing.x, Position.y - 0.650000f}, 0.855862f };
		for (int j = 0; j < num_giraffes; ++j) {
			if (j != i) {
				if (giraffes[j]->GrabHit(grabCol, Facing)) {
					State &= ~(STATE_WEAK | STATE_HEAVY| STATE_RUNNING);
					State |= STATE_GRABBING;
					//GrabPointer = j;
					break;
				}
			}
		}
	}
	//B-Reverse
	if ((State & STATE_HEAVY) && (State & STATE_JUMPING) && AnimFrame == 1 && (((inputs & INPUT_LEFT) && Facing.x == 1) || ((inputs & INPUT_RIGHT) && Facing.x == -1))) {
		Facing.x *= -1;
	}
	
	//Character Move-Specific Updates
	if ((State & STATE_HEAVY) && (State & STATE_UP)) {
		//Fire neck
		if (State & STATE_JUMPING) {
			if (AnimFrame == 10) {
				Projectiles.Append(Projectile(Position + Vec2(0.2f, -1.2f), { Facing.x * 0.65f, -0.65f }, 0.3f, { 0.0f, 0.0f }, 0.1f, 0.1f, 1.0f, true, LastAttackID, AttackDelay - 10, NormProjFuncs::NeckGrabOnHit, NormProjFuncs::NeckGrabUpdate, NormProjFuncs::NeckGrabDraw, GiraffePen, SpitBrush));
			}
		}
		//Main hit of upsmash
		else if (AnimFrame == 17) {
			++LastAttackID;
		}
	}
	//Fire spit
	else if ((State & STATE_HEAVY) && !(State & (STATE_WEAK | STATE_UP | STATE_FORWARD | STATE_BACK | STATE_DOWN)) && AnimFrame == 13) {
		Projectiles.Append(Projectile(Position + Vec2(0.2f, -1.2f), { Facing.x * 0.5f, 0.0f }, 0.3f, { Facing.x, 0.0f }, 0.1f, 0.1f, 1.0f, true, LastAttackID, frameNumber + 100, NormProjFuncs::SpitOnHit, NormProjFuncs::SpitUpdate, NormProjFuncs::SpitDraw, GiraffePen, SpitBrush));
	}
	//Reverse direction in bair
	else if ((State & STATE_JUMPING) && (State & STATE_WEAK) && (State & STATE_BACK) && AnimFrame == 7) {
		Facing.x *= -1;
	}
	//Spin like a maniac in get-up attack
	else if ((State & STATE_GETUPATTACK) && (7 <= AnimFrame) && (AnimFrame <= 10)) {
		Facing.x *= -1;
	}


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
			Hurtboxes = Moves->GetHurtboxes(6, AnimFrame % 9);
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
			Hurtboxes = Moves->GetHurtboxes(6, AnimFrame % 9);
		}
		else if (State & STATE_GRABBING) {
			Hurtboxes = Moves->GetHurtboxes(12, 7); //Should be 12
		}
		else { //Idle
			Hurtboxes = Moves->GetHurtboxes(0,0);
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
					Projectiles[p].OnHit(Projectiles[p], *this, giraffes[j]);
					Projectiles.Remove(p);
					break;
				}
			}
		}
	}


	if (!(Hitboxes == nullptr)) {
		for (int j = 0; j < num_giraffes; ++j) {
			if (j != i) {
				for (int h = 0; h < numHitboxes; ++h) {
					(*giraffes[j]).AddHit((*Hitboxes)[h], LastAttackID, Facing, Position);
				}
			}
		}
	}

}

void NormGiraffe::Move(Stage& stage, const int frameNumber)
{
	//Recieve incoming hits
	if (!(State & STATE_INTANGIBLE)) {
		if (incomingGrab) {
			State &= ~(STATE_UP | STATE_BACK | STATE_DOWN | STATE_FORWARD | STATE_WEAK | STATE_HEAVY | STATE_SHIELDING | STATE_DROPSHIELD | STATE_SHIELDSTUN | STATE_JUMPSQUAT | STATE_JUMPLAND | STATE_HITSTUN | STATE_FASTFALL | STATE_DOUBLEJUMPWAIT | STATE_WAVEDASH | STATE_RUNNING | STATE_SHORTHOP | STATE_CROUCH | STATE_TECHATTEMPT | STATE_TECHLAG | STATE_TECHING | STATE_KNOCKDOWN | STATE_KNOCKDOWNLAG | STATE_ROLLING | STATE_GETUPATTACK | STATE_GRABBING | STATE_THROW);
			State |= STATE_GRABBED;
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
			Projectiles[i].OnHit(Projectiles[i], *this, nullptr);
			Projectiles.Remove(i);
		};
	}
	
	Position += Velocity;
	//Correct intersection with the stage
	bool landed = false;
	bool hogging = false;
	bool bounced = false;
	Vec2 offset;
	if (!(State & STATE_LEDGEHOG)) {
		if (stage.Intersects(Position, StageCollider, State & (STATE_CROUCH | STATE_FASTFALL), State & STATE_JUMPING, Velocity.y > -0.00001, State & STATE_HITSTUN, landed, bounced, Facing, offset, Velocity, hogging, LedgeID)) {
			Position += offset;
			if (hogging) {
				State &= ~(STATE_UP | STATE_BACK | STATE_DOWN | STATE_FORWARD | STATE_WEAK | STATE_HEAVY | STATE_JUMPING | STATE_FASTFALL | STATE_GETUPATTACK | STATE_GRABBING | STATE_GRABBED | STATE_THROW);
				State |= STATE_LEDGEHOG;
			}
			else if (bounced) {
				if (State & STATE_TECHATTEMPT) {
					Velocity = { 0,0 };
					State &= ~STATE_TECHATTEMPT;
					State |= STATE_TECHING | STATE_INTANGIBLE;
					TechDelay = frameNumber + 5;
				}
			}
			else if (landed && (State & STATE_JUMPING)) {
				if (State & STATE_HITSTUN && !(State & STATE_TECHATTEMPT)) {
					Velocity = { 0,0 };
					State |= STATE_KNOCKDOWNLAG;
					AnimFrame = 0;
					TechDelay = frameNumber + 30;
				}
				else {
					if (State & (STATE_WEAK | STATE_HEAVY)) {
						State |= STATE_HITSTUN;
						AttackDelay = Moves->GetLandingLag(max(0,(AttackNum - 25)));
					}
					else if ((State & STATE_TECHATTEMPT) && (State & STATE_HITSTUN)) {
						State &= ~STATE_TECHATTEMPT;
						State |= STATE_TECHING | STATE_INTANGIBLE;
						TechDelay = frameNumber + 20;
					}

					State |= STATE_JUMPLAND;
					HasAirDash = true;
					HasDoubleJump = false;
					JumpDelay = frameNumber + MaxJumpDelay / 2;
					AnimFrame = 0;
				}
				State &= ~(STATE_UP | STATE_BACK | STATE_DOWN | STATE_FORWARD | STATE_WEAK | STATE_HEAVY | STATE_JUMPING | STATE_FASTFALL | STATE_HITSTUN | STATE_TECHLAG | STATE_THROW);
			}
		}
		else if (!(State & (STATE_JUMPING | STATE_GRABBED))) {
			State &= ~(STATE_UP | STATE_BACK | STATE_DOWN | STATE_FORWARD | STATE_WEAK | STATE_HEAVY | STATE_SHIELDING | STATE_DROPSHIELD | STATE_SHIELDSTUN | STATE_JUMPSQUAT | STATE_SHORTHOP | STATE_JUMPLAND | STATE_WAVEDASH | STATE_CROUCH | STATE_GRABBING | STATE_GRABBED | STATE_THROW);
			State |= STATE_JUMPING | STATE_DOUBLEJUMPWAIT;
			JumpDelay = frameNumber + MaxJumpDelay * 2;
		}
	}


	//Make giraffes loop around like pac-man
	//Remove this later
	if (Position.y > 60) {
		Position.y = 0;
	}
	else if (Position.y < 0) {
		Position.y = 60;
	}
	if (Position.x > 60) {
		Position.x = 0;
	}
	else if (Position.x < 0) {
		Position.x = 60;
	}

	//This is the last stateful part of the update loop

}

void NormGiraffe::Draw(HDC hdc, Vec2 Scale)
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
					points[i] = (Scale * (Position + Facing * (*Moves->GetSkelPoints(CurrentAnim, CurrentFrame))[i])).ToPoint();
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
			points[0] = (Scale * (Position + Vec2(r, 0))).ToPoint();
			points[1] = (Scale * (Position + Vec2(r1, r2))).ToPoint();
			points[2] = (Scale * (Position + Vec2(-r1, r2))).ToPoint();
			points[3] = (Scale * (Position + Vec2(-r, 0))).ToPoint();
			points[4] = (Scale * (Position + Vec2(-r1, -r2))).ToPoint();
			points[5] = (Scale * (Position + Vec2(r1, -r2))).ToPoint();

			r *= 0.6f;
			r1 *= 0.6f;
			r2 *= 0.6f;
			points[6] = (Scale * (Position + Vec2(r, 0))).ToPoint();
			points[7] = (Scale * (Position + Vec2(r1, r2))).ToPoint();
			points[8] = (Scale * (Position + Vec2(-r1, r2))).ToPoint();
			points[9] = (Scale * (Position + Vec2(-r, 0))).ToPoint();
			points[10] = (Scale * (Position + Vec2(-r1, -r2))).ToPoint();
			points[11] = (Scale * (Position + Vec2(r1, -r2))).ToPoint();

			Polygon(hdc, points, 12);
		}
		else {
			DrawSelf(hdc, Scale, AnimFrame, AttackNum);
		}
		return;
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
			CurrentFrame = AnimFrame % 9;
		}
		else if (State & STATE_SHIELDSTUN) {
			SelectObject(hdc, ShieldBrush);
			DrawHitbox(hdc, Scale, { 0,0 }, 2.5f);
			//Ellipse(hdc, (Position.x - 2.5f) * Scale.x, (Position.y - 2.5f) * Scale.y, (Position.x + 2.5f) * Scale.x, (Position.y + 2.5f) * Scale.y);
			CurrentAnim = 6;
			CurrentFrame = AnimFrame % 9;
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
			CurrentFrame = AnimFrame % 9;
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
			CurrentFrame = AnimFrame % 9;
		}
		else if (State & STATE_GRABBING) {
			CurrentAnim = 12;
			CurrentFrame = 7;
		}
	}
	DrawSelf(hdc, Scale, CurrentFrame, CurrentAnim);
}

void NormGiraffe::DrawSelf(HDC hdc, Vec2 Scale, int CurrentFrame, int CurrentAnim)
{
	POINT points[NUM_POINTS];

	for (int i = 0; i < NUM_POINTS; ++i) {
		points[i] = (Scale * (Position + Facing * (*Moves->GetSkelPoints(CurrentAnim, CurrentFrame))[i])).ToPoint();
	}

	Polyline(hdc, points, 27);
	PolyBezier(hdc, &points[26], 4);
	Polyline(hdc, &points[29], 5);
	PolyBezier(hdc, &points[33], 4);
	Polyline(hdc, &points[36], 2);
}

void NormGiraffe::DrawHitbox(HDC hdc, Vec2 Scale, Vec2 Pos,float Rad)
{
	Ellipse(hdc, Scale.x * (Position.x + Facing.x * Pos.x - Rad), Scale.y * (Position.y + Facing.y * Pos.y - Rad), Scale.x * (Position.x + Facing.x * Pos.x + Rad), Scale.y * (Position.y + Facing.y * Pos.y + Rad));
}

