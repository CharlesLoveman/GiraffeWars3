#ifndef _COOLPROJFUNCS_H_
#define _COOLPROJFUNCS_H_

#include "Giraffe.h"

struct CoolProjFuncs {
	static void StandardOnHit(Projectile& self, Giraffe& parent, Giraffe* collided, int frameNumber) {
		//Do nothing?
	}

	static bool StandardUpdate(Projectile& self, Giraffe& parent, int frameNumber) {
		return frameNumber >= self.LifeSpan;
	}

	static void FireballDraw(Projectile& self, Giraffe& parent, HDC hdc, Vector2 Scale, int frameNumber) {
		SelectObject(hdc, self.Pen);
		Vector2 ControlPoints[21];
		Vector2 dir = self.Velocity;
		dir.Normalize();
		Vector2 perp = { -dir.y, dir.x };

		ControlPoints[0] = self.Position - 0.75f * dir;
		ControlPoints[1] = self.Position - 0.4f * dir + 0.05f * perp;
		ControlPoints[2] = self.Position - 0.45f * dir + 0.1f * perp;
		ControlPoints[3] = self.Position - 0.35f * dir + 0.15f * perp;
		ControlPoints[4] = self.Position - 0.4f * dir + 0.2f * perp;
		ControlPoints[5] = self.Position - 0.3f * dir + 0.25f * perp;
		ControlPoints[6] = self.Position - 0.35f * dir + 0.3f * perp;
		ControlPoints[7] = self.Position + 0.5f * perp;

		ControlPoints[8] = self.Position + 0.25f * dir + 0.4f * perp;
		ControlPoints[9] = self.Position + 0.45f * dir + 0.2f * perp;
		ControlPoints[10] = self.Position + 0.5f * dir;
		ControlPoints[11] = self.Position + 0.45f * dir - 0.2f * perp;
		ControlPoints[12] = self.Position + 0.25f * dir - 0.4f * perp;

		ControlPoints[13] = self.Position - 0.5f * perp;
		ControlPoints[14] = self.Position - 0.35f * dir - 0.3f * perp;
		ControlPoints[15] = self.Position - 0.3f * dir - 0.25f * perp;
		ControlPoints[16] = self.Position - 0.4f * dir - 0.2f * perp;
		ControlPoints[17] = self.Position - 0.35f * dir - 0.15f * perp;
		ControlPoints[18] = self.Position - 0.45f * dir - 0.1f * perp;
		ControlPoints[19] = self.Position - 0.4f * dir - 0.05f * perp;
		ControlPoints[20] = self.Position - 0.75f * dir;

		POINT points[201];
		Crackle(points, ControlPoints, 20, 10, 0.3f, Scale);
		Polyline(hdc, points, 201);
	}

	static void Crackle(POINT* points, Vector2* controlPoints, const int numPoints, const int division, float factor, Vector2 Scale) {
		for (int i = 0; i < numPoints; ++i) {
			for (int t = 0; t < division; ++t) {
				points[i * division + t] = Giraffe::VecToPoint((t / (float)division) * controlPoints[i + 1] + (1.0f - t / (float)division) * controlPoints[i] + factor * Vector2(((float)rand() / RAND_MAX) - 0.5f, ((float)rand() / RAND_MAX) - 0.5f), Scale);
			}
		}
		points[numPoints * division] = Giraffe::VecToPoint(controlPoints[numPoints], Scale);
	}
};

#endif