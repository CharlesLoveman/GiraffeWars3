#ifndef _AUDIOPLAYER_H_
#define _AUDIOPLAYER_H_

#include "Audio.h"
#include "gamestate.h"
#include "movebankheader.h"
#include "attackbankheader.h"

using namespace DirectX;

class AudioPlayer {
public:
	AudioPlayer();
	void Update(GameState gs);
private:
	std::unique_ptr<AudioEngine> audEngine;
	std::array<std::unique_ptr<WaveBank>, GGPO_MAX_PLAYERS> moveBanks;
	std::array<std::unique_ptr<WaveBank>, GGPO_MAX_PLAYERS> attackBanks;
	std::array<std::array<std::unique_ptr<SoundEffectInstance>, XACT_WAVEBANK_MOVEBANK_ENTRY_COUNT>, GGPO_MAX_PLAYERS> soundMoveInstances;
	std::array<std::array<std::unique_ptr<SoundEffectInstance>, XACT_WAVEBANK_ATTACKBANK_ENTRY_COUNT>, GGPO_MAX_PLAYERS> soundAttackInstances;
	std::array<int, GGPO_MAX_PLAYERS> soundAttackStates;
	std::array<int, GGPO_MAX_PLAYERS> soundMoveStates;
};

#endif // !_AUDIOPLAYER_H_
