#include "GiraffeRenderer.h"

void GiraffeRenderer::DrawNorm(HDC hdc, std::array<Vec2, NUM_POINTS> Skeleton, Vec2 Position, Vec2 facing, Vec2 Scale)
{
	POINT points[NUM_POINTS];

	for (int i = 0; i < NUM_POINTS; ++i) {
		points[i] = (Scale * Position + Scale * facing * Skeleton[i]).ToPoint();
	}

	//Ellipse(hdc, (int)(Scale.x*(Position.x - 2.5)), (int)(Scale.y * (Position.y - 2.5)), (int)(Scale.x * (Position.x + 2.5)), (int)(Scale.y * (Position.y + 2.5)));
	//Polyline(hdc, points, NUM_POINTS);
	Polyline(hdc, points, 27);
	PolyBezier(hdc, &points[26], 4);
	Polyline(hdc, &points[29], 5);
	PolyBezier(hdc, &points[33], 4);
	Polyline(hdc, &points[36], 2);
}

void GiraffeRenderer::DrawCool(HDC hdc, std::array<Vec2, NUM_POINTS> Skeleton, Vec2 Position, Vec2 facing, Vec2 Scale)
{
}

void GiraffeRenderer::DrawPosh(HDC hdc, std::array<Vec2, NUM_POINTS> Skeleton, Vec2 Position, Vec2 facing, Vec2 Scale)
{
}

void GiraffeRenderer::DrawRobot(HDC hdc, std::array<Vec2, NUM_POINTS> Skeleton, Vec2 Position, Vec2 facing, Vec2 Scale)
{
}
