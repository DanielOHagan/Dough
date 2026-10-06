#pragma once

#include <SDL3/SDL.h>

#include "dough/Core.h"
#include "dough/audio/AudioTransferTickData.h"
#include "dough/audio/EMusicNote.h"

namespace DOH {

	struct AudioOutputTarget {
		SDL_AudioSpec Spec;
		SDL_AudioStream* AudioStream;
		int BufferSampleCount;
		int BufferSizeBytes;

		AudioOutputTarget(SDL_AudioFormat format, int channelCount, int frequency, int bufferSampleCount);

		void open();

		//Get the size of queued samples in bytes.
		inline int getQueuedSampleSizeBytes() const { return SDL_GetAudioStreamQueued(AudioStream); }
		//TODO:: how to handle different sample types
		inline bool hasEnoughSamplesInBytes(int bytes) const { return getQueuedSampleSizeBytes() > (bytes * Spec.freq); }

		inline bool isOpen() const { return AudioStream != nullptr; }
	};

	//TODO:: Handle int16 format as a backup.
	class AudioEngine {
	private:
		//TODO:: Link sample transfer sizes to mAppLoop.AudioTransferTime

		constexpr static const int FREQUENCY = 44100;
		constexpr static const int CHANNEL_COUNT = 2;
		constexpr static const int SAMPLE_COUNT = 256; //TODO:: LINK THIS TO mAppLoop AUDIO TRANSFER SIZES!!!
		//Number of samples per chunk to transfer.
		constexpr static const int BUFFER_SAMPLE_COUNT = SAMPLE_COUNT * CHANNEL_COUNT;
		constexpr static const int BUFFER_CHUNK_SIZE_BYTES = BUFFER_SAMPLE_COUNT * sizeof(float);
		//Number of EXTRA samples to transfer to help ensure the audio stream has samples.
		//Increasing this means this will generally increase audio input lag but will give the app more time to provide samples.
		constexpr static const int EXTRA_BUFFERS_COUNT = 1;
		constexpr static const int EXTRA_BUFFERS_SAMPLE_COUNT = BUFFER_SAMPLE_COUNT * EXTRA_BUFFERS_COUNT;
		constexpr static const int EXTRA_BUFFERS_SIZE_BYTES = EXTRA_BUFFERS_SAMPLE_COUNT * sizeof(float);

		std::unique_ptr<AudioOutputTarget> mOutputTarget;
		bool mReady;

	public:
		AudioEngine();

		void init();
		void close();

		void update();

		//Assumes there is a valid output target
		void addToAudioStream(AudioTransferTickData data);

		inline bool isReady() const { return mReady; }
		inline bool hasOutput() const { return mOutputTarget != nullptr && mOutputTarget->isOpen(); }
		inline AudioOutputTarget& getPrimaryAudioOutputTarget() { return *mOutputTarget; }
	private:
		void DEBUG_playNoise(EMusicNote note0, float octave0, EMusicNote note1, float octave1);
	};
}
