#ifndef _COOlGIRAFFE_H_
#define _COOLGIRAFFE_H_

#include "Giraffe.h"
#include "CoolMoveSet.h"
#include <memory>
#include <array>

//constexpr int NUM_MOVES_COOL= 24;
//constexpr int NUM_POINTS_COOL = 38;

class CoolGiraffe : public Giraffe {
public:
	CoolGiraffe(Vector2 _Position, MoveSet* _Moves, COLORREF _Colour);
	~CoolGiraffe();

	void UniqueChanges(std::array<Giraffe*, 4> giraffes, const int num_giraffes, const int i, const int inputs, const int frameNumber, Stage& stage);
	void Draw(HDC hdc, Vector2 Scale, int frameNumber);
private:
	HPEN FirePen;
	void DrawSelf(HDC hdc, Vector2 Scale, int CurrentFrame, int CurrentAnim);
	void DrawHitbox(HDC hdc, Vector2 Scale, Vector2 Pos, float Rad);
	void Landing(Stage& stage, const int frameNumber, std::array<Giraffe*, GGPO_MAX_PLAYERS> giraffes);
	void RecieveHits(Stage& stage, const int frameNumber);
	void DrawSmallFlame(HDC hdc, Vector2 Scale, Vector2 Elbow, Vector2 Hand);
	void DrawLargeFlame(HDC hdc, Vector2 Scale, Vector2 Neck, Vector2 Head);
};

#endif