#include "Giraffe.h"
#include "Collider.h"

bool Giraffe::AddHit(HitCollider hit, int ID, Vec2 facing2, Vec2 position2)
{
	if (Intersect(position2, hit, facing2, Position, Fullbody, Facing)) {
		for (int k = 0; k < 6; ++k) {
			if (Intersect(position2, hit, facing2, Position, (*Hurtboxes)[k], Facing) && !PrevHitQueue.Contains(ID)) {
				IncomingHits[numIncoming].hit = hit;
				IncomingHits[numIncoming].hit.Force *= facing2;
				//IncomingHits[numIncoming].hit.Damage *= multiplier;
				IncomingHits[numIncoming].ID = ID;
				++numIncoming;
				return true;
			}
		}
	}
	return false;
}

bool Giraffe::ProjectileHit(Projectile p)
{
	if ((p.Position - Position).Length() < (p.Radius + Fullbody.Radius)) {
		for (int k = 0; k < 6; ++k) {
			if ((p.Position - (Position + (*Hurtboxes)[k].Position)).Length() < (p.Radius + (*Hurtboxes)[k].Radius)) {
				IncomingHits[numIncoming].hit = p;
				IncomingHits[numIncoming].ID = p.ID;
				++numIncoming;
				return true;
			}
		}
	}
	return false;
}

bool Giraffe::GrabHit(Collider col, Vec2 _Facing, int frameNumber)
{
	if (State & (STATE_GRABBED | STATE_INTANGIBLE) || incomingGrab) {
		return false;
	}
	
	if ((col.Position - Position).Length() < (col.Radius + Fullbody.Radius)) {
		for (int k = 0; k < 6; ++k) {
			if ((col.Position - (Position + (*Hurtboxes)[k].Position)).Length() < (col.Radius + (*Hurtboxes)[k].Radius)) {
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
