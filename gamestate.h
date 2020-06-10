#ifndef _GAMESTATE_H_
#define _GAMESTATE_H_
#include "Giraffe.h"
#include "NormGiraffe.h"
#include "RobotGiraffe.h"
#include "CoolGiraffe.h"
#include "Stage.h"
#include "MoveSet.h"
#include <memory>
#include <array>
#include "Line.h"

//Structure to be serialized at each frame
//Needs to be small

struct GameState
{
public:
	void Init(HWND hwnd, int num_players);
	void Update(int inputs[], int disconnect_flags);

	int state;
	int selectors[GGPO_MAX_PLAYERS];
	bool selected[GGPO_MAX_PLAYERS];
	int selectDelay[GGPO_MAX_PLAYERS];
	int _framenumber;
	int _num_giraffes;
	RECT _bounds;
	std::array<Giraffe*, GGPO_MAX_PLAYERS> giraffes;
	Stage stage;
	std::vector<Line> lines;
private:
	void CreateGiraffes();
	bool ParseCharSelectInputs(int inputs[]);
};
#endif // !_GAMESTATE_H_

