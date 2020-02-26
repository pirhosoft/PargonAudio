#include "Pargon/Audio/Sound.h"
#include "Pargon/Audio/Mixer.h"
#include "XAudio2/XAudio2Player.h"
#include "Pargon/Audio.XAudio2.h"

using namespace Pargon;

auto Pargon::CreateXAudio2Player() -> std::unique_ptr<AudioPlayer>
{
	return std::make_unique<XAudio2Player>();
}

XAudio2Player::~XAudio2Player()
{
	_masteringVoice->DestroyVoice();
	CoUninitialize();
}

void XAudio2Player::Setup()
{
	CoInitialize(nullptr);

	auto result = XAudio2Create(&_interface, 0, XAUDIO2_DEFAULT_PROCESSOR);
	if (FAILED(result))
	{
		_interface.Reset();
		return;
	}

	//if (isDebug)
	{
		XAUDIO2_DEBUG_CONFIGURATION configuration = { 0 };
		configuration.TraceMask = XAUDIO2_LOG_ERRORS;
		_interface->SetDebugConfiguration(&configuration);
	}

	result = _interface->CreateMasteringVoice(&_masteringVoice);
	if (FAILED(result))
	{
		_interface.Reset();
		return;
	};
}

void XAudio2Player::Process()
{
	for (auto i = 0; i < _releasedSounds.Count(); i++)
	{
		auto id = _releasedSounds.Item(i);
		auto index = _sounds.GetIndex(id);
		if (index != Sequence::InvalidIndex)
		{
			auto& handle = _sounds.ItemAtIndex(index);
			if (handle.Voice != nullptr)
			{
				auto state = XAUDIO2_VOICE_STATE{};
				handle.Voice->GetState(&state, XAUDIO2_VOICE_NOSAMPLESPLAYED);

				if (state.BuffersQueued != 0)
					continue;

				ReleaseVoice(handle);
				_sounds.RemoveAtIndex(index);
			}
		}
		
		_releasedSounds.RemoveAt(i--);
	}
}

auto XAudio2Player::GetMasterVolume() -> float
{
	assert(_interface);
	auto volume = 1.0f;

	_masteringVoice->GetVolume(&volume);

	return volume;
}

void XAudio2Player::SetMasterVolume(float volume)
{
	assert(_interface);
	_masteringVoice->SetVolume(volume);
}

void XAudio2Player::PlayAll()
{
	assert(_interface);
	_interface->StartEngine();
}

void XAudio2Player::PauseAll()
{
	assert(_interface);
	_interface->StopEngine();
}

void XAudio2Player::StopAll()
{
	PauseAll();
}

namespace
{
	template<typename T>
	auto GetItemById(int id, Map<int, T>& items) -> T*
	{
		auto index = items.GetIndex(id);
		if (index == Sequence::InvalidIndex)
			return nullptr;

		return std::addressof(items.ItemAtIndex(index));
	}
}

auto XAudio2Player::CreateMixer(const Mixer& mixer) -> bool
{
	assert(_interface);

	auto index = _mixers.GetIndex(mixer.Id());
	if (index != Sequence::InvalidIndex)
		return false;

	XAUDIO2_VOICE_DETAILS details;
	_masteringVoice->GetVoiceDetails(&details);

	auto& submix = _mixers.AddOrGet(mixer.Id(), Submix{ mixer.Channels(), nullptr });

	auto result = _interface->CreateSubmixVoice(std::addressof(submix.Voice), submix.Channels, details.InputSampleRate);
	if (FAILED(result))
	{
		_mixers.RemoveWithKey(mixer.Id());
		return false;
	}

	submix.Descriptor = XAUDIO2_SEND_DESCRIPTOR{ 0, submix.Voice };
	submix.SendList = XAUDIO2_VOICE_SENDS{ 1, &submix.Descriptor };

	return true;
}

void XAudio2Player::DestroyMixer(const Mixer& mixer)
{
	auto index = _mixers.GetIndex(mixer.Id());
	if (index == Sequence::InvalidIndex)
		return;

	auto& submix = _mixers.ItemAtIndex(index);
	if (submix.Voice != nullptr)
		submix.Voice->DestroyVoice();

		_mixers.RemoveAtIndex(index);
}

void XAudio2Player::PlayMixer(const Mixer& mixer)
{
	for (auto& handle : _sounds.Items())
	{
		if (handle.Voice != nullptr && mixer.Id() == handle.Mixer)
			handle.Voice->Start();
	}
}

void XAudio2Player::PauseMixer(const Mixer& mixer)
{
	for (auto& handle : _sounds.Items())
	{
		if (handle.Voice != nullptr && mixer.Id() == handle.Mixer)
			handle.Voice->Stop();
	}
}

void XAudio2Player::StopMixer(const Mixer& mixer)
{
	for (auto i = 0; i < _sounds.Count(); i++)
	{
		auto& handle = _sounds.ItemAtIndex(i);

		if (mixer.Id() && handle.Mixer)
		{
			ReleaseVoice(handle);
			_sounds.RemoveAtIndex(i--);
		}
	}
}

auto XAudio2Player::GetMixerVolume(const Mixer& mixer) -> float
{
	auto volume = 1.0f;

	auto submix = GetItemById(mixer.Id(), _mixers);
	if (submix != nullptr && submix->Voice != nullptr)
		submix->Voice->GetVolume(&volume);

	return volume;
}

void XAudio2Player::SetMixerVolume(const Mixer& mixer, float volume)
{
	auto submix = GetItemById(mixer.Id(), _mixers);
	if (submix != nullptr && submix->Voice != nullptr)
		submix->Voice->SetVolume(volume);
}

void XAudio2Player::SetMixerFrequency(const Mixer& mixer, float frequency)
{
	//auto submix = GetItemById(mixer.Id(), Mixers);
	//if (submix != nullptr && submix->Voice != nullptr)
	//	submix->Voice->SetVolume(volume);
}

auto XAudio2Player::GetMixerFrequency(const Mixer& mixer) -> float
{
	auto frequency = 1.0f;

	//auto submix = GetItemById(mixer.Id(), Mixers);
	//if (submix != nullptr && submix->Voice != nullptr)
	//	submix->Voice->GetVolume(&volume);

	return frequency;
}

auto XAudio2Player::CreateSound(const Sound& sound) -> bool
{
	return true;
}

void XAudio2Player::DestroySound(const Sound& sound)
{
}

void XAudio2Player::SetSoundData(const Sound& sound)
{
}

auto XAudio2Player::PlaySound(const Sound& sound, SoundBehavior* behavior) -> SoundInstance
{
	auto& handle = _sounds.AddOrGet(_nextHandleId, { _nextHandleId, sound.Mixer().Id(), static_cast<int>(sound.Channels), static_cast<int>(sound.SampleRate), nullptr });
	auto rate = handle.SampleRate / 1000.0f;

	handle.Data.AudioBytes = sound.Data().Size();
	handle.Data.pAudioData = sound.Data().begin();
	handle.Data.PlayBegin = behavior == nullptr ? 0u : static_cast<unsigned int>(behavior->Start * rate);
	handle.Data.PlayLength = behavior == nullptr ? 0u : static_cast<unsigned int>(behavior->Length * rate);
	handle.Data.LoopBegin = behavior == nullptr ? 0u : static_cast<unsigned int>(behavior->LoopStart * rate);
	handle.Data.LoopLength = behavior == nullptr ? 0u : static_cast<unsigned int>(behavior->LoopLength * rate);
	handle.Data.LoopCount = behavior == nullptr ? 0u : behavior->LoopCount == Sequence::InvalidIndex ? XAUDIO2_LOOP_INFINITE : static_cast<unsigned int>(behavior->LoopCount);
	handle.Data.pContext = reinterpret_cast<void*>(behavior);
	handle.Data.Flags = 0;
	handle.Voice = AcquireVoice(sound);

	if (handle.Voice != nullptr)
	{
		handle.Voice->SubmitSourceBuffer(&handle.Data);
		handle.Voice->Start();
	}

	return SoundInstance(this, _nextHandleId++);
}

void XAudio2Player::PlaySound(const SoundInstance& sound)
{
	auto handle = GetItemById(sound.Id(), _sounds);
	if (handle != nullptr && handle->Voice != nullptr)
		handle->Voice->Start();
}

void XAudio2Player::PauseSound(const SoundInstance& sound)
{
	auto handle = GetItemById(sound.Id(), _sounds);
	if (handle != nullptr && handle->Voice != nullptr)
		handle->Voice->Stop();
}

void XAudio2Player::StopSound(const SoundInstance& sound)
{
	auto index = _sounds.GetIndex(sound.Id());
	if (index == Sequence::InvalidIndex)
		return;

	auto& handle = _sounds.ItemAtIndex(index);
	ReleaseVoice(handle);
	_sounds.RemoveAtIndex(index);
}

void XAudio2Player::ReleaseSound(const SoundInstance& sound)
{
	auto handle = GetItemById(sound.Id(), _sounds);
	if (handle != nullptr && handle->Voice != nullptr)
	{
		handle->Voice->ExitLoop();
		_releasedSounds.Add(handle->Id);
	}
}

void XAudio2Player::ContinueSound(const SoundInstance& sound)
{
	auto handle = GetItemById(sound.Id(), _sounds);
	if (handle != nullptr && handle->Voice != nullptr)
	{
		auto rate = handle->SampleRate / 1000.0f;
		auto behavior = reinterpret_cast<SoundBehavior*>(handle->Data.pContext);
		handle->Data.PlayBegin = behavior == nullptr ? 0u : static_cast<unsigned int>(behavior->Start * rate);
		handle->Data.PlayLength = behavior == nullptr ? 0u : static_cast<unsigned int>(behavior->Length * rate);
		handle->Data.LoopBegin = behavior == nullptr ? 0u : static_cast<unsigned int>(behavior->LoopStart * rate);
		handle->Data.LoopLength = behavior == nullptr ? 0u : static_cast<unsigned int>(behavior->LoopLength * rate);
		handle->Data.LoopCount = behavior == nullptr ? 0u : behavior->LoopCount == Sequence::InvalidIndex ? XAUDIO2_LOOP_INFINITE : static_cast<unsigned int>(behavior->LoopCount);
		handle->Voice->SubmitSourceBuffer(&handle->Data);
		handle->Voice->ExitLoop();
	}
}

auto XAudio2Player::GetSoundVolume(const SoundInstance& sound) -> float
{
	auto volume = 1.0f;

	auto handle = GetItemById(sound.Id(), _sounds);
	if (handle != nullptr && handle->Voice != nullptr)
		handle->Voice->GetVolume(&volume);

	return volume;
}

void XAudio2Player::SetSoundVolume(const SoundInstance& sound, float volume)
{
	auto handle = GetItemById(sound.Id(), _sounds);
	if (handle != nullptr && handle->Voice != nullptr)
		handle->Voice->SetVolume(volume);
}

void XAudio2Player::SetPanning(const SoundInstance& sound, float left, float right)
{
	auto handle = GetItemById(sound.Id(), _sounds);
	if (handle != nullptr && handle->Voice != nullptr)
	{
		auto mixer = GetItemById(handle->Mixer, _mixers);
		if (mixer != nullptr && mixer->Voice != nullptr)
		{
			float panning[2] = { left, right };
			handle->Voice->SetOutputMatrix(mixer->Voice, handle->Channels, mixer->Channels, panning);
		}
	}
}

auto XAudio2Player::GetSoundFrequency(const SoundInstance& sound) -> float
{
	auto frequency = 1.0f;

	auto handle = GetItemById(sound.Id(), _sounds);
	if (handle != nullptr && handle->Voice != nullptr)
		handle->Voice->GetFrequencyRatio(&frequency);

	return frequency;
}

void XAudio2Player::SetSoundFrequency(const SoundInstance& sound, float frequency)
{
	auto handle = GetItemById(sound.Id(), _sounds);
	if (handle != nullptr && handle->Voice != nullptr)
		handle->Voice->SetFrequencyRatio(frequency);
}

auto XAudio2Player::GetSoundLocation(const SoundInstance& sound) -> int
{
	auto milliseconds = 0;
	auto handle = GetItemById(sound.Id(), _sounds);
	if (handle != nullptr && handle->Voice != nullptr)
	{
		auto state = XAUDIO2_VOICE_STATE{};
		handle->Voice->GetState(&state);
		milliseconds = static_cast<int>(state.SamplesPlayed / handle->SampleRate * 1000.0f);
	}

	return milliseconds;
}

void XAudio2Player::SetSoundLocation(const SoundInstance& sound, int location)
{
	auto handle = GetItemById(sound.Id(), _sounds);
	if (handle != nullptr && handle->Voice != nullptr)
	{
		auto samples = location * handle->SampleRate / 1000.0f;
		handle->Voice->Stop();
		handle->Voice->FlushSourceBuffers();
		handle->Data.PlayBegin = static_cast<unsigned int>(samples);
		handle->Voice->SubmitSourceBuffer(&handle->Data);
		handle->Voice->Start();
	}
}

auto XAudio2Player::AcquireVoice(const Sound& sound) -> IXAudio2SourceVoice*
{
	assert(_interface);

	auto submix = GetItemById(sound.Mixer().Id(), _mixers);
	if (submix != nullptr)
	{
		IXAudio2SourceVoice* voice = nullptr;

		WAVEFORMATEX format;
		format.wFormatTag = WAVE_FORMAT_PCM;
		format.nSamplesPerSec = 48000; //sound.SampleRate;
		format.wBitsPerSample = 16; //sound.BitsPerSample;
		format.nChannels = 2; //sound.Channels;
		format.nBlockAlign = (format.wBitsPerSample / 8) * 2;//sound.Channels;
		format.nAvgBytesPerSec = format.nBlockAlign * 48000;//sound.SampleRate;
		format.cbSize = 0;

		_interface->CreateSourceVoice(&voice, &format, 0, XAUDIO2_DEFAULT_FREQ_RATIO, 0, &submix->SendList);

		return voice;
	}

	return nullptr;
}

void XAudio2Player::ReleaseVoice(SoundHandle& handle)
{
	if (handle.Voice != nullptr)
	{
		handle.Voice->Stop();
		handle.Voice->FlushSourceBuffers();
		handle.Voice->DestroyVoice();
		handle.Voice = nullptr;
	}
}

void XAudio2Player::VoiceCallbacks::OnBufferStart(void* pBufferContext)
{
	if (pBufferContext != nullptr)
		reinterpret_cast<SoundBehavior*>(pBufferContext)->OnStart();
}

void XAudio2Player::VoiceCallbacks::OnBufferEnd(void* pBufferContext)
{
	if (pBufferContext != nullptr)
		reinterpret_cast<SoundBehavior*>(pBufferContext)->OnStop();
}

void XAudio2Player::VoiceCallbacks::OnLoopEnd(void* pBufferContext)
{
	if (pBufferContext != nullptr)
		reinterpret_cast<SoundBehavior*>(pBufferContext)->OnLoopEnd();
}
