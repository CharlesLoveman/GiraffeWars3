#include <windows.h>
#include <gl/GL.h>
#include <gl/glu.h>
#include <math.h>
#include <stdio.h>
#include "gdi_renderer.h"
#include "giraffewar.h"
#include <memory>
#include <array>
#include "MoveSet.h"
#include "NormMoveSet.h"
#include "AudioPlayer.h"

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Winmm.lib")

GameState gs = { 0 };
NonGameState ngs = { 0 };
std::array<MoveSet*, 4> MoveSets;
struct {
	int	key;
	int	input;
} inputtable[8];
Renderer* renderer = NULL;
AudioPlayer* audioPlayer;
GGPOSession* ggpo = NULL;

int
fletcher32_checksum(short* data, size_t len)
{
	int sum1 = 0xffff, sum2 = 0xffff;

	while (len) {
		size_t tlen = len > 360 ? 360 : len;
		len -= tlen;
		do {
			sum1 += *data++;
			sum2 += sum1;
		} while (--tlen);
		sum1 = (sum1 & 0xffff) + (sum1 >> 16);
		sum2 = (sum2 & 0xffff) + (sum2 >> 16);
	}

	/* Second reduction step to reduce sums to 16 bits */
	sum1 = (sum1 & 0xffff) + (sum1 >> 16);
	sum2 = (sum2 & 0xffff) + (sum2 >> 16);
	return sum2 << 16 | sum1;
}


//begin game callback, deprecated

bool __cdecl gw_begin_game_callback(const char*)
{
	return true;
}

//on ggpo event callback, updates status
bool __cdecl gw_on_event_callback(GGPOEvent *info)
{
	int progress;
	switch (info->code) {
	case GGPO_EVENTCODE_CONNECTED_TO_PEER:
		ngs.SetConnectState(info->u.connected.player, Synchronizing);
		break;
	case GGPO_EVENTCODE_SYNCHRONIZING_WITH_PEER:
		progress = 100 * info->u.synchronizing.player / info->u.synchronizing.total;
		ngs.UpdateConnectProgress(info->u.synchronizing.player, progress);
		break;
	case GGPO_EVENTCODE_SYNCHRONIZED_WITH_PEER:
		ngs.UpdateConnectProgress(info->u.synchronized.player, 100);
		break;
	case GGPO_EVENTCODE_RUNNING:
		ngs.SetConnectState(Running);
		renderer->SetStatusText("");
		break;
	case GGPO_EVENTCODE_CONNECTION_INTERRUPTED:
		ngs.SetDisconnectTimeout(info->u.connection_interrupted.player, timeGetTime(), info->u.connection_interrupted.disconnect_timeout);
		break;
	case GGPO_EVENTCODE_CONNECTION_RESUMED:
		ngs.SetConnectState(info->u.connection_resumed.player, Running);
		break;
	case GGPO_EVENTCODE_DISCONNECTED_FROM_PEER:
		ngs.SetConnectState(info->u.disconnected.player, Disconnected);
		break;
	case GGPO_EVENTCODE_TIMESYNC:
		Sleep(1000 * info->u.timesync.frames_ahead / 60);
		break;
	}
	return true;
}


//Move foreward one frame during a rollback
bool __cdecl gw_advance_frame_callback(int)
{
	int inputs[MAX_PLAYERS] = { 0 };
	int disconnect_flags;

	//uses the inputs from GGPO, not from the keyboard
	ggpo_synchronize_input(ggpo, (void*)inputs, sizeof(int) * MAX_PLAYERS, &disconnect_flags);
	GiraffeWar_AdvanceFrame(inputs, disconnect_flags);
	return true;
}


//Match the current state to the one provided by GGPO
bool __cdecl gw_load_game_state_callback(unsigned char* buffer, int len)
{
	
	int normSize = sizeof(gs.normGiraffes[0]) * gs.normGiraffes.size();
	int robotSize = sizeof(gs.robotGiraffes[0]) * gs.robotGiraffes.size();
	int ledgeSize = sizeof(gs.stage.Ledges[0]) * gs.stage.Ledges.size();
	int gsSize = len - normSize - robotSize - ledgeSize;
	memcpy(&gs, buffer, gsSize);
	memcpy(gs.normGiraffes.data(), buffer + gsSize, normSize);
	memcpy(gs.robotGiraffes.data(), buffer + gsSize + normSize, robotSize);
	memcpy(gs.stage.Ledges.data(), buffer + gsSize + normSize + robotSize, ledgeSize);

	return true;
}


//Saves the current state to the buffer
bool __cdecl gw_save_game_state_callback(unsigned char** buffer, int* len, int* checksum, int)
{
	int normSize = sizeof(gs.normGiraffes[0]) * gs.normGiraffes.size();
	int robotSize = sizeof(gs.robotGiraffes[0]) * gs.robotGiraffes.size();
	int ledgeSize = sizeof(gs.stage.Ledges[0]) * gs.stage.Ledges.size();
	int gsSize = sizeof(gs);
	*len = gsSize + normSize + robotSize + ledgeSize;
	*buffer = (unsigned char*)malloc(*len);
	if (!*buffer) {
		return false;
	}

	*checksum = fletcher32_checksum((short*)gs.normGiraffes.data(), sizeof(gs.normGiraffes[0]) * gs.normGiraffes.size() / 2);
	memcpy(*buffer, &gs, *len);
	memcpy(*buffer + gsSize, gs.normGiraffes.data(), normSize);
	memcpy(*buffer + gsSize + normSize, gs.robotGiraffes.data(), robotSize);
	memcpy(*buffer + gsSize + normSize + robotSize, gs.stage.Ledges.data(), ledgeSize);

	return true;
}


//Logs the gamestate
//------------Fix logs once gamestate is written-------------------
bool __cdecl gw_log_game_state(char* filename, unsigned char* buffer, int)
{
	FILE* fp = nullptr;
	fopen_s(&fp, filename, "w");
	if (fp) {
		GameState* gamestate = (GameState*)buffer;
		fprintf(fp, "GameState object.\n");

		/*fprintf(fp, " bounds: %d, %d x %d, %d.\n", gamestate->_bounds.left, gamestate->_bounds.top, gamestate->_bounds.right, gamestate->_bounds.bottom);
		fprintf(fp, " num_paddles: %d.\n", gamestate->_num_paddles);
		for (int i = 0; i < gamestate->_num_paddles; ++i) {
			Paddle* paddle = gamestate->_paddles + i;
			fprintf(fp, " paddle %d position: %.4f, %.4f\n", i, paddle->position.x, paddle->position.y);
			fprintf(fp, " paddle %d velocity: %.4f, %.4f\n", i, paddle->velocity.dx, paddle->velocity.dy);
			fprintf(fp, " paddle %d height: %d", i, paddle->height);
			fprintf(fp, " paddle %d width: %d", i, paddle->width);
			fprintf(fp, " paddle %d speed: %d", i, paddle->speed);
			fprintf(fp, " paddle %d score: %d", i, paddle->score);
		}
		Ball ball = gamestate->ball;
		fprintf(fp, " Ball position: %.4f, %.4f\n", ball.position.x, ball.position.y);
		fprintf(fp, " Ball velocity: %.4f, %.4f\n", ball.velocity.dx, ball.velocity.dy);
		fprintf(fp, " Ball radius: %d", ball.radius);*/
		fclose(fp);
	}
	return true;
}


//Frees unnecessary states saved to the buffer
void __cdecl gw_free_buffer(void* buffer)
{
	free(buffer);
}


//Initialize the GiraffeWar game state, renderer, and network session
void GiraffeWar_Init(HWND hwnd, unsigned short localport, GGPOPlayer* players, int num_players, int num_spectators, int inputKeys[8])
{
	GGPOErrorCode result;
	renderer = new GDIRenderer(hwnd);
	audioPlayer = new AudioPlayer();
	

	MoveSets[0] = new NormMoveSet();
	MoveSets[1] = new RobotMoveSet();
	for (int i = 2; i < GGPO_MAX_PLAYERS; ++i) {
		MoveSets[i] = new NormMoveSet();
	}

	//Initialize the game state
	gs.Init(hwnd, num_players, MoveSets);
	ngs.num_players = num_players;

	//Fill in the callback structure used by GGPO
	GGPOSessionCallbacks cb = { 0 };
	cb.begin_game = gw_begin_game_callback;
	cb.advance_frame = gw_advance_frame_callback;
	cb.load_game_state = gw_load_game_state_callback;
	cb.save_game_state = gw_save_game_state_callback;
	cb.free_buffer = gw_free_buffer;
	cb.on_event = gw_on_event_callback;
	cb.log_game_state = gw_log_game_state;

#if defined(SYNC_TEST)
	result = ggpo_start_synctest(&ggpo, &cb, "GiraffeWar", num_players, sizeof(int), 1);
#else
	result = ggpo_start_session(&ggpo, &cb, "GiraffeWar", num_players, sizeof(int), localport);
#endif

	//disconnect clients after 3000ms, starts count dime timer after 1000ms
	ggpo_set_disconnect_timeout(ggpo, 3000);
	ggpo_set_disconnect_notify_start(ggpo, 1000);

	for (int i = 0; i < num_players + num_spectators; ++i) {
		GGPOPlayerHandle handle;
		result = ggpo_add_player(ggpo, players + i, &handle);
		ngs.players[i].handle = handle;
		ngs.players[i].type = players[i].type;
		if (players[i].type == GGPO_PLAYERTYPE_LOCAL) {
			ngs.players[i].connect_progress = 100;
			ngs.local_player_handle = handle;
			ngs.SetConnectState(handle, Connecting);
			ggpo_set_frame_delay(ggpo, handle, FRAME_DELAY);
		}
		else {
			ngs.players[i].connect_progress = 0;
		}
	}


	inputtable[0] = { inputKeys[0], INPUT_UP };
	inputtable[1] = { inputKeys[1], INPUT_LEFT };
	inputtable[2] = { inputKeys[2], INPUT_DOWN };
	inputtable[3] = { inputKeys[3], INPUT_RIGHT };
	inputtable[4] = { inputKeys[4], INPUT_JUMP };
	inputtable[5] = { inputKeys[5], INPUT_WEAK };
	inputtable[6] = { inputKeys[6], INPUT_HEAVY };
	inputtable[7] = { inputKeys[7], INPUT_SHIELD };

	//ggpoutil_perfmon_init(hwnd);
	renderer->SetStatusText("Connecting to peers.");
}


//Create a new spectator session
void GiraffeWar_InitSpectator(HWND hwnd, unsigned short localport, int num_players, char* host_ip, unsigned short host_port)
{
	GGPOErrorCode result;
	renderer = new GDIRenderer(hwnd);
	audioPlayer = new AudioPlayer();

	//std::shared_ptr<NormMoveSet> pNorm = std::make_shared<NormMoveSet>();
	for (int i = 0; i < 4; ++i) {
		MoveSets[i] = new NormMoveSet();
	}

	//Initialize the game state
	gs.Init(hwnd, num_players, MoveSets);

	//Create the callback structure
	GGPOSessionCallbacks cb = { 0 };
	cb.begin_game = gw_begin_game_callback;
	cb.advance_frame = gw_advance_frame_callback;
	cb.load_game_state = gw_load_game_state_callback;
	cb.save_game_state = gw_save_game_state_callback;
	cb.free_buffer = gw_free_buffer;
	cb.on_event = gw_on_event_callback;
	cb.log_game_state = gw_log_game_state;

	result = ggpo_start_spectating(&ggpo, &cb, "GiraffeWar", num_players, sizeof(int), localport, host_ip, host_port);

	//ggpoutil_perfmon_init(hwnd);

	renderer->SetStatusText("Starting new spectator session");
}


//Disconnect a player from the session
void GiraffeWar_DisconnectPlayer(int player)
{
	if (player < ngs.num_players) {
		char logbuf[128];
		GGPOErrorCode result = ggpo_disconnect_player(ggpo, ngs.players[player].handle);
		if (GGPO_SUCCEEDED(result)) {
			sprintf_s(logbuf, ARRAYSIZE(logbuf), "Disconnected player %d.\n", player);
		}
		else {
			sprintf_s(logbuf, ARRAYSIZE(logbuf), "Error while disconnnecting player (err:%d).\n", result);
		}
		renderer->SetStatusText(logbuf);
	}
}


//Draw the current frame, does not modify game state!
void GiraffeWar_DrawCurrentFrame()
{
	if (renderer != nullptr) {
		renderer->Draw(gs, ngs);
	}
	if (audioPlayer != nullptr) {
		audioPlayer->Update(gs);
	}
}


//Advances the game state by 1 frame, using the inputs given by ggpo
void GiraffeWar_AdvanceFrame(int inputs[], int disconnect_flags)
{
	gs.Update(inputs, disconnect_flags);

	//update the checksums
	ngs.now.framenumber = gs._framenumber;
	ngs.now.checksum = fletcher32_checksum((short*)& gs.normGiraffes, sizeof(gs.normGiraffes) / 2);
	if ((gs._framenumber % 90) == 0) {
		ngs.periodic = ngs.now;
	}

	//Tell ggpo that we've moved forward 1 frame
	ggpo_advance_frame(ggpo);

	////Update the performance display
	//GGPOPlayerHandle handles[2];
	//int count = 0;
	//for (int i = 0; i < 2; ++i) {
	//	if (ngs.players[i].type == GGPO_PLAYERTYPE_REMOTE) {
	//		handles[count] = ngs.players[i].handle;
	//		++count;
	//	}
	//}
	////ggpoutil_perfmon_update(ggpo, handles);
}


//Read the keyboard inputs
int ReadInputs(HWND hwnd)
{
	int i, inputs = 0;
	if (GetForegroundWindow() == hwnd) {
		for (i = 0; i < sizeof(inputtable) / sizeof(inputtable[0]); ++i) {
			if (GetAsyncKeyState(inputtable[i].key)) {
				inputs |= inputtable[i].input;
			}
		}
	}

	return inputs;
}


//Runs 1 frame
void GiraffeWar_RunFrame(HWND hwnd)
{
	GGPOErrorCode result = GGPO_OK;
	int disconnect_flags;
	int inputs[MAX_PLAYERS] = { 0 };

	if (ngs.local_player_handle != GGPO_INVALID_HANDLE) {
		int input = ReadInputs(hwnd);
#if defined(SYNC_TEST)
		input = rand();
#endif
		result = ggpo_add_local_input(ggpo, ngs.local_player_handle, &input, sizeof(input));
	}

	//sync inputs with ggpo.
	//if we have enough input to move forward, then ggpo will provide the new inputs

	if (GGPO_SUCCEEDED(result)) {
		result = ggpo_synchronize_input(ggpo, (void*)inputs, sizeof(int) * MAX_PLAYERS, &disconnect_flags);
		if (GGPO_SUCCEEDED(result)) {
			//Advance a frame with the new inputs
			GiraffeWar_AdvanceFrame(inputs, disconnect_flags);
		}
	}
	GiraffeWar_DrawCurrentFrame();
}


//Idle time is spent within ggpo
void GiraffeWar_Idle(int time)
{
	ggpo_idle(ggpo, time);
}


void GiraffeWar_Exit()
{
	for (int i = 0; i < 4; ++i) {
		delete MoveSets[i];
	}

	if (ggpo) {
		ggpo_close_session(ggpo);
		ggpo = NULL;
	}
	delete renderer;
	delete audioPlayer;
}