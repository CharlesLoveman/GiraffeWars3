#ifndef _NORMPROJFUNCS_H_
#define _NORMPROJFUNCS_H_
#include "Giraffe.h"

struct NormProjFuncs {
	static void SpitOnHit(Projectile& self, Giraffe& parent, Giraffe* collided)
	{
		//Do nothing
	}
	static bool SpitUpdate(Projectile& self, Giraffe& parent, int frameNumber)
	{
		self.Velocity.y += parent.Gravity;
		return (frameNumber >= self.LifeSpan);
	}
	static void SpitDraw(Projectile& self, Giraffe& parent, HDC hdc, Vec2 Scale)
	{
		SelectObject(hdc, self.Pen);
		SelectObject(hdc, self.Brush);
		Ellipse(hdc, Scale.x * (self.Position.x - self.Radius), Scale.y * (self.Position.y - self.Radius), Scale.x * (self.Position.x + self.Radius), Scale.y * (self.Position.y + self.Radius));
	}
	static void NeckGrabOnHit(Projectile& self, Giraffe& parent, Giraffe* collided)
	{
		Vec2 diff = Vec2(0.2f, 0.3f) *(self.Position - parent.Position);
		if (diff.Length() > 1) {
			diff = 2.0f * diff.Normalise();
		}
		parent.Velocity += diff;
		parent.State &= ~(STATE_HEAVY | STATE_UP);
		if (collided != nullptr) {
			collided->Velocity -= diff;
		}
	}
	static bool NeckGrabUpdate(Projectile& self, Giraffe& parent, int frameNumber)
	{
		self.Velocity.y += parent.Gravity;
		return (frameNumber >= self.LifeSpan);
	}
	static void NeckGrabDraw(Projectile& self, Giraffe& parent, HDC hdc, Vec2 Scale)
	{
		SelectObject(hdc, self.Pen);
		POINT points[7];

		points[0] = (Scale * (parent.Position + parent.Facing * (*(parent.Moves)->GetSkelPoints(0, 0))[26])).ToPoint();
		points[6] = (Scale * (parent.Position + parent.Facing * (*(parent.Moves)->GetSkelPoints(0, 0))[36])).ToPoint();
		for (int i = 1; i < 6; ++i) {
			points[i] = (Scale * (self.Position + parent.Facing * (*(parent.Moves)->GetSkelPoints(0, 0))[29 + i])).ToPoint();
		}

		Polyline(hdc, points, 7);
	}
};

#endif
