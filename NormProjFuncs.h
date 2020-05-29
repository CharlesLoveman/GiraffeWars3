#ifndef _NORMPROJFUNCS_H_
#define _NORMPROJFUNCS_H_
#include "Giraffe.h"

struct NormProjFuncs {
	static void SpitOnHit(Projectile& self, Giraffe& parent, Giraffe* collided, int frameNumber)
	{
		//Do nothing
	}
	static bool SpitUpdate(Projectile& self, Giraffe& parent, int frameNumber)
	{
		self.Velocity.y += parent.Gravity;
		return (frameNumber >= self.LifeSpan);
	}
	static void SpitDraw(Projectile& self, Giraffe& parent, HDC hdc, Vector2 Scale)
	{
		SelectObject(hdc, self.Pen);
		SelectObject(hdc, self.Brush);
		Ellipse(hdc, Scale.x * (self.Position.x - self.Radius), Scale.y * (self.Position.y - self.Radius), Scale.x * (self.Position.x + self.Radius), Scale.y * (self.Position.y + self.Radius));
	}
	static void NeckGrabOnHit(Projectile& self, Giraffe& parent, Giraffe* collided, int frameNumber)
	{
		Vector2 diff = Vector2(0.2f, 0.3f) *(self.Position - parent.Position);
		float len = diff.Length();
		if (len > 1) {
			diff *= 2.0f / len;
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
		return (frameNumber >= self.LifeSpan) || !(parent.State & STATE_JUMPING);
	}
	static void NeckGrabDraw(Projectile& self, Giraffe& parent, HDC hdc, Vector2 Scale)
	{
		SelectObject(hdc, self.Pen);
		POINT points[7];

		points[0] = Giraffe::VecToPoint(parent.Position + parent.Facing * (*(parent.Moves)->GetSkelPoints(0, 0))[26], Scale);
		points[6] = Giraffe::VecToPoint(parent.Position + parent.Facing * (*(parent.Moves)->GetSkelPoints(0, 0))[36], Scale);
		for (int i = 1; i < 6; ++i) {
			points[i] = Giraffe::VecToPoint(self.Position + parent.Facing * (*(parent.Moves)->GetSkelPoints(0, 0))[29 + i], Scale);
		}

		Polyline(hdc, points, 7);
	}
};

#endif
