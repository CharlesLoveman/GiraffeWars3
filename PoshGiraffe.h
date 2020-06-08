#ifndef _POSHGIRAFFE_H_
#define _POSHGIRAFFE_H_

#include "Giraffe.h"
#include "NormMoveSet.h"
#include <memory>
#include <array>

//constexpr int NUM_MOVES_NORM = 24;
//constexpr int NUM_POINTS_NORM = 40;

class PoshGiraffe : public Giraffe {
public:
	PoshGiraffe(Vector2 _Position, MoveSet* _Moves, COLORREF _Colour);
	~PoshGiraffe();

	void UniqueChanges(std::array<Giraffe*, 4> giraffes, const int num_giraffes, const int i, const int inputs, const int frameNumber, Stage& stage);
	void Draw(HDC hdc, Vector2 Scale, int frameNumber);
private:
	void DrawSelf(HDC hdc, Vector2 Scale, int CurrentFrame, int CurrentAnim);
	void DrawHitbox(HDC hdc, Vector2 Scale, Vector2 Pos, float Rad);
	int Hat;
	HBRUSH MonocleBrush;
	HBRUSH TieBrush;
};

#endif