#pragma once

#include "Pargon/Audio/AudioPlayer.h"
#include "Pargon/Containers/List.h"
#include "Pargon/Containers/Map.h"

#include <Xaudio2.h>
#include <x3daudio.h>
#include <wrl.h>

#undef max
#undef PlaySound

namespace Pargon
{
	class XAudio2Player : public AudioPlayer
	{
	public:
		~XAudio2Player();

	protected:
		void Setup() override;
		void Process() override;

		auto GetMasterVolume() -> float override;
		void SetMasterVolume(float volume) override;

		void PlayAll() override;
		void PauseAll() override;
		void StopAll() override;

		auto CreateMixer(const Mixer& mixer) -> bool override;
		void DestroyMixer(const Mixer& mixer) override;

		void PlayMixer(const Mixer& mixer) override;
		void PauseMixer(const Mixer& mixer) override;
		void StopMixer(const Mixer& mixer) override;

		auto GetMixerVolume(const Mixer& mixer) -> float override;
		void SetMixerVolume(const Mixer& mixer, float volume) override;

		auto GetMixerFrequency(const Mixer& mixer) -> float override;
		void SetMixerFrequency(const Mixer& mixer, float frequency) override;

		auto CreateSound(const Sound& sound) -> bool override;
		void DestroySound(const Sound& sound) override;

		void SetSoundData(const Sound& sound) override;

		auto PlaySound(const Sound& sound, SoundBehavior* behavior) -> SoundInstance override;
		void PlaySound(const SoundInstance& sound) override;
		void PauseSound(const SoundInstance& sound) override;
		void StopSound(const SoundInstance& sound) override;
		void ReleaseSound(const SoundInstance& sound) override;
		void ContinueSound(const SoundInstance& sound) override;

		auto GetSoundVolume(const SoundInstance& sound) -> float override;
		void SetSoundVolume(const SoundInstance& sound, float volume) override;
		void SetPanning(const SoundInstance& sound, float left, float right) override;

		auto GetSoundFrequency(const SoundInstance& sound) -> float override;
		void SetSoundFrequency(const SoundInstance& sound, float frequency) override;

		auto GetSoundLocation(const SoundInstance& sound) -> int override;
		void SetSoundLocation(const SoundInstance& sound, int location) override;

	private:
		struct SoundHandle
		{
			int Id;
			int Mixer;
			int Channels;
			int SampleRate;
			IXAudio2SourceVoice* Voice;
			XAUDIO2_BUFFER Data;
		};

		struct Submix
		{
			int Channels;
			IXAudio2SubmixVoice* Voice;
			XAUDIO2_SEND_DESCRIPTOR Descriptor;
			XAUDIO2_VOICE_SENDS SendList;
		};

		class VoiceCallbacks : public IXAudio2VoiceCallback
		{
			void OnVoiceProcessingPassStart(unsigned int BytesRequired) override {};
			void OnVoiceProcessingPassEnd() override {};
			void OnStreamEnd() override {};
			void OnBufferStart(void* pBufferContext) override;
			void OnBufferEnd(void* pBufferContext) override;
			void OnLoopEnd(void* pBufferContext) override;
			void OnVoiceError(void* pBufferContext, HRESULT Error) override {};
		};

		Microsoft::WRL::ComPtr<IXAudio2> _interface;
		IXAudio2MasteringVoice* _masteringVoice;

		Map<int, Submix> _mixers;
		Map<int, SoundHandle> _sounds;

		List<int> _releasedSounds;

		int _nextHandleId = 0;

		VoiceCallbacks _voiceCallbacks;

		auto AcquireVoice(const Sound& sound) -> IXAudio2SourceVoice*;
		void ReleaseVoice(SoundHandle& handle);
	};
}
