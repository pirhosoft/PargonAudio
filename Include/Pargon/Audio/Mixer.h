#pragma once

#include "Pargon/Containers/String.h"

namespace Pargon
{
	class AudioDevice;

	class Mixer
	{
	public:
		auto Audio() const -> AudioDevice&;
		auto Id() const -> int;
		auto Channels() const -> int;

		auto Name() const -> StringView;
		void SetName(StringView name);

		void Play();
		void Pause();
		void Stop();

		auto GetVolume() const -> float;
		void SetVolume(float volume);

		auto GetFrequency() const -> float;
		void SetFrequency(float frequency);

	private:
		friend class AudioDevice;

		Mixer(AudioDevice& audio, int id, int channels);

		AudioDevice& _audio;
		int _id;
		int _channels;

		String _name;
	};
}

inline
auto Pargon::Mixer::Audio() const -> AudioDevice&
{
	return _audio;
}

inline
auto Pargon::Mixer::Id() const -> int
{
	return _id;
}

inline
auto Pargon::Mixer::Channels() const -> int
{
	return _channels;
}

inline
auto Pargon::Mixer::Name() const -> StringView
{
	return _name;
}
