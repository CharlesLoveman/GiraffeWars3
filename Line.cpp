#include "Line.h"
#include <math.h>
#include "Giraffe.h"

Line::Line(Vector2 Pos, Vector2 Vel, float Len, int Span, HPEN _Pen)
{
	Position = Pos;
	Velocity = Vel;
	Length = Len;
	LifeSpan = Span;
	Pen = _Pen;
}

bool Line::Update(int frameNumber)
{
	Velocity.y += 0.03f;
	Position += Velocity;
	return frameNumber >= LifeSpan;
}

void Line::Draw(HDC hdc, Vector2 Scale, int frameNumber)
{
	SelectObject(hdc, Pen);
	POINT points[2];
	points[0] = Giraffe::VecToPoint(Position + Length * Vector2(cosf(frameNumber / (10 * 3.14159f)), sinf(frameNumber / (10 * 3.14159f))), Scale);
	points[1] = Giraffe::VecToPoint(Position - Length * Vector2(cosf(frameNumber / (10 * 3.14159f)), sinf(frameNumber / (10 * 3.14159f))), Scale);
	Polyline(hdc, points, 2);
}
