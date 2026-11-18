#pragma once

#include "dough/Core.h"
#include "dough/files/AFileData.h"

#include <SDL3/SDL_audio.h>

namespace DOH {

	//enum class EAudioFileType {
	//	NONE = 0,
	//
	//	MP3,
	//	WAV
	//};

	//constexpr static std::array<const char*, 3> EAudioFileType_Strings = {
	//	"NONE",
	//
	//	"MP3",
	//	"WAV"
	//};

	struct WavFileData : public AFileData {
		//AudioEngine uses SDL_AudioSpec for defining output so I'll use it here.
		//Even for file types that SDL doesn't have a loading function for.
		SDL_AudioSpec Spec;
		Uint8* Data;
		Uint32 DataLengthBytes;

		WavFileData(const char* filePath)
		:	AFileData(),
			Spec({ SDL_AUDIO_UNKNOWN, 0, 0 }),
			Data(nullptr),
			DataLengthBytes(0u)
		{}

		inline bool isCompatible(SDL_AudioSpec& comparison) const {
			if (Spec.format != comparison.format) return false;
			if (Spec.freq != comparison.freq) return false;
			if (Spec.channels != comparison.channels) return false;
			return true;
		}

		inline void close() {
			if (Data != nullptr) {
				SDL_free(Data);
				DataLengthBytes = 0u;
			}
		}
	};

	//TODO:: This
	//struct Mp3FileData : public AFileData {
	//
	//};
}
