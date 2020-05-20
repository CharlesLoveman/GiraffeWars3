#ifndef _COLLIDER_H_
#define _COLLIDER_H_

#include "Vec2.h"

struct Collider {
	Vec2 Position;
	float Radius;
	friend bool Intersect(Vec2 pos1, Collider col1, Vec2 fac1, Vec2 pos2, Collider col2, Vec2 fac2);
};


struct HitCollider : public Collider {
	HitCollider();
	HitCollider(Vec2 Pos, float Rad, Vec2 Frc, float Dmg, float Knk, float Scl, bool Fix);
	Vec2 Force;
	float Damage;
	float Knockback;
	float Scale;
	bool Fixed;
};

struct HurtCollider : public Collider {
	HurtCollider();
	HurtCollider(Vec2 Pos, float Rad, float DmgMult);
	float DamageMultiplier;
};

#endif // !_COLLIDER_H_

