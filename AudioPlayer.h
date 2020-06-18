#ifndef _AUDIOPLAYER_H_
#define _AUDIOPLAYER_H_

#include "Audio.h"
#include "gamestate.h"
#include "movebankheader.h"
#include "attackbankheader.h"

using namespace DirectX;

class AudioPlayer {
public:
	AudioPlayer(int NumGiraffes);
	~AudioPlayer();
	void Update(GameState gs);
	void AddGiraffeBank(int ID);
	void Clear();
	void StartMusic();
	void StopMusic();

private:
	std::unique_ptr<AudioEngine> audEngine;
	std::vector<std::unique_ptr<WaveBank>> moveBanks;
	std::vector<std::unique_ptr<WaveBank>> attackBanks;
	std::vector<std::vector<std::unique_ptr<SoundEffectInstance>>> soundMoveInstances;
	std::vector<std::vector<std::unique_ptr<SoundEffectInstance>>> soundAttackInstances;
	std::vector<int> soundAttackStates;
	std::vector<int> soundMoveStates;

	std::unique_ptr<SoundEffect> musicEffect;
	std::unique_ptr<SoundEffectInstance> music;
};

#endif // !_AUDIOPLAYER_H_
