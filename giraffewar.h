#ifndef _GiraffeWar_H_
#define _GiraffeWar_H_

#include "ggponet.h"
#include <Windows.h>
//Interface to the GiraffeWar application


enum GiraffeWarInputs {
	INPUT_UP = (1 << 0),
	INPUT_LEFT = (1 << 1),
	INPUT_DOWN = (1 << 2),
	INPUT_RIGHT = (1 << 3),
	INPUT_WEAK = (1 << 4),
	INPUT_HEAVY = (1 << 5),
	INPUT_SHIELD = (1 << 6),
	INPUT_JUMP = (1 << 7),
};

void GiraffeWar_Init(HWND hwnd, unsigned short localport, GGPOPlayer *players, int num_players, int num_spectators, int inputKeys[8]);
void GiraffeWar_InitSpectator(HWND hwnd, unsigned short localport, int num_players, char* host_ip, unsigned short host_port);
void GiraffeWar_DrawCurrentFrame();
void GiraffeWar_AdvanceFrame(int inputs[], int disconnect_flags);
void GiraffeWar_RunFrame(HWND hwnd);
void GiraffeWar_Idle(int time);
void GiraffeWar_DisconnectPlayer(int player);
void GiraffeWar_Exit();

#define ARRAY_SIZE(n) (sizeof(n)/sizeof(n[0]))
#define FRAME_DELAY	2
#endif