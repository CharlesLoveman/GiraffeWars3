#ifndef _LINE_H_
#define _LINE_H_

#include "SimpleMath.h"
#include <Windows.h>

using namespace DirectX::SimpleMath;

struct Line {
public:
	Line(Vector2 Pos, Vector2 Vel, float Len, int Span, HPEN _Pen);
	bool Update(int frameNumber);
	void Draw(HDC hdc, Vector2 Scale, int frameNumber);
private:
	Vector2 Position;
	Vector2 Velocity;
	float Length;
	int LifeSpan;
	HPEN Pen;
};

#endif // !_LINE_H_

