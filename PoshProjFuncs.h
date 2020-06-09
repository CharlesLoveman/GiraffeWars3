#ifndef _POSHPROJFUNCS_H_
#define _POSHPROJFUNCS_H_

#include "Giraffe.h"

struct PoshProjFuncs {
	static void StandardOnHit(Projectile& self, Giraffe& parent, Giraffe* collided, int frameNumber) {
		//Do nothing?
	}

	static bool StandardUpdate(Projectile& self, Giraffe& parent, int frameNumber) {
		return frameNumber >= self.LifeSpan;
	}

	static void TopHatDraw(Projectile& self, Giraffe& parent, HDC hdc, Vector2 Scale, int frameNumber)
	{
		SelectObject(hdc, self.Pen);
		Vector2 perp = self.Velocity;
		perp.Normalize();
		Vector2 dir = { 0, 1 };

		POINT points[6];
		points[0] = Giraffe::VecToPoint(self.Position + perp * 0.5f, Scale);
		points[1] = Giraffe::VecToPoint(self.Position + perp * 0.2f, Scale);
		points[2] = Giraffe::VecToPoint(self.Position + perp * 0.2f + dir * -0.75f, Scale);
		points[3] = Giraffe::VecToPoint(self.Position + perp * -0.2f + dir * -0.75f, Scale);
		points[4] = Giraffe::VecToPoint(self.Position + perp * -0.2f, Scale);
		points[5] = Giraffe::VecToPoint(self.Position + perp * -0.5f, Scale);

		Polyline(hdc, points, 6);
	}

	static void SombreroDraw(Projectile& self, Giraffe& parent, HDC hdc, Vector2 Scale, int frameNumber)
	{
		SelectObject(hdc, self.Pen);
		Vector2 perp = self.Velocity;
		perp.Normalize();
		Vector2 dir = { 0, 1 };

		POINT points[9];
		points[0] = Giraffe::VecToPoint(self.Position + perp * 0.7f + dir * -0.3f, Scale);
		points[1] = Giraffe::VecToPoint(self.Position + perp * 0.8f, Scale);
		points[2] = Giraffe::VecToPoint(self.Position + perp * 0.2f, Scale);
		points[3] = Giraffe::VecToPoint(self.Position + perp * 0.1f + dir * -0.4f, Scale);
		points[4] = Giraffe::VecToPoint(self.Position + dir * -0.5f, Scale);
		points[5] = Giraffe::VecToPoint(self.Position + perp * -0.1f + dir * -0.4f, Scale);
		points[6] = Giraffe::VecToPoint(self.Position + perp * -0.2f, Scale);
		points[7] = Giraffe::VecToPoint(self.Position + perp * -0.8f, Scale);
		points[8] = Giraffe::VecToPoint(self.Position + perp * -0.7f + dir * -0.3f, Scale);

		Polyline(hdc, points, 9);
	}

	static void RobinDraw(Projectile& self, Giraffe& parent, HDC hdc, Vector2 Scale, int frameNumber)
	{
		SelectObject(hdc, self.Pen);
		Vector2 perp = self.Velocity;
		perp.Normalize();
		Vector2 dir = { 0, -1 };

		POINT points[6];
		points[0] = Giraffe::VecToPoint(self.Position + perp * 0.2f, Scale);
		points[1] = Giraffe::VecToPoint(self.Position + perp * 0.5f, Scale);
		points[2] = Giraffe::VecToPoint(self.Position + perp * 0.45f + dir * 0.225f, Scale);
		points[3] = Giraffe::VecToPoint(self.Position + perp * -0.25f + dir * 0.225f, Scale);
		points[4] = Giraffe::VecToPoint(self.Position + perp * -0.6f, Scale);
		points[5] = Giraffe::VecToPoint(self.Position + perp * -0.2f, Scale);
		Polyline(hdc, points, 6);

		points[0] = Giraffe::VecToPoint(self.Position + perp * 0.4f + dir * 0.225f, Scale);
		points[1] = Giraffe::VecToPoint(self.Position + perp * 0.2f + dir * 0.6f, Scale);
		points[2] = Giraffe::VecToPoint(self.Position + perp * -0.18f + dir * 0.225f, Scale);
		Polyline(hdc, points, 3);
	}

	static void CrownDraw(Projectile& self, Giraffe& parent, HDC hdc, Vector2 Scale, int frameNumber)
	{
		SelectObject(hdc, self.Pen);
		Vector2 perp = self.Velocity;
		perp.Normalize();
		Vector2 dir = { 0, -1 };

		POINT points[9];
		points[0] = Giraffe::VecToPoint(self.Position + perp * 0.2f, Scale);
		points[1] = Giraffe::VecToPoint(self.Position + perp * 0.45f + dir * 0.5f, Scale);
		points[2] = Giraffe::VecToPoint(self.Position + perp * 0.3f + dir * 0.3f, Scale);
		points[3] = Giraffe::VecToPoint(self.Position + perp * 0.15f + dir * 0.6f, Scale);
		points[4] = Giraffe::VecToPoint(self.Position + dir * 0.35f, Scale);
		points[5] = Giraffe::VecToPoint(self.Position + perp * -0.15f + dir * 0.6f, Scale);
		points[6] = Giraffe::VecToPoint(self.Position + perp * -0.3f + dir * 0.3f, Scale);
		points[7] = Giraffe::VecToPoint(self.Position + perp * -0.45f + dir * 0.5f, Scale);
		points[8] = Giraffe::VecToPoint(self.Position + perp * -0.2f, Scale);

		Polyline(hdc, points, 9);
	}
};
#endif