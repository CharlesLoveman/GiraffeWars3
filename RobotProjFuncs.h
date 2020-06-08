#ifndef _ROBOTPROJFUNCS_H_
#define _ROBOTPROJFUNCS_H_

#include "Giraffe.h"
#include <math.h>

struct RobotProjFuncs {

	static void StandardOnHit(Projectile& self, Giraffe& parent, Giraffe* collided, int frameNumber) {
		//Do nothing?
	}

	static bool StandardUpdate(Projectile& self, Giraffe& parent, int frameNumber) {
		return frameNumber >= self.LifeSpan;
	}

	static void GrabberOnHit(Projectile& self, Giraffe& parent, Giraffe* collided, int frameNumber) {
		if (collided != nullptr) {
			if (collided->GrabHit(self, parent.Facing, frameNumber)) {
				parent.State &= ~(STATE_WEAK | STATE_HEAVY | STATE_RUNNING);
				parent.State |= STATE_GRABBING;
				parent.TechDelay = frameNumber + 30;
				collided->Velocity = (parent.Position - collided->Position) * 0.1f;
			}
		}
	}

	static void GrabberDraw(Projectile& self, Giraffe& parent, HDC hdc, Vector2 Scale, int frameNumber) {

		SelectObject(hdc, self.Pen);
		std::vector<Vector2>* skel = parent.Moves->GetSkelPoints(12, 11);
		Vector2 toGrabber = (self.Position - (parent.Position + parent.Facing * ((*skel)[27] + (*skel)[28]) * 0.5f));
		toGrabber.Normalize();
		Vector2 grabPerp = { -toGrabber.y, toGrabber.x };
		POINT points[8];
		points[0] = Giraffe::VecToPoint((parent.Position + parent.Facing * ((*skel)[27] + (*skel)[28]) * 0.5f), Scale);
		points[1] = Giraffe::VecToPoint(self.Position, Scale);
		points[2] = Giraffe::VecToPoint(self.Position + toGrabber * 0.2f + grabPerp * 0.5f, Scale);
		points[3] = Giraffe::VecToPoint(self.Position + toGrabber * 0.4f + grabPerp * 0.5f, Scale);
		points[4] = Giraffe::VecToPoint(self.Position + toGrabber * 0.2f + grabPerp * 0.5f, Scale);
		points[5] = Giraffe::VecToPoint(self.Position, Scale);
		points[6] = Giraffe::VecToPoint(self.Position + toGrabber * 0.2f + grabPerp * -0.5f, Scale);
		points[7] = Giraffe::VecToPoint(self.Position + toGrabber * 0.4f + grabPerp * -0.5f, Scale);

		Polyline(hdc, points, 8);
	}

	static void MissileOnHit(Projectile& self, Giraffe& parent, Giraffe* collided, int frameNumber) {
		//Add explosion projectile to parent
		parent.Projectiles.Append(Projectile(self.Position, Vector2::Zero, 0, Vector2::Zero, 0, 0, 0, 0, self.ID, frameNumber + 20, StandardOnHit, StandardUpdate, M_ExplosionDraw, CreatePen(PS_SOLID, 1, RGB(230, 128, 40)), self.Brush));
	}

	static bool MissileUpdate(Projectile& self, Giraffe& parent, int frameNumber) {
		Vector2 newVel;
		float theta = 0.2f * ((((frameNumber + (int)abs(self.Position.y * 10)) % (((int)abs(self.Position.x) ^ self.ID) + 1)) % 3) - 1);
		newVel.x = self.Velocity.Dot({ cosf(theta), -sinf(theta) });
		newVel.y = self.Velocity.Dot({ sinf(theta), cosf(theta) });
		self.Velocity = newVel;
		if (frameNumber >= self.LifeSpan) {
			parent.Projectiles.Append(Projectile(self.Position, Vector2::Zero, 0, Vector2::Zero, 0, 0, 0, 0, self.ID, frameNumber + 20, StandardOnHit, StandardUpdate, M_ExplosionDraw, CreatePen(PS_SOLID, 1, RGB(230, 128, 40)), self.Brush));
			return true;
		}
		else {
			return false;
		}
	}

	static void MissileDraw(Projectile& self, Giraffe& parent, HDC hdc, Vector2 Scale, int frameNumber) {
		SelectObject(hdc, self.Pen);
		Vector2 dir = self.Velocity;
		dir.Normalize();
		Vector2 perp = { -dir.y, dir.x };

		POINT points[5];
		points[0] = Giraffe::VecToPoint(self.Position + dir * -0.5f + perp * 0.3f, Scale);
		points[1] = Giraffe::VecToPoint(self.Position + dir * -0.5f + perp * -0.3f, Scale);
		points[2] = Giraffe::VecToPoint(self.Position + dir * -1.0f + perp * -0.3f, Scale);
		points[3] = Giraffe::VecToPoint(self.Position + dir * -1.0f + perp * 0.3f, Scale);
		points[4] = Giraffe::VecToPoint(self.Position + dir * -0.5f + perp * 0.3f, Scale);

		Polyline(hdc, points, 5);

		points[1] = Giraffe::VecToPoint(self.Position + dir * 0.5f + perp * 0.3f, Scale);
		points[2] = Giraffe::VecToPoint(self.Position + dir * 0.75f + perp * 0.3f, Scale);
		points[3] = Giraffe::VecToPoint(self.Position + dir, Scale);

		PolyBezier(hdc, points, 4);

		points[0] = Giraffe::VecToPoint(self.Position + dir * -0.5f + perp * -0.3f, Scale);
		points[1] = Giraffe::VecToPoint(self.Position + dir * 0.5f + perp * -0.3f, Scale);
		points[2] = Giraffe::VecToPoint(self.Position + dir * 0.75f + perp * -0.3f, Scale);

		PolyBezier(hdc, points, 4);
	}

	static void M_ExplosionDraw(Projectile& self, Giraffe& parent, HDC hdc, Vector2 Scale, int frameNumber) {
		SelectObject(hdc, self.Pen);
		POINT points[17];
		Vector2 up = { 0, 1.0f };
		Vector2 right = { 1.0f, 0 };
		points[0] = Giraffe::VecToPoint(self.Position + up, Scale);
		points[1] = Giraffe::VecToPoint(self.Position + 0.25f * up + 0.125f * right, Scale);
		points[2] = Giraffe::VecToPoint(self.Position + 0.5f * up + 0.5f * right, Scale);
		points[3] = Giraffe::VecToPoint(self.Position + 0.125f * up + 0.25f * right, Scale);
		points[4] = Giraffe::VecToPoint(self.Position + right, Scale);
		points[5] = Giraffe::VecToPoint(self.Position + -0.125f * up + 0.25f * right, Scale);
		points[6] = Giraffe::VecToPoint(self.Position + -0.5f * up + 0.5f * right, Scale);
		points[7] = Giraffe::VecToPoint(self.Position + -0.25f * up + 0.125f * right, Scale);
		points[8] = Giraffe::VecToPoint(self.Position + -up, Scale);
		points[9] = Giraffe::VecToPoint(self.Position + -0.25f * up + -0.125f * right, Scale);
		points[10] = Giraffe::VecToPoint(self.Position + -0.5f * up + -0.5f * right, Scale);
		points[11] = Giraffe::VecToPoint(self.Position + -0.125f * up + -0.25f * right, Scale);
		points[12] = Giraffe::VecToPoint(self.Position + -right, Scale);
		points[13] = Giraffe::VecToPoint(self.Position + 0.125f * up + -0.25f * right, Scale);
		points[14] = Giraffe::VecToPoint(self.Position + 0.5f * up + -0.5f * right, Scale);
		points[15] = Giraffe::VecToPoint(self.Position + 0.25f * up + -0.125f * right, Scale);
		points[16] = Giraffe::VecToPoint(self.Position + up, Scale);

		Polyline(hdc, points, 17);
	}

	static void LanceDraw(Projectile& self, Giraffe& parent, HDC hdc, Vector2 Scale, int frameNumber) {
		SelectObject(hdc, self.Pen);
		SelectObject(hdc, self.Brush);
		Vector2 dir = self.Velocity;
		dir.Normalize();
		Vector2 perp = { -dir.y, dir.x };

		POINT points[7];
		points[0] = Giraffe::VecToPoint(self.Position, Scale);
		points[1] = Giraffe::VecToPoint(self.Position - 3 * dir, Scale);
		Polyline(hdc, points, 2);

		Ellipse(hdc, Scale.x * (self.Position.x + dir.x * 0.2f - 0.2f), Scale.y * (self.Position.y + dir.y * 0.2f - 0.2f), Scale.x * (self.Position.x + dir.x * 0.2f + 0.2f), Scale.y * (self.Position.y + dir.y * 0.2f + 0.2f));
		Ellipse(hdc, Scale.x * (self.Position.x + dir.x * 0.6f - 0.2f), Scale.y * (self.Position.y + dir.y * 0.6f - 0.2f), Scale.x * (self.Position.x + dir.x * 0.6f + 0.2f), Scale.y * (self.Position.y + dir.y * 0.6f + 0.2f));
		Ellipse(hdc, Scale.x * (self.Position.x + dir.x - 0.2f), Scale.y * (self.Position.y + dir.y - 0.2f), Scale.x * (self.Position.x + dir.x + 0.2f), Scale.y * (self.Position.y + dir.y + 0.2f));

		points[0] = Giraffe::VecToPoint(self.Position + dir, Scale);
		points[1] = Giraffe::VecToPoint(self.Position + 1.2f * dir + 0.1f * perp, Scale);
		points[2] = Giraffe::VecToPoint(self.Position + 1.4f * dir + 0.3f * perp, Scale);
		points[3] = Giraffe::VecToPoint(self.Position + 2.1f * dir + 0.3f * perp, Scale);
		points[4] = Giraffe::VecToPoint(self.Position + 1.9f * dir + 0.2f * perp, Scale);
		points[5] = Giraffe::VecToPoint(self.Position + 1.2f * dir + 0.2f * perp, Scale);
		points[6] = Giraffe::VecToPoint(self.Position + dir, Scale);
		Polyline(hdc, points, 7);

		points[0] = Giraffe::VecToPoint(self.Position + dir, Scale);
		points[1] = Giraffe::VecToPoint(self.Position + 1.2f * dir - 0.1f * perp, Scale);
		points[2] = Giraffe::VecToPoint(self.Position + 1.4f * dir - 0.3f * perp, Scale);
		points[3] = Giraffe::VecToPoint(self.Position + 2.1f * dir - 0.3f * perp, Scale);
		points[4] = Giraffe::VecToPoint(self.Position + 1.9f * dir - 0.2f * perp, Scale);
		points[5] = Giraffe::VecToPoint(self.Position + 1.2f * dir - 0.2f * perp, Scale);
		points[6] = Giraffe::VecToPoint(self.Position + dir, Scale);
		Polyline(hdc, points, 7);
	}

	static void SmallLaserDraw(Projectile& self, Giraffe& parent, HDC hdc, Vector2 Scale, int frameNumber) {
		SelectObject(hdc, self.Pen);
		Vector2 controlPoints[2];
		controlPoints[0] = self.Position - self.Velocity * 0.5f;
		controlPoints[1] = self.Position + self.Velocity * 0.5f;
		POINT points[11];
		Crackle(points, controlPoints, 1, 10, 0.1f, Scale);
		Polyline(hdc, points, 11);
	}

	static void BigLaserDraw(Projectile& self, Giraffe& parent, HDC hdc, Vector2 Scale, int frameNumber) {
		SelectObject(hdc, self.Pen);

		Vector2 dir = self.Velocity;
		dir.Normalize();
		Vector2 perp = { -dir.y, dir.x };

		Vector2 controlPoints[2];
		POINT points[11];
		controlPoints[0] = self.Position - dir * 3.0f + perp * 0.5f;
		controlPoints[1] = self.Position + dir * 3.0f + perp * 0.5f;
		Crackle(points, controlPoints, 1, 10, 0.4f, Scale);
		Polyline(hdc, points, 11);
		controlPoints[0] = self.Position - dir * 3.0f - perp * 0.5f;
		controlPoints[1] = self.Position + dir * 3.0f - perp * 0.5f;
		Crackle(points, controlPoints, 1, 10, 0.4f, Scale);
		Polyline(hdc, points, 11);
	}

	static void BombOnHit(Projectile& self, Giraffe& parent, Giraffe* collided, int frameNumber) {
		parent.Projectiles.Append(Projectile(self.Position, Vector2::Zero, 0, Vector2::Zero, 0, 0, 0, 0, self.ID, frameNumber + 20, StandardOnHit, StandardUpdate, B_ExplosionDraw, CreatePen(PS_SOLID, 1, RGB(230, 128, 40)), self.Brush));
	}

	static bool BombUpdate(Projectile& self, Giraffe& parent, int frameNumber) {
		self.Velocity.y += parent.Gravity;
		return frameNumber >= self.LifeSpan;
	}

	static void BombDraw(Projectile& self, Giraffe& parent, HDC hdc, Vector2 Scale, int frameNumber) {
		SelectObject(hdc, self.Pen);
		POINT points[7];
		float theta = frameNumber / 15.0f * 3.14159f;
		Vector2 up = { sinf(theta), -cosf(theta) };
		Vector2 right = { -up.y, up.x };

		//Outer box
		points[0] = Giraffe::VecToPoint(self.Position - 0.9f * right - 0.5f * up, Scale);
		points[1] = Giraffe::VecToPoint(self.Position - 0.9f * right + 0.5f * up, Scale);
		points[2] = Giraffe::VecToPoint(self.Position + 0.9f * right + 0.5f * up, Scale);
		points[3] = Giraffe::VecToPoint(self.Position + 0.9f * right - 0.5f * up, Scale);
		points[4] = Giraffe::VecToPoint(self.Position - 0.9f * right - 0.5f * up, Scale);
		Polyline(hdc, points, 5);

		//Inner box
		points[0] = Giraffe::VecToPoint(self.Position - 0.7f * right + 0.5f * up, Scale);
		points[1] = Giraffe::VecToPoint(self.Position - 0.7f * right - 0.5f * up, Scale);
		points[2] = Giraffe::VecToPoint(self.Position + 0.7f * right - 0.5f * up, Scale);
		points[3] = Giraffe::VecToPoint(self.Position + 0.7f * right + 0.5f * up, Scale);
		Polyline(hdc, points, 4);

		//N
		points[0] = Giraffe::VecToPoint(self.Position - 0.4f * right - 0.35f * up, Scale);
		points[1] = Giraffe::VecToPoint(self.Position - 0.4f * right + 0.35f * up, Scale);
		points[2] = Giraffe::VecToPoint(self.Position - 0.35f * up, Scale);
		points[3] = Giraffe::VecToPoint(self.Position + 0.35f * up, Scale);
		Polyline(hdc, points, 4);

		//2 Loop
		points[0] = Giraffe::VecToPoint(self.Position + 0.2f * right + 0.35f * up, Scale);
		points[1] = Giraffe::VecToPoint(self.Position + 0.5f * right + 0.35f * up, Scale);
		points[2] = Giraffe::VecToPoint(self.Position + 0.5f * right + 0.25f * up, Scale);
		points[3] = Giraffe::VecToPoint(self.Position + 0.2f * right + 0.1f * up, Scale);
		PolyBezier(hdc, points, 4);

		//2 Base
		points[4] = Giraffe::VecToPoint(self.Position + 0.5f * right + 0.1f * up, Scale);
		Polyline(hdc, &points[3], 2);

		//B Side
		points[0] = Giraffe::VecToPoint(self.Position + 0.2f * right, Scale);
		points[1] = Giraffe::VecToPoint(self.Position + 0.2f * right - 0.35f * up, Scale);
		Polyline(hdc, points, 2);

		//B Loops
		points[1] = Giraffe::VecToPoint(self.Position + 0.5f * right - 0.1f * up, Scale);
		points[2] = Giraffe::VecToPoint(self.Position + 0.5f * right - 0.15f * up, Scale);
		points[3] = Giraffe::VecToPoint(self.Position + 0.2f * right - 0.2f * up, Scale);
		points[4] = Giraffe::VecToPoint(self.Position + 0.5f * right - 0.25f * up, Scale);
		points[5] = Giraffe::VecToPoint(self.Position + 0.5f * right - 0.3f * up, Scale);
		points[6] = Giraffe::VecToPoint(self.Position + 0.2f * right - 0.35f * up, Scale);
		PolyBezier(hdc, points, 7);
	}

	static void B_ExplosionDraw(Projectile& self, Giraffe& parent, HDC hdc, Vector2 Scale, int frameNumber) {
		SelectObject(hdc, self.Pen);
		Vector2 controlPoints[9];
		controlPoints[0] = self.Position + Vector2(0, 0);
		controlPoints[1] = self.Position + Vector2(-0.2f, -3.0f);
		controlPoints[2] = self.Position + Vector2(-1.0f, -3.1f);
		controlPoints[3] = self.Position + Vector2(-0.2f, -3.2f);
		controlPoints[4] = self.Position + Vector2(0, -3.5f);
		controlPoints[5] = self.Position + Vector2(0.2f, -3.2f);
		controlPoints[6] = self.Position + Vector2(1.0f, -3.1f);
		controlPoints[7] = self.Position + Vector2(0.2f, -3.0f);
		controlPoints[8] = self.Position + Vector2(0, 0);

		POINT points[81];
		Crackle(points, controlPoints, 8, 10, 0.3f, Scale);
		Polyline(hdc, points, 81);
	}

	static void Crackle(POINT* points, Vector2* controlPoints, const int numPoints, const int division, float factor, Vector2 Scale) {
		for (int i = 0; i < numPoints; ++i) {
			for (int t = 0; t < division; ++t) {
				points[i * division + t] = Giraffe::VecToPoint((t / (float)division) * controlPoints[i + 1] + (1.0f - t / (float)division) * controlPoints[i] + factor * Vector2(((float)rand() / RAND_MAX) - 0.5f, ((float)rand() / RAND_MAX) - 0.5f), Scale);
			}
		}
		points[numPoints * division] = Giraffe::VecToPoint(controlPoints[numPoints], Scale);
	}

	static void SwordOnHit(Projectile& self, Giraffe& parent, Giraffe* collided, int frameNumber) {
		if (!(collided == nullptr) && !(self.Fixed)) {
			parent.Projectiles.Append(Projectile(self.Position, -self.Velocity, 1.5f, -self.Force, 0.5f, 0.5f, 0.5f, true, self.ID, frameNumber + 50, SwordOnHit, SwordUpdate, SwordDraw, self.Pen, self.Brush));
		}
	}

	static bool SwordUpdate(Projectile& self, Giraffe& parent, int frameNumber) {
		if (frameNumber >= self.LifeSpan) {
			if (!self.Fixed) {
				parent.Projectiles.Append(Projectile(self.Position, -self.Velocity, 1.5f, -self.Force, 0.5f, 0.5f, 0.5f, true, self.ID, frameNumber + 50, SwordOnHit, SwordUpdate, SwordDraw, self.Pen, self.Brush));
			}
			return true;
		}
		return false;
	}

	static void SwordDraw(Projectile& self, Giraffe& parent, HDC hdc, Vector2 Scale, int frameNumber) {
		SelectObject(hdc, self.Pen);
		float theta = frameNumber / 15.0f * 3.14159f;
		Vector2 dir = { sinf(theta), -cosf(theta) };
		Vector2 perp = { -dir.y, dir.x };
		dir.Normalize();

		POINT points[61];
		points[0] = Giraffe::VecToPoint(self.Position, Scale);
		points[1] = Giraffe::VecToPoint(self.Position - 1.5f * dir, Scale);
		points[2] = Giraffe::VecToPoint(self.Position - 1.2f * dir, Scale);
		points[3] = Giraffe::VecToPoint(self.Position - 1.2f * dir + perp * 0.2f, Scale);
		points[4] = Giraffe::VecToPoint(self.Position - 1.2f * dir - perp * 0.2f, Scale);
		Polyline(hdc, points, 5);

		Vector2 controlPoints[7];
		controlPoints[0] = self.Position - 1.2f * dir + perp * 0.2f;
		controlPoints[1] = self.Position - 0.7f * dir + perp * 0.3f;
		controlPoints[2] = self.Position + 0.5f * dir + perp * 0.2f;
		controlPoints[3] = self.Position + dir;
		controlPoints[4] = self.Position + 0.5f * dir + perp * -0.2f;
		controlPoints[5] = self.Position - 0.7f * dir + perp * -0.3f;
		controlPoints[6] = self.Position - 1.2f * dir + perp * -0.2f;

		Crackle(points, controlPoints, 6, 10, 0.2f, Scale);
		Polyline(hdc, points, 61);
	}
};

#endif // !_ROBOTPROJFUNCS_H_
