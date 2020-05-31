#include "Giraffe.h"
#include "Collider.h"

bool Giraffe::AddHit(HitCollider hit, int ID, Vector2 facing2, Vector2 position2)
{
	if (numIncoming < 8) {
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
	}
	return false;
}

bool Giraffe::ProjectileHit(Projectile p)
{
	if (numIncoming < 8) {
		if (Vector2::DistanceSquared(Position, p.Position) < (p.Radius * p.Radius + Fullbody.Radius * Fullbody.Radius)) {
			for (int k = 0; k < 6; ++k) {
				if (Vector2::DistanceSquared((Position + Facing * (*Hurtboxes)[k].Position), p.Position) < (p.Radius * p.Radius + (*Hurtboxes)[k].Radius * (*Hurtboxes)[k].Radius)) {
					IncomingHits[numIncoming].hit = p;
					IncomingHits[numIncoming].ID = p.ID;
					++numIncoming;
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
