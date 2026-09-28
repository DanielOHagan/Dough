#pragma once

#include <SDL3/SDL.h>

namespace DOH {

	class AudioEngine {
	private:
		SDL_AudioStream* mAudioStream;
		int currentSineSample = 0;

		bool mReady = false;

	public:
		void init();
		void close();

		void update();

		inline bool isReady() const { return mReady; }
	};
}
