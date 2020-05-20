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
