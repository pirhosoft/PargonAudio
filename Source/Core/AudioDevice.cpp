#include "Pargon/Audio/AudioDevice.h"

using namespace Pargon;

void AudioDevice::Setup(std::unique_ptr<AudioPlayer>&& player)
{
	_player = std::move(player);
	_player->Setup();
}

void AudioDevice::Process()
{
	assert(_player);
	_player->Process();
}

auto AudioDevice::GetVolume() const -> float
{
	assert(_player);
	return _player->GetMasterVolume();
}

void AudioDevice::SetVolume(float volume)
{
	assert(_player);
	_player->SetMasterVolume(volume);
}

void AudioDevice::Play()
{
	assert(_player);
	_player->PlayAll();
}

void AudioDevice::Pause()
{
	assert(_player);
	_player->PauseAll();
}

void AudioDevice::Stop()
{
	assert(_player);
	_player->StopAll();
}

auto AudioDevice::CreateMixer(int channels) -> Mixer*
{
	assert(_player);

	auto& mixer = _mixers.AddOrSet(_nextMixerId, std::unique_ptr<Mixer>(new Mixer(*this, _nextMixerId, channels)));

	if (!_player->CreateMixer(*mixer))
	{
		_mixers.RemoveWithKey(_nextMixerId);
		return nullptr;
	}

	_nextMixerId++;
	return mixer.get();
}

void AudioDevice::DestroyMixer(const Mixer& mixer)
{
	assert(_player);

	_player->DestroyMixer(mixer);

	auto index = _mixerNames.GetIndex(mixer.Name());
	if (index != Sequence::InvalidIndex)
		_mixerNames.RemoveAtIndex(index);

	_mixers.RemoveWithKey(mixer._id);
}

void AudioDevice::NameMixer(const Mixer& mixer, StringView name)
{
	_mixerNames.AddOrSet(name, mixer._id);
}

auto AudioDevice::GetMixer(StringView name) const -> Mixer*
{
	auto id = _mixerNames.GetIndex(name);
	if (id != Sequence::InvalidIndex)
	{
		auto index = _sounds.GetIndex(id);
		return index == Sequence::InvalidIndex ? nullptr : _mixers.ItemAtIndex(index).get();
	}

	return nullptr;
}

auto AudioDevice::CreateSound(Mixer& mixer) -> Sound*
{
	assert(_player);

	auto& sound = _sounds.AddOrSet(_nextSoundId, std::unique_ptr<Sound>(new Sound(*this, mixer, _nextSoundId)));
	
	if (!_player->CreateSound(*sound))
	{
		_sounds.RemoveWithKey(_nextSoundId);
		return nullptr;
	}

	_nextSoundId++;
	return sound.get();
}

void AudioDevice::DestroySound(const Sound& sound)
{
	assert(_player);

	_player->DestroySound(sound);

	auto index = _soundNames.GetIndex(sound.Name());
	if (index != Sequence::InvalidIndex)
		_soundNames.RemoveAtIndex(index);

	_sounds.RemoveWithKey(sound._id);
}

void AudioDevice::NameSound(const Sound& sound, StringView name)
{
	_soundNames.AddOrSet(name, sound._id);
}

auto AudioDevice::GetSound(StringView name) const -> Sound*
{
	auto id = _soundNames.GetIndex(name);
	if (id != Sequence::InvalidIndex)
	{
		auto index = _sounds.GetIndex(id);
		return index == Sequence::InvalidIndex ? nullptr : _sounds.ItemAtIndex(index).get();
	}

	return nullptr;
}
