#include "AudioPlayer.h"
#include "resource.h"

AudioPlayer::AudioPlayer(int NumGiraffes)
{
	AUDIO_ENGINE_FLAGS eflags = AudioEngine_Default;
#if defined(_DEBUG)
	eflags = eflags | AudioEngine_Debug;
#endif
	audEngine = std::make_unique<AudioEngine>(eflags);
	for (int i = 0; i < NumGiraffes; ++i) {
		moveBanks.push_back(std::make_unique<WaveBank>(audEngine.get(), L"movebank.xwb"));
		soundMoveInstances.push_back({});
		for (int j = 0; j < XACT_WAVEBANK_MOVEBANK_ENTRY_COUNT; ++j) {
			soundMoveInstances.back().push_back(moveBanks.back()->CreateInstance(j));
		}
		soundAttackStates.push_back(0);
		soundMoveStates.push_back(0);
	}
}

AudioPlayer::~AudioPlayer()
{
	soundAttackInstances.clear();
	soundMoveInstances.clear();
	attackBanks.clear();
	moveBanks.clear();
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

void AudioPlayer::AddGiraffeBank(int ID)
{
	switch(ID) {
	case 1:
		attackBanks.push_back(std::make_unique<WaveBank>(audEngine.get(), L"poshattackbank.xwb"));
		break;
	case 2:
		attackBanks.push_back(std::make_unique<WaveBank>(audEngine.get(), L"coolattackbank.xwb"));
		break;
	case 3:
		attackBanks.push_back(std::make_unique<WaveBank>(audEngine.get(), L"robotattackbank.xwb"));
		break;
	default:
		attackBanks.push_back(std::make_unique<WaveBank>(audEngine.get(), L"normattackbank.xwb"));
		break;
	}
	soundAttackInstances.push_back({});
	for (int i = 0; i < XACT_WAVEBANK_ATTACKBANK_ENTRY_COUNT; ++i) {
		soundAttackInstances.back().push_back(attackBanks.back()->CreateInstance(i));
	}
	if (ID == 3) {
		for (int i = XACT_WAVEBANK_ATTACKBANK_ENTRY_COUNT; i < XACT_WAVEBANK_ATTACKBANK_ENTRY_COUNT + 2; ++i) {
			soundAttackInstances.back().push_back(attackBanks.back()->CreateInstance(i));
		}
	}
}

void AudioPlayer::Clear()
{
	soundAttackInstances.clear();
	attackBanks.clear();
}
