#pragma once

#include "Pargon/Audio.h"
#include <memory>

namespace Pargon
{
	auto CreateXAudio2Player() -> std::unique_ptr<AudioPlayer>;
}
