#include "Stage.h"

constexpr float EPSILON = 0.00001f;

bool Stage::Intersects(Vector2 pos, Collider col, bool down, bool jumping, bool falling, bool hitstun, bool& landed, bool& bounced, Vector2& facing, Vector2& offset, Vector2& deltaV, bool& hogging, int& ledgeID)
{
	Vector2 position = pos + col.Position;

	//Grab Ledge
	if (!(down) && jumping) {
		for (int l = 0; l < Ledges.size(); ++l) {
			if (!Ledges[l].Hogged) {
				if (Intersect({ 0,0 }, Ledges[l].Col, { 1,1 }, pos, col, facing)) {
					ledgeID = l;
					hogging = true;
					Ledges[l].Hogged = true;
					if (Ledges[l].Facing) {
						facing = { 1,1 };
					}
					else {
						facing = { -1,1 };
					}
					offset = Ledges[l].Col.Position - pos - facing * Vector2(1.5f * col.Radius, -col.Radius);
					deltaV = { 0,0 };
					return true;
				}
			}
		}
	}
	hogging = false;

	float bounceLim = 1.0f;
	if (hitstun) {
		bounceLim = 0.64f;
	}

	//Land on platforms
	if (falling) {
		Vector2 base = position + col.Position;
		base.x += col.Radius;
		for (int p = 0; p < Platforms.size(); ++p) {
			if (Platforms[p].Contains(base)) {
				if (down) {
					return false;
				}
				else {
					landed = true;
					offset = { 0, Platforms[p].y - col.Radius - position.y };
					if (deltaV.LengthSquared() > bounceLim) {
						deltaV.y *= -0.8f;
						bounced = true;
					}
					else {
						deltaV.y = 0;
					}
					
					return true;
				}
			}
		}
	}


	//Land on stage
	if (position.x + col.Radius < Box.y || position.x - col.Radius > Box.y + Box.width || position.y + col.Radius < Box.y || position.y - col.Radius > Box.y + Box.height) {
		return false;
	}

	if (position.y < Box.y && position.x + col.Radius > Box.x && position.x - col.Radius < Box.x + Box.width) {
		offset = { 0, Box.y - col.Radius - position.y };

		if (deltaV.LengthSquared() > bounceLim) {
			deltaV.y *= -0.8f;
			bounced = true;
		}
		else {
			deltaV.y = 0;
		}
		landed = true;
	}
	else if (position.x < Box.x) {
		offset = { Box.x - col.Radius - position.x, 0 };
		if (deltaV.LengthSquared() > bounceLim) {
			deltaV.x *= -0.8f;
			bounced = true;
		}
		else {
			deltaV.x = 0;
		}
	}
	else if (position.x > Box.x + Box.width) {
		offset = { Box.x + Box.width + col.Radius - position.x, 0 };
		if (deltaV.LengthSquared() > bounceLim) {
			deltaV.x *= -0.8f;
			bounced = true;
		}
		else {
			deltaV.x = 0;
		}
	}
	else if (position.y > Box.y + Box.height) {
		offset = { 0, Box.y + Box.height + col.Radius - position.y };
		if (deltaV.LengthSquared() > bounceLim) {
			deltaV.y *= -0.8f;
			bounced = true;
		}
		else {
			deltaV.y = 0;
		}
	}
	else {
		//Just send them up if they are in the middle
		landed = true;
		offset = { 0, Box.y - col.Radius - position.y };
		if (deltaV.LengthSquared() > bounceLim) {
			deltaV.y *= -0.8f;
			bounced = true;
		}
		else {
			deltaV.y = 0;
		}
	}

	return true;
}

bool Stage::KillProjectile(Projectile p)
{
	return !(p.Position.x + p.Radius < Box.x || p.Position.x - p.Radius > Box.x + Box.width || p.Position.y + p.Radius < Box.y || p.Position.y - p.Radius > Box.y + Box.height);
}

void Stage::Draw(HDC hdc, Vector2 Scale, HBRUSH Brush)
{
	SelectObject(hdc, Brush);
	RECT scaleBox = { (int)(Box.x * Scale.x), (int)(Box.y * Scale.y), (int)((Box.x + Box.width) * Scale.x), (int)((Box.y + Box.height) * Scale.y) };
	FillRect(hdc, &scaleBox, Brush);
	for (int p = 0; p < Platforms.size(); ++p) {
		scaleBox = { (int)(Platforms[p].x * Scale.x), (int)(Platforms[p].y * Scale.y), (int)((Platforms[p].x + Platforms[p].width) * Scale.x), (int)((Platforms[p].y + Platforms[p].height) * Scale.y) };
		FillRect(hdc, &scaleBox, Brush);
	}
}
