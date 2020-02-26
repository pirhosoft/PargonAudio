#include "Pargon/Audio/AudioDevice.h"
#include "Pargon/Audio/Sound.h"
#include "Pargon/Application/Log.h"
#include "Pargon/Files/Directory.h"
#include "Pargon/Files/File.h"
#include "Pargon/Serialization/Serializer.h"

using namespace Pargon;

namespace
{
	auto IdIs(StringView id, StringView v)
	{
		return Equals(id, v);
	}

	auto ReadFormat(BufferReader& reader, Sound& sound) -> bool
	{
		uint32_t chunkSize;
		uint16_t formatType;
		uint16_t channels;
		uint32_t sampleRate;
		uint32_t averageBytesPerSecond;
		uint16_t bytesPerSample;
		uint16_t bitsPerSample;

		reader.Read(chunkSize);
		reader.Read(formatType);
		reader.Read(channels);
		reader.Read(sampleRate);
		reader.Read(averageBytesPerSecond);
		reader.Read(bytesPerSample);
		reader.Read(bitsPerSample);

		reader.Advance(chunkSize - 16);

		if (formatType != 1) { return false; }

		sound.Channels = channels;
		sound.BitsPerSample = bitsPerSample;
		sound.SampleRate = sampleRate;

		return true;
	}

	void ReadData(BufferReader& reader, Sound& sound)
	{
		uint32_t totalBytes;
		reader.Read(totalBytes);

		BufferWriter data;
		auto bytes = reader.ReadBytes(totalBytes);
		data.WriteBytes(bytes, false);

		sound.SetData(data.ExtractBuffer());

		if (totalBytes % 2 == 1)
			reader.Advance(1);
	}

	void SkipChunk(BufferReader& reader)
	{
		uint32_t chunkSize;
		reader.Read(chunkSize);
		reader.Advance(chunkSize);
	}

	auto ReadChunk(BufferReader& reader, Sound& sound) -> bool
	{
		auto id = StringView(reader.ReadBytes(4));

		if (Equals(id, "fmt ", true))
			return ReadFormat(reader, sound);
		else if (Equals(id, "data", true))
			ReadData(reader, sound);
		else
			SkipChunk(reader);

		return true;
	}

	void LoadWav(StringView filename, Sound& sound)
	{
		auto file = ApplicationDirectory().GetFile(filename);
		auto data = file.ReadData();

		if (data.Exists)
		{
			auto reader = BufferReader(data.Data);
			reader.SetEndian(Endian::Little);

			if (reader.Remaining() < 12)
			{
				reader.ReportError("content is not a wav");
				return;
			}

			auto id = StringView(reader.ReadBytes(4));
			if (!Equals(id, "RIFF", true))
			{
				reader.ReportError("content is not a wav");
				return;
			}

			uint32_t skip = 0;
			reader.Read(skip);

			id = reader.ReadBytes(4);
			if (!Equals(id, "WAVE", true))
			{
				reader.ReportError("content is not a wav");
				return;
			}

			while (!reader.AtEnd() && sound.Data().IsEmpty())
			{
				if (!ReadChunk(reader, sound))
					return;
			}

			if (sound.Data().IsEmpty())
				reader.ReportError("missing wav data");
		}
	}
}

Sound::Sound(AudioDevice& audio, Pargon::Mixer& mixer, int id) :
	_audio(audio),
	_mixer(mixer),
	_id(id)
{
}

void Sound::SetName(StringView name)
{
	_name = name;
	_audio.NameSound(*this, name);
}

void Sound::SetFilename(StringView filename)
{
	_filename = filename;

	if (filename.IsEmpty())
		_data.Clear();
	else
		LoadWav(filename, *this);
}

void Sound::SetData(Buffer&& data)
{
	_data = std::move(data);
	_audio._player->SetSoundData(*this);
}

auto Sound::Play(SoundBehavior* behavior) const -> SoundInstance
{
	assert(_audio._player);
	return _audio._player->PlaySound(*this, behavior);
}

SoundInstance::SoundInstance() :
	_audio(nullptr),
	_id(Sequence::InvalidIndex)
{
}

SoundInstance::SoundInstance(AudioPlayer* audio, int id) :
	_audio(audio),
	_id(id)
{
}

SoundInstance::~SoundInstance()
{
	if (_audio != nullptr && _id != Sequence::InvalidIndex)
		Release();
}

SoundInstance::SoundInstance(SoundInstance&& move) :
	_audio(move._audio),
	_id(move._id)
{
	move.Reset();
}

auto SoundInstance::operator=(SoundInstance&& move) -> SoundInstance&
{
	if (_audio != nullptr && _id != Sequence::InvalidIndex)
		Stop();

	_audio = move._audio;
	_id = move._id;

	move.Reset();

	return *this;
}

void SoundInstance::Reset()
{
	_audio = nullptr;
	_id = Sequence::InvalidIndex;
}

void SoundInstance::Play()
{
	if (_audio != nullptr)
		_audio->PlaySound(*this);
}

void SoundInstance::Pause()
{
	if (_audio != nullptr)
		_audio->PauseSound(*this);
}

void SoundInstance::Stop()
{
	if (_audio != nullptr)
		_audio->StopSound(*this);

	Reset();
}

void SoundInstance::Release()
{
	if (_audio != nullptr)
		_audio->ReleaseSound(*this);

	Reset();
}

void SoundInstance::Continue()
{
	if (_audio != nullptr)
		_audio->ContinueSound(*this);
}

auto SoundInstance::GetVolume() const -> float
{
	return _audio == nullptr ? 0.0f : _audio->GetSoundVolume(*this);
}

void SoundInstance::SetVolume(float volume)
{
	if (_audio != nullptr)
		_audio->SetSoundVolume(*this, volume);
}

void SoundInstance::SetPanning(float left, float right)
{
	if (_audio != nullptr)
		_audio->SetPanning(*this, left, right);
}

auto SoundInstance::GetFrequency() const -> float
{
	return _audio == nullptr ? 1.0f : _audio->GetSoundFrequency(*this);
}

void SoundInstance::SetFrequency(float frequency)
{
	if (_audio != nullptr)
		_audio->SetSoundFrequency(*this, frequency);
}

auto SoundInstance::GetLocation() const -> int
{
	return _audio == nullptr ? 0 : _audio->GetSoundLocation(*this);
}

void SoundInstance::SetLocation(int location)
{
	if (_audio != nullptr)
		_audio->SetSoundLocation(*this, location);
}
