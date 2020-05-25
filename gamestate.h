#ifndef _GAMESTATE_H_
#define _GAMESTATE_H_
#include "Giraffe.h"
#include "NormGiraffe.h"
#include "Stage.h"
#include "MoveSet.h"
#include <memory>
#include <array>

//Structure to be serialized at each frame
//Needs to be small

constexpr int MAX_PLAYERS = 4;


struct GameState
{
	void Init(HWND hwnd, int num_players, const std::array<MoveSet*, 4> MoveSets);
	//void GetGiraffeAI(int i);
	//void ParseGiraffeInputs(int inputs, int i, int *direction);
	//void MoveGiraffe(int i, int direction);
	//void MoveProjectile();
	void Update(int inputs[], int disconnect_flags);

	int _framenumber;
	int _num_giraffes;
	RECT _bounds;
	std::array<Giraffe*, 4> giraffes;
	std::vector<NormGiraffe> normGiraffes;
	Stage stage;
	//HINSTANCE hInst;
};
#endif // !_GAMESTATE_H_

