#include "dough/audio/AudioEngine.h"

#include "dough/Logging.h"

//TEMP:: For testing sound
#include "dough/input/Input.h"
#include "dough/input/InputCodes.h"

namespace DOH {

	void AudioEngine::init() {
		SDL_AudioSpec spec = {};

		if (!SDL_Init(SDL_INIT_AUDIO)) {
			LOG_ERR("Failed to initialise SDL Audio");
			mReady = false;
			return;
		}

		spec.channels = 1;
		spec.format = SDL_AUDIO_F32;
		spec.freq = 8000;
		mAudioStream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, NULL, NULL);
		if (mAudioStream == nullptr) {
			LOG_ERR("Failed to create SDL audio stream: " << SDL_GetError());
			mReady = false;
			return;
		}

		SDL_ResumeAudioStreamDevice(mAudioStream);

		mReady = true;
	}

	void AudioEngine::update() {

		if (Input::getInputLayers().size() > 0) {
			auto& inputLayer = Input::getInputLayers().front();
			if (inputLayer->isKeyPressed(DOH_KEY_SPACE)) {

				/* see if we need to feed the audio stream more data yet.
			   We're being lazy here, but if there's less than half a second queued, generate more.
			   A sine wave is unchanging audio--easy to stream--but for video games, you'll want
			   to generate significantly _less_ audio ahead of time! */
				const int minimum_audio = (8000 * sizeof(float)) / 2;  /* 8000 float samples per second. Half of that. */
				if (SDL_GetAudioStreamQueued(mAudioStream) < minimum_audio) {
					static float samples[512];  /* this will feed 512 samples each frame until we get to our maximum. */
					int i;

					/* generate a 440Hz pure tone */
					for (i = 0; i < SDL_arraysize(samples); i++) {
						const int freq = 440;
						const float phase = currentSineSample * freq / 8000.0f;
						samples[i] = SDL_sinf(phase * 2 * SDL_PI_F);
						currentSineSample++;
					}

					/* wrapping around to avoid floating-point errors */
					currentSineSample %= 8000;

					/* feed the new data to the stream. It will queue at the end, and trickle out as the hardware needs more data. */
					SDL_PutAudioStreamData(mAudioStream, samples, sizeof(samples));
				}
			}
		}
	}

	void AudioEngine::close() {
		
	}
}
