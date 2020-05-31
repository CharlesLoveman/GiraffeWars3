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
