#pragma once

namespace Pargon
{
	class SoundBehavior;

	class AudioPlayer
	{
	public:
		virtual ~AudioPlayer() = default;

	protected:
		friend class AudioDevice;
		friend class Mixer;
		friend class Sound;
		friend class SoundInstance;

		virtual void Setup() = 0;
		virtual void Process() = 0;

		virtual auto GetMasterVolume() -> float = 0;
		virtual void SetMasterVolume(float volume) = 0;

		virtual void PlayAll() = 0;
		virtual void PauseAll() = 0;
		virtual void StopAll() = 0;

		virtual auto CreateMixer(const Mixer& mixer) -> bool = 0;
		virtual void DestroyMixer(const Mixer& mixer) = 0;

		virtual void PlayMixer(const Mixer& mixer) = 0;
		virtual void PauseMixer(const Mixer& mixer) = 0;
		virtual void StopMixer(const Mixer& mixer) = 0;

		virtual auto GetMixerVolume(const Mixer& mixer) -> float = 0;
		virtual void SetMixerVolume(const Mixer& mixer, float volume) = 0;

		virtual auto GetMixerFrequency(const Mixer& mixer) -> float = 0;
		virtual void SetMixerFrequency(const Mixer& mixer, float frequency) = 0;

		virtual auto CreateSound(const Sound& sound) -> bool = 0;
		virtual void DestroySound(const Sound& sound) = 0;

		virtual void SetSoundData(const Sound& sound) = 0;

		virtual auto PlaySound(const Sound& sound, SoundBehavior* behavior) -> SoundInstance = 0;
		virtual void PlaySound(const SoundInstance& sound) = 0;
		virtual void PauseSound(const SoundInstance& sound) = 0;
		virtual void StopSound(const SoundInstance& sound) = 0;
		virtual void ReleaseSound(const SoundInstance& sound) = 0;
		virtual void ContinueSound(const SoundInstance& sound) = 0;

		virtual auto GetSoundVolume(const SoundInstance& sound) -> float = 0;
		virtual void SetSoundVolume(const SoundInstance& sound, float volume) = 0;
		virtual void SetPanning(const SoundInstance& sound, float left, float right) = 0;

		virtual auto GetSoundFrequency(const SoundInstance& sound) -> float = 0;
		virtual void SetSoundFrequency(const SoundInstance& sound, float frequency) = 0;

		virtual auto GetSoundLocation(const SoundInstance& sound) -> int = 0;
		virtual void SetSoundLocation(const SoundInstance& sound, int location) = 0;
	};
}
