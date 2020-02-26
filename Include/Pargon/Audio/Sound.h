#pragma once

#include "Pargon/Containers/Buffer.h"
#include "Pargon/Containers/String.h"

namespace Pargon
{
	class AudioDevice;
	class AudioPlayer;
	class Mixer;

	class SoundBehavior
	{
	public:
		int Start = 0;
		int Length = 0;
		int LoopCount = 0;
		int LoopStart = 0;
		int LoopLength = 0;

		virtual void OnStart() {};
		virtual void OnStop() {};
		virtual void OnLoopEnd() {};
	};

	class SoundInstance
	{
	public:
		SoundInstance();
		SoundInstance(AudioPlayer* audio, int id);
		~SoundInstance();

		SoundInstance(SoundInstance&& move);
		auto operator=(SoundInstance&& move) -> SoundInstance&;

		auto Id() const -> int;
		void Reset();

		void Play();
		void Pause();
		void Stop();
		void Release();
		void Continue();

		auto GetVolume() const -> float;
		void SetVolume(float volume);
		void SetPanning(float left, float right);

		auto GetFrequency() const -> float;
		void SetFrequency(float frequency);

		auto GetLocation() const -> int;
		void SetLocation(int location);

	private:
		AudioPlayer* _audio;
		int _id;
	};

	class Sound
	{
	public:
		uint16_t Channels;
		uint32_t SampleRate;
		uint16_t BitsPerSample;

		auto Audio() const -> AudioDevice&;
		auto Mixer() const -> const Mixer&;
		auto Id() const -> int;

		auto Name() const -> StringView;
		void SetName(StringView name);

		auto Filename() const -> StringView;
		void SetFilename(StringView filename);

		auto Data() const -> BufferView;
		void SetData(Buffer&& data);

		auto Play(SoundBehavior* behavior) const -> SoundInstance;

	private:
		friend class AudioDevice;

		Sound(AudioDevice& audio, Pargon::Mixer& mixer, int id);

		AudioDevice& _audio;

		Pargon::Mixer& _mixer;
		int _id;

		String _name;
		String _filename;
		Buffer _data;
	};
}

inline
auto Pargon::Sound::Audio() const -> AudioDevice&
{
	return _audio;
}

inline
auto Pargon::Sound::Mixer() const -> const Pargon::Mixer&
{
	return _mixer;
}

inline
auto Pargon::Sound::Id() const -> int
{
	return _id;
}

inline
auto Pargon::Sound::Name() const -> StringView
{
	return _name;
}

inline
auto Pargon::Sound::Filename() const -> StringView
{
	return _filename;
}

inline
auto Pargon::Sound::Data() const -> BufferView
{
	return _data;
}

inline
auto Pargon::SoundInstance::Id() const -> int
{
	return _id;
}
