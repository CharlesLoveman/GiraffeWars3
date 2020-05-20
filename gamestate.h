#ifndef _GAMESTATE_H_
#define _GAMESTATE_H_
#include "Vec2.h"
#include "Giraffe.h"
#include "Stage.h"
#include "MoveSet.h"

//Structure to be serialized at each frame
//Needs to be small

constexpr int MAX_PLAYERS = 4;


struct GameState
{
	void Init(HWND hwnd, int num_players, const MoveSet Moves[4]);
	//void GetGiraffeAI(int i);
	//void ParseGiraffeInputs(int inputs, int i, int *direction);
	//void MoveGiraffe(int i, int direction);
	//void MoveProjectile();
	void Update(int inputs[], int disconnect_flags);

	int _framenumber;
	int _num_giraffes;
	RECT _bounds;
	Giraffe giraffes[MAX_PLAYERS];
	Stage stage;
};
#endif // !_GAMESTATE_H_

