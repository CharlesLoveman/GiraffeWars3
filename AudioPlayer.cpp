#include "AudioPlayer.h"
#include "resource.h"

AudioPlayer::AudioPlayer()
{
	AUDIO_ENGINE_FLAGS eflags = AudioEngine_Default;
#if defined(_DEBUG)
	eflags = eflags | AudioEngine_Debug;
#endif
	audEngine = std::make_unique<AudioEngine>(eflags);
	for (int i = 0; i < 4; ++i) {
		moveBanks[i] = std::make_unique<WaveBank>(audEngine.get(), L"movebank.xwb");
		attackBanks[i] = std::make_unique<WaveBank>(audEngine.get(), L"attackbank.xwb");
		for (int j = 0; j < XACT_WAVEBANK_MOVEBANK_ENTRY_COUNT; ++j) {
			soundMoveInstances[i][j] = moveBanks[i]->CreateInstance(j);
		}
		for (int j = 0; j < XACT_WAVEBANK_ATTACKBANK_ENTRY_COUNT; ++j) {
			soundAttackInstances[i][j] = attackBanks[i]->CreateInstance(j);
		}
		//soundMoveInstances[i][13]->l
	}
}

void AudioPlayer::Update(GameState gs)
{
	audEngine->Update();
	for (int i = 0; i < gs._num_giraffes; ++i) {
		int changes = gs.giraffes[i]->SoundMoveState ^ soundMoveStates[i];
		for (int j = 0; j < XACT_WAVEBANK_MOVEBANK_ENTRY_COUNT; ++j) {
			if (changes & (1 << j)) {
				if (soundMoveStates[i] & (1 << j)) {
					soundMoveInstances[i][j]->Stop(true);
				}
				else {
					soundMoveInstances[i][j]->Play(true);
				}
			}
		}
		soundMoveStates[i] = gs.giraffes[i]->SoundMoveState;

		changes = gs.giraffes[i]->SoundAttackState ^ soundAttackStates[i];
		for (int j = 0; j < XACT_WAVEBANK_ATTACKBANK_ENTRY_COUNT; ++j) {
			if (changes & (1 << j)) {
				if (soundAttackStates[i] & (1 << j)) {
					soundAttackInstances[i][j]->Stop(true);
				}
				else {
					soundAttackInstances[i][j]->Play(true);
				}
			}
		}
		soundAttackStates[i] = gs.giraffes[i]->SoundAttackState;
	}
}
