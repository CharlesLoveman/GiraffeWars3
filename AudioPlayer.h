#ifndef _AUDIOPLAYER_H_
#define _AUDIOPLAYER_H_

#include "Audio.h"
#include "gamestate.h"
#include "testbankheader.h"

using namespace DirectX;

class AudioPlayer {
public:
	AudioPlayer();
	void Update(GameState gs);
private:
	std::unique_ptr<AudioEngine> audEngine;
	std::array<std::unique_ptr<WaveBank>, GGPO_MAX_PLAYERS> waveBanks;
	std::array<std::array<std::unique_ptr<SoundEffectInstance>, XACT_WAVEBANK_TESTBANK_ENTRY_COUNT>, GGPO_MAX_PLAYERS> soundInstances;
	std::array<int, GGPO_MAX_PLAYERS> soundStates;
};

#endif // !_AUDIOPLAYER_H_
