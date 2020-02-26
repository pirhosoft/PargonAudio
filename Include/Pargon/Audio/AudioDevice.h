#pragma once

#include "Pargon/Audio/AudioPlayer.h"
#include "Pargon/Audio/Mixer.h"
#include "Pargon/Audio/Sound.h"
#include "Pargon/Containers/Map.h"

namespace Pargon
{
	class AudioDevice
	{
	public:
		void Setup(std::unique_ptr<AudioPlayer>&& player);
		void Process();

		auto GetVolume() const -> float;
		void SetVolume(float volume);

		void Play();
		void Pause();
		void Stop();

		auto CreateMixer(int channels) -> Mixer*;
		void DestroyMixer(const Mixer& mixer);
		auto GetMixer(StringView name) const -> Mixer*;

		auto CreateSound(Mixer& mixer) -> Sound*;
		void DestroySound(const Sound& sound);
		auto GetSound(StringView name) const -> Sound*;

	private:
		friend class Mixer;
		friend class Sound;
		friend class SoundInstance;

		int _nextMixerId = 0;
		int _nextSoundId = 0;

		std::unique_ptr<AudioPlayer> _player;

		Map<int, std::unique_ptr<Mixer>> _mixers;
		Map<int, std::unique_ptr<Sound>> _sounds;

		Map<String, int> _mixerNames;
		Map<String, int> _soundNames;

		void NameMixer(const Mixer& mixer, StringView name);
		void NameSound(const Sound& sound, StringView name);
	};
}
