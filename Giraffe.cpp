#include "Giraffe.h"
#include "Collider.h"

bool Giraffe::AddHit(HitCollider hit, int ID, Vector2 facing2, Vector2 position2)
{
	if (numIncoming < 8 && !PrevHitQueue.Contains(ID)) {
		if (Intersect(position2, hit, facing2, Position, Fullbody, Facing)) {
			for (int k = 0; k < 6; ++k) {
				if (Intersect(position2, hit, facing2, Position, (*Hurtboxes)[k], Facing) && !PrevHitQueue.Contains(ID)) {
					IncomingHits[numIncoming] = hit;
					IncomingHits[numIncoming].Force *= facing2;
					++numIncoming;
					PrevHitQueue.Push(ID);
					return true;
				}
			}
		}
	}
	return false;
}

bool Giraffe::ProjectileHit(Projectile p)
{
	if (numIncoming < 8 && !PrevHitQueue.Contains(p.ID)) {
		if (Vector2::DistanceSquared(Position, p.Position) < (p.Radius * p.Radius + Fullbody.Radius * Fullbody.Radius)) {
			for (int k = 0; k < 6; ++k) {
				if (Vector2::DistanceSquared((Position + Facing * (*Hurtboxes)[k].Position), p.Position) < (p.Radius * p.Radius + (*Hurtboxes)[k].Radius * (*Hurtboxes)[k].Radius)) {
					IncomingHits[numIncoming] = p;
					++numIncoming;
					PrevHitQueue.Push(p.ID);
					return true;
				}
			}
		}
	}
	return false;
}

bool Giraffe::GrabHit(Collider col, Vector2 _Facing, int frameNumber)
{
	if (State & (STATE_GRABBED | STATE_INTANGIBLE) || incomingGrab) {
		return false;
	}
	
	if (Vector2::DistanceSquared(col.Position, Position) < (col.Radius * col.Radius + Fullbody.Radius * Fullbody.Radius)) {
		for (int k = 0; k < 6; ++k) {
			if (Vector2::DistanceSquared(col.Position, (Position + Facing * (*Hurtboxes)[k].Position)) < (col.Radius * col.Radius + (*Hurtboxes)[k].Radius * (*Hurtboxes)[k].Radius)) {
				incomingGrab = true;
				Facing.x = -1 * _Facing.x;
				Position.x = col.Position.x - 1.0f * Facing.x;
				Velocity = { 0,0 };
				TechDelay = frameNumber + 30;
				return true;
			}
		}
	}

	return false;
}

POINT Giraffe::VecToPoint(Vector2 vec, Vector2 scale)
{
	return { (int)(vec.x * scale.x), (int)(vec.y * scale.y) };
}

void Giraffe::Move(Stage& stage, const int frameNumber, std::array<Giraffe*, 4> giraffes)
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
				State |= STATE_SHIELDSTUN;
				AttackDelay = (int)max(AttackDelay, frameNumber + IncomingHits[j].Damage * 50);
			}
		}
		else {
			for (int j = 0; j < numIncoming; ++j) {
				Knockback += IncomingHits[j].Damage;
				float KnockbackApplied;
				if (IncomingHits[j].Fixed) {
					KnockbackApplied = IncomingHits[j].Knockback;
				}
				else {
					KnockbackApplied = ((((Knockback / 10 + (Knockback * IncomingHits[j].Damage / 20)) * (200 / (Mass + 100)) * 1.4f + 0.18f) * IncomingHits[j].Scale) + IncomingHits[j].Knockback);
				}
				Velocity += KnockbackApplied * IncomingHits[j].Force;
				if (State & STATE_LEDGEHOG) {
					stage.Ledges[LedgeID].Hogged = false;
					State &= ~STATE_LEDGEHOG;
				}
				State &= ~(STATE_UP | STATE_BACK | STATE_DOWN | STATE_FORWARD | STATE_WEAK | STATE_HEAVY | STATE_JUMPSQUAT | STATE_JUMPLAND | STATE_SHORTHOP | STATE_KNOCKDOWN | STATE_KNOCKDOWNLAG | STATE_GETUPATTACK | STATE_GRABBING | STATE_GRABBED | STATE_THROW);
				State |= STATE_HITSTUN;
				SoundMoveState |= SOUND_HITSTUN;
				SoundMoveDelay[XACT_WAVEBANK_MOVEBANK_HITSTUN] = 100000000;
				AttackDelay = (int)max(AttackDelay, frameNumber + min(KnockbackApplied * 40, 100));
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
					if (Velocity.x > 0) {
						Facing.x = -1;
					}
					else {
						Facing.x = 1;
					}
					Velocity = { 0,0 };
					State |= STATE_KNOCKDOWNLAG;
					State &= ~STATE_HITSTUN;
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
				State &= ~(STATE_UP | STATE_BACK | STATE_DOWN | STATE_FORWARD | STATE_WEAK | STATE_HEAVY | STATE_JUMPING | STATE_FASTFALL | STATE_TECHLAG | STATE_THROW | STATE_GRABBED | STATE_GRABBING);
			}
		}
		else if (!(State & (STATE_JUMPING | STATE_GRABBED))) {
			State &= ~(STATE_UP | STATE_BACK | STATE_DOWN | STATE_FORWARD | STATE_WEAK | STATE_HEAVY | STATE_SHIELDING | STATE_DROPSHIELD | STATE_SHIELDSTUN | STATE_JUMPSQUAT | STATE_SHORTHOP | STATE_JUMPLAND | STATE_WAVEDASH | STATE_CROUCH | STATE_GRABBING | STATE_THROW | STATE_KNOCKDOWN | STATE_RUNNING);
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

void Giraffe::TransitionStates(const int frameNumber)
{
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
}

void Giraffe::ParseInputs(const int inputs, const int frameNumber, Stage& stage)
{
	if (!(State & (STATE_JUMPSQUAT | STATE_JUMPLAND | STATE_DROPSHIELD | STATE_SHIELDSTUN | STATE_KNOCKDOWNLAG | STATE_ROLLING | STATE_GRABBED | STATE_THROW))) {
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
				LastAttackID++;
				SoundAttackState |= SOUND_FTHROW;
				SoundAttackDelay[XACT_WAVEBANK_ATTACKBANK_FTHROW] = frameNumber + Moves->GetAttackSoundLength(XACT_WAVEBANK_ATTACKBANK_FTHROW);
			}
			else if (inputs & INPUT_UP) {
				State &= ~STATE_GRABBING;
				State |= STATE_UP | STATE_THROW;
				AttackNum = 14;
				AttackDelay = frameNumber + Moves->GetMoveLength(AttackNum);
				AnimFrame = 0;
				LastAttackID++;
				SoundAttackState |= SOUND_UPTHROW;
				SoundAttackDelay[XACT_WAVEBANK_ATTACKBANK_UPTHROW] = frameNumber + Moves->GetAttackSoundLength(XACT_WAVEBANK_ATTACKBANK_UPTHROW);
			}
			else if (inputs & INPUT_DOWN) {
				State &= ~STATE_GRABBING;
				State |= STATE_DOWN | STATE_THROW;
				AttackNum = 15;
				AttackDelay = frameNumber + Moves->GetMoveLength(AttackNum);
				AnimFrame = 0;
				LastAttackID++;
				SoundAttackState |= SOUND_DOWNTHROW;
				SoundAttackDelay[XACT_WAVEBANK_ATTACKBANK_DOWNTHROW] = frameNumber + Moves->GetAttackSoundLength(XACT_WAVEBANK_ATTACKBANK_DOWNTHROW);
			}
			else if ((inputs & INPUT_LEFT && Facing.x == 1) || (inputs & INPUT_RIGHT && Facing.x == -1)) {
				State &= ~STATE_GRABBING;
				State |= STATE_BACK | STATE_THROW;
				AttackNum = 16;
				AttackDelay = frameNumber + Moves->GetMoveLength(AttackNum);
				AnimFrame = 0;
				LastAttackID++;
				SoundAttackState |= SOUND_BACKTHROW;
				SoundAttackDelay[XACT_WAVEBANK_ATTACKBANK_BACKTHROW] = frameNumber + Moves->GetAttackSoundLength(XACT_WAVEBANK_ATTACKBANK_BACKTHROW);
			}
		}
		else if (State & STATE_HITSTUN) {
			if (State & STATE_JUMPING) {
				if (inputs & INPUT_LEFT && Velocity.x > -MaxAirSpeed.x) {
					Velocity.x -= AirAccel / 10.0f;
				}
				else if (inputs & INPUT_RIGHT && Velocity.x < MaxAirSpeed.x) {
					Velocity.x += AirAccel / 10.0f;
				}
				if (inputs & INPUT_UP && Velocity.y > -MaxAirSpeed.y) {
					Velocity.y -= AirAccel / 10.0f;
				}
				else if (inputs & INPUT_DOWN && Velocity.y < MaxAirSpeed.y) {
					Velocity.y += AirAccel / 10.0f;
				}
			}
		}
		else if (State & STATE_JUMPING) {
			if (inputs & INPUT_LEFT && Velocity.x > -MaxAirSpeed.x) {
				Velocity.x -= AirAccel;
			}
			else if (inputs & INPUT_RIGHT && Velocity.x < MaxAirSpeed.x) {
				Velocity.x += AirAccel;
			}
			if (inputs & INPUT_DOWN && !(inputs & (INPUT_WEAK | INPUT_HEAVY)) && !(State & STATE_FASTFALL)) {
				State |= STATE_FASTFALL;
				SoundMoveState |= SOUND_FASTFALL;
				SoundMoveDelay[XACT_WAVEBANK_MOVEBANK_FASTFALL] = frameNumber + Moves->GetMoveSoundLength(XACT_WAVEBANK_MOVEBANK_FASTFALL);
			}
		}
		else if (State & (STATE_TECHING | STATE_SHIELDING | STATE_KNOCKDOWN)) {
			if (inputs & INPUT_LEFT) {
				State &= ~(STATE_TECHING | STATE_SHIELDING | STATE_KNOCKDOWN | STATE_CROUCH);
				State |= STATE_ROLLING | STATE_INTANGIBLE;
				AttackDelay = frameNumber + 20;
				AnimFrame = 0;
				Facing = { -1, 1 };
				SoundMoveState |= SOUND_ROLL;
				SoundMoveState &= ~SOUND_SHIELD;
				SoundMoveDelay[XACT_WAVEBANK_MOVEBANK_ROLL] = frameNumber + Moves->GetMoveSoundLength(XACT_WAVEBANK_MOVEBANK_ROLL);
			}
			else if (inputs & INPUT_RIGHT) {
				State &= ~(STATE_TECHING | STATE_SHIELDING | STATE_KNOCKDOWN | STATE_CROUCH);
				State |= STATE_ROLLING | STATE_INTANGIBLE;
				AttackDelay = frameNumber + 20;
				AnimFrame = 0;
				Facing = { 1, 1 };
				SoundMoveState |= SOUND_ROLL;
				SoundMoveState &= ~SOUND_SHIELD;
				SoundMoveDelay[XACT_WAVEBANK_MOVEBANK_ROLL] = frameNumber + Moves->GetMoveSoundLength(XACT_WAVEBANK_MOVEBANK_ROLL);
			}
		}
		else if (!(State & (STATE_WEAK | STATE_HEAVY))) {
			if (inputs & INPUT_LEFT && Velocity.x > -MaxGroundSpeed) {
				Velocity.x -= RunAccel;
				State &= ~STATE_CROUCH;
				State |= STATE_RUNNING;
				Facing = { -1, 1 };
				SoundMoveState |= SOUND_RUN;
				SoundMoveDelay[XACT_WAVEBANK_MOVEBANK_RUN] = frameNumber + Moves->GetMoveSoundLength(XACT_WAVEBANK_MOVEBANK_RUN);
			}
			else if (inputs & INPUT_RIGHT && Velocity.x < MaxGroundSpeed) {
				Velocity.x += RunAccel;
				State &= ~STATE_CROUCH;
				State |= STATE_RUNNING;
				Facing = { 1, 1 };
				SoundMoveState |= SOUND_RUN;
				SoundMoveDelay[XACT_WAVEBANK_MOVEBANK_RUN] = frameNumber + Moves->GetMoveSoundLength(XACT_WAVEBANK_MOVEBANK_RUN);
			}
			else if (!(inputs & (INPUT_LEFT | INPUT_RIGHT))) {
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
	//B-Reverse
	if ((State & STATE_HEAVY) && (State & STATE_JUMPING) && AnimFrame == 1 && (((inputs & INPUT_LEFT) && Facing.x == 1) || ((inputs & INPUT_RIGHT) && Facing.x == -1))) {
		Facing.x *= -1;
	}
}

void Giraffe::ApplyChanges(std::array<Giraffe*, GGPO_MAX_PLAYERS> giraffes, const int num_giraffes, const int frameNumber, const int i)
{
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
			Hurtboxes = Moves->GetHurtboxes(6, AnimFrame % Moves->GetMoveLength(6));
		}
		else if (State & STATE_RUNNING) {
			Hurtboxes = Moves->GetHurtboxes(1, AnimFrame % Moves->GetMoveLength(1));
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
		else if (State & STATE_KNOCKDOWN) {
			Hurtboxes = Moves->GetHurtboxes(5, 30);
		}
		else if (State & STATE_KNOCKDOWNLAG) {
			Hurtboxes = Moves->GetHurtboxes(5, AnimFrame);
		}
		else if (State & STATE_LEDGEHOG) {
			Hurtboxes = Moves->GetHurtboxes(6, AnimFrame % Moves->GetMoveLength(6));
		}
		else if (State & (STATE_WAVEDASH | STATE_CROUCH)) {
			Hurtboxes = Moves->GetHurtboxes(7, 0);
		}
		else if (State & STATE_ROLLING) {
			Hurtboxes = Moves->GetHurtboxes(8, 0);
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
			Velocity.y *= 0.9f;
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
