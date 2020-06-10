#ifndef _ROBOTGIRAFFE_H_
#define _ROBOTGIRAFFE_H_

#include "Giraffe.h"
#include "RobotMoveSet.h"

constexpr int CHARGELIMIT = 1000;

enum RobotState {
	ROBOTSTATE_BIGLASER = (1 << 0),
	ROBOTSTATE_HASSWORD = (1 << 1),
};

class RobotGiraffe : public Giraffe {
public:
	RobotGiraffe(Vector2 _Position, COLORREF _Colour);
	~RobotGiraffe();

	void Update(std::array<Giraffe*, 4> giraffes, const int num_giraffes, const int i, const int inputs, const int frameNumber, Stage& stage);
	void UniqueChanges(std::array<Giraffe*, 4> giraffes, const int num_giraffes, const int i, const int inputs, const int frameNumber, Stage& stage);
	void Draw(HDC hdc, Vector2 Scale, int frameNumber);
	int Size();
private:
	HPEN LaserPen;
	//HPEN FirePen;
	HBRUSH LanceBrush;
	HBRUSH RedBrush;
	int Charge;
	bool BigLaser;
	bool HasSword;
	int SwordDelay;
	void DrawSelf(HDC hdc, Vector2 Scale, int CurrentFrame, int CurrentAnim);
	void DrawHitbox(HDC hdc, Vector2 Scale, Vector2 Pos, float Rad);
	void DrawAxe(HDC hdc, Vector2 Scale, Vector2 Neck, Vector2 Head);
	void DrawSword(HDC hdc, Vector2 Scale, Vector2 Neck, Vector2 Head);
	void DrawMace(HDC hdc, Vector2 Scale, Vector2 Neck, Vector2 Head);
	void DrawBlast(HDC hdc, Vector2 Scale, Vector2 Neck, Vector2 Head);
	void DrawBeamSword(HDC hdc, Vector2 Scale, Vector2 Neck, Vector2 Head);
	void DrawML(HDC hdc, Vector2 Scale, Vector2 dir, Vector2 Body);
	void DrawDrill(HDC hdc, Vector2 Scale, Vector2 Neck, Vector2 Head);
};

#endif // !_NORMGIRAFFE_H_