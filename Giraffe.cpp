#include "Giraffe.h"
#include "Collider.h"

void Giraffe::AddHit(HitCollider hit, int ID, Vec2 facing2, Vec2 position2)
{
	if (Intersect(position2, hit, facing2, Position, Fullbody, Facing)) {
		for (int k = 0; k < 6; ++k) {
			if (Intersect(position2, hit, facing2, Position, (*Hurtboxes)[k], Facing)) {
				IncomingHits[numIncoming].hit = hit;
				IncomingHits[numIncoming].hit.Force *= facing2;
				//IncomingHits[numIncoming].hit.Damage *= multiplier;
				IncomingHits[numIncoming].ID = ID;
				++numIncoming;
				return;
			}
		}
	}
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
