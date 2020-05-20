#ifndef _NON_GAMESTATE_H_
#define _NON_GAMESTATE_H_

#include "ggponet.h"


//Holds all the information not related to the game state

enum PlayerConnectState {
	Connecting = 0,
	Synchronizing,
	Running,
	Disconnected,
	Disconnecting
};


struct PlayerConnectionInfo {
	GGPOPlayerType type;
	GGPOPlayerHandle handle;
	PlayerConnectState state;
	int connect_progress;
	int disconnect_timeout;
	int disconnect_start;
};


struct NonGameState {
	struct ChecksumInfo {
		int framenumber;
		int checksum;
	};

	void SetConnectState(GGPOPlayerHandle handle, PlayerConnectState state) {
		for (int i = 0; i < num_players; ++i) {
			if (players[i].handle == handle) {
				players[i].connect_progress = 0;
				players[i].state = state;
				break;
			}
		}
	}

	void SetDisconnectTimeout(GGPOPlayerHandle handle, int when, int timeout) {
		for (int i = 0; i < num_players; ++i) {
			if (players[i].handle == handle) {
				players[i].disconnect_start = when;
				players[i].disconnect_timeout = timeout;
				players[i].state = Disconnecting;
				break;
			}
		}
	}

	void SetConnectState(PlayerConnectState state) {
		for (int i = 0; i < num_players; ++i) {
			players[i].state = state;
		}
	}

	void UpdateConnectProgress(GGPOPlayerHandle handle, int progress) {
		for (int i = 0; i < num_players; i++) {
			if (players[i].handle == handle) {
				players[i].connect_progress = progress;
				break;
			}
		}
	}

	GGPOPlayerHandle local_player_handle;
	PlayerConnectionInfo players[MAX_PLAYERS];
	int num_players;
	ChecksumInfo now;
	ChecksumInfo periodic;
};
#endif // !_NON_GAMESTATE_H_

