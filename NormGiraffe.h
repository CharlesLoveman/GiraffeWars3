#ifndef _NORMGIRAFFE_H_
#define _NORMGIRAFFE_H_

#include "Giraffe.h"
#include "NormMoveSet.h"
#include <memory>
#include <array>

constexpr int NUM_MOVES_NORM = 24;
constexpr int NUM_POINTS_NORM = 38;

class NormGiraffe : public Giraffe {
public:
	NormGiraffe(Vec2 _Position, MoveSet* _Moves, HPEN _GiraffePen);
	~NormGiraffe();

	void Update(std::array<Giraffe*, 4> giraffes, const int num_giraffes, const int i, const int inputs, const int frameNumber, Stage& stage);
	void Move(Stage& stage, const int frameNumber, std::array<Giraffe*, 4> giraffes);
	void Draw(HDC hdc, Vec2 Scale);
private:
	HBRUSH SpitBrush;
	HBRUSH ShineBrush;
	int CommandGrabPointer;
	void DrawSelf(HDC hdc, Vec2 Scale, int CurrentFrame, int CurrentAnim);
	void DrawHitbox(HDC hdc, Vec2 Scale, Vec2 Pos, float Rad);
};

#endif // !_NORMGIRAFFE_H_
