#include "Pargon/Audio/AudioDevice.h"
#include "Pargon/Audio/Mixer.h"

using namespace Pargon;

Mixer::Mixer(AudioDevice& audio, int id, int channels) :
	_audio(audio),
	_id(id),
	_channels(channels)
{
}

void Mixer::SetName(StringView name)
{
	_name = name;
	_audio.NameMixer(*this, name);
}

void Mixer::Play()
{
	assert(_audio._player);
	_audio._player->PlayMixer(*this);
}

void Mixer::Pause()
{
	assert(_audio._player);
	_audio._player->PauseMixer(*this);
}

void Mixer::Stop()
{
	assert(_audio._player);
	_audio._player->StopMixer(*this);
}

auto Mixer::GetVolume() const -> float
{
	assert(_audio._player);
	return _audio._player->GetMixerVolume(*this);
}

void Mixer::SetVolume(float volume)
{
	assert(_audio._player);
	_audio._player->SetMixerVolume(*this, volume);
}

auto Mixer::GetFrequency() const -> float
{
	assert(_audio._player);
	return _audio._player->GetMixerFrequency(*this);
}

void Mixer::SetFrequency(float frequency)
{
	assert(_audio._player);
	_audio._player->SetMixerFrequency(*this, frequency);
}
