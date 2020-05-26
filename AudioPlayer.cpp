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
		waveBanks[i] = std::make_unique<WaveBank>(audEngine.get(), L"testbank.xwb");
		for (int j = 0; j < XACT_WAVEBANK_TESTBANK_ENTRY_COUNT; ++j) {
			soundInstances[i][j] = waveBanks[i]->CreateInstance(j);
		}
	}
}

void AudioPlayer::Update(GameState gs)
{
	audEngine->Update();
	for (int i = 0; i < gs._num_giraffes; ++i) {
		int changes = gs.giraffes[i]->SoundState ^ soundStates[i];
		for (int j = 0; j < XACT_WAVEBANK_TESTBANK_ENTRY_COUNT; ++j) {
			if (changes & (1 << j)) {
				if (soundStates[i] & (1 << j)) {
					soundInstances[i][j]->Stop();
				}
				else {
					soundInstances[i][j]->Play();
				}
			}
		}
		soundStates[i] = gs.giraffes[i]->SoundState;
	}
}
