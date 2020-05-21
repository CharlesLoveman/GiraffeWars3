#include "Stage.h"

constexpr float EPSILON = 0.00001f;

bool Stage::Intersects(Vec2 pos, Collider col, bool down, bool jumping, bool falling, bool& landed, Vec2& facing, Vec2& offset, Vec2& deltaV, bool& hogging, int& ledgeID)
{
	Vec2 position = pos + col.Position;

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
					offset = Ledges[l].Col.Position - pos - facing * Vec2(1.5f * col.Radius, -col.Radius);
					deltaV = { 0,0 };
					return true;
				}
			}
		}
	}
	hogging = false;

	if (falling) {
		for (int p = 0; p < Platforms.size(); ++p) {
			if (position.y < Platforms[p].top && Platforms[p].top - position.y - EPSILON < col.Radius && position.x < Platforms[p].right && position.x > Platforms[p].left) {
				if (down) {
					return false;
				}
				else {
					landed = true;
					offset = { 0, Platforms[p].top - col.Radius - position.y };
					return true;
				}
			}
		}
	}



	if (position.x + col.Radius < Box.left || position.x - col.Radius > Box.right || position.y + col.Radius < Box.top || position.y - col.Radius > Box.bottom) {
		return false;
	}

	if (position.y < Box.top && position.x + col.Radius > Box.left && position.x - col.Radius < Box.right) {
		offset = { 0, Box.top - col.Radius - position.y };
		deltaV.y = 0;
		landed = true;
	}
	else if (position.x < Box.left) {
		offset = { Box.left - col.Radius - position.x, 0 };
		deltaV.x = 0;
	}
	else if (position.x > Box.right) {
		offset = { Box.right + col.Radius - position.x, 0 };
		deltaV.x = 0;
	}
	else if (position.y > Box.bottom) {
		offset = { 0, Box.bottom + col.Radius - position.y };
		deltaV.y = 0;
	}
	else {
		//Just send them up if they are in the middle
		landed = true;
		offset = { 0, Box.top - col.Radius - position.y };
		deltaV.y = 0;
	}

	return true;
}

void Stage::Draw(HDC hdc, Vec2 Scale, HBRUSH Brush)
{
	SelectObject(hdc, Brush);
	FillRect(hdc, &Box.ToRect(Scale), Brush);
	for (int p = 0; p < Platforms.size(); ++p) {
		FillRect(hdc, &Platforms[p].ToRect(Scale), Brush);
	}
	/*for (int l = 0; l < Ledges.size(); ++l) {
		Ellipse(hdc, (int)(Scale.x * (Ledges[l].Col.Position.x - Ledges[l].Col.Radius)), (int)(Scale.y * (Ledges[l].Col.Position.y - Ledges[l].Col.Radius)), (int)(Scale.x * (Ledges[l].Col.Position.x + Ledges[l].Col.Radius)), (int)(Scale.y * (Ledges[l].Col.Position.y + Ledges[l].Col.Radius)));
	}*/
}
