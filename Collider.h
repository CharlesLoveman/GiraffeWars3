#ifndef _COLLIDER_H_
#define _COLLIDER_H_

#include "SimpleMath.h"

using namespace DirectX::SimpleMath;

struct Collider {
	Vector2 Position;
	float Radius;
	friend bool Intersect(Vector2 pos1, Collider col1, Vector2 fac1, Vector2 pos2, Collider col2, Vector2 fac2);
};


struct HitCollider : public Collider {
	HitCollider();
	HitCollider(Vector2 Pos, float Rad, Vector2 Frc, float Dmg, float Knk, float Scl, bool Fix);
	Vector2 Force;
	float Damage;
	float Knockback;
	float Scale;
	bool Fixed;
};

struct HurtCollider : public Collider {
	HurtCollider();
	HurtCollider(Vector2 Pos, float Rad, float DmgMult);
	float DamageMultiplier;
};

#endif // !_COLLIDER_H_

