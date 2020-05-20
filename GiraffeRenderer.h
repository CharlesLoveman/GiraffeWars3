#ifndef _GIRAFFE_RENDERER_H_
#define _GIRAFFE_RENDERER_H_

#include <Windows.h>
#include "Collider.h"
#include "MoveSet.h"

//constexpr int NUM_POINTS = 43;

class GiraffeRenderer {
public:
	static void DrawNorm(HDC hdc, std::array<Vec2, NUM_POINTS> Skeleton, Vec2 Position, Vec2 facing, Vec2 Scale);
	static void DrawCool(HDC hdc, std::array<Vec2, NUM_POINTS> Skeleton, Vec2 Position, Vec2 facing, Vec2 Scale);
	static void DrawPosh(HDC hdc, std::array<Vec2, NUM_POINTS> Skeleton, Vec2 Position, Vec2 facing, Vec2 Scale);
	static void DrawRobot(HDC hdc, std::array<Vec2, NUM_POINTS> Skeleton, Vec2 Position, Vec2 facing, Vec2 Scale);

private:
	GiraffeRenderer();
};

#endif // !_GIRAFFE_RENDERER_H_
