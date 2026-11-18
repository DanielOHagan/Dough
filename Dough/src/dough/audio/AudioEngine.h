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
		//Total number of samples the current "buffer" and any extras can hold.
		//This is NOT a hard limit 
		uint32_t TotalBuffersSampleCount;

		AudioOutputTarget(SDL_AudioFormat format, int channelCount, int frequency, int bufferSampleCount, uint32_t totalBuffersSampleCount);

		void open();
		uint32_t getDesiredSampleCountToAdd();

		//Get the size of queued samples in bytes.
		inline int getQueuedSampleSizeBytes() const { return SDL_GetAudioStreamQueued(AudioStream); }
		inline uint32_t getQueuedSampleCount() const { return static_cast<uint32_t>(getQueuedSampleSizeBytes() / sizeof(float)); }
		//TODO:: how to handle different sample types
		inline bool hasEnoughSamplesInBytes(int bytes) const { return getQueuedSampleSizeBytes() >= (bytes * Spec.freq); }
		inline bool hasEnoughSamples(uint32_t sampleCount) const {
			return static_cast<uint32_t>(getQueuedSampleSizeBytes() / sizeof(float)) >= sampleCount;
		}
		//inline bool hasSpaceForSamples(uint32_t sampleCount) const {
		//	return TotalBufferSampleCount > sampleCount;
		//}

		inline bool isOpen() const { return AudioStream != nullptr; }
	};

	//TODO:: Handle int format as a backup.
	//TODO:: SDL uses int, would it be better for me to use uint32_t as that's the preferred int type in the engine.
	//	Maybe just use it for init settings and not runtime as it would require more type casting.
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
		constexpr static const int TOTAL_BUFFER_SAMPLE_COUNT = BUFFER_SAMPLE_COUNT + EXTRA_BUFFERS_SAMPLE_COUNT;

		std::unique_ptr<AudioOutputTarget> mOutputTarget;
		bool mReady;

	public:
		AudioEngine();

		void init();
		void close();

		void update();

		/**
		* Add audio data to target audio ouput, data.QueuedSampleCount of samples from data.Data is added.
		* This way you queue up as many samples as you want.
		* 
		* @param target The audio output target, this is usually a device (e.g. headset or speaker).
		* @param data The data to transfer to the audio stream. data.QueuedSampleCount MUST be > 0u and data.Data MUST be a valid pointer.
		*/
		void addToAudioStream(AudioOutputTarget& target, AudioTransferTickData& data);
		/**
		* Add audio data to target audio output up to a desired number of total queued samples.
		* If ouput target's queued sample count is already over the limit then no samples are added.
		* 
		* @param target The audio output target, this is usually a device (e.g. headset or speaker).
		* @param data The data to transfer to the audio stream. data.QueuedSampleCount MUST be > 0u and data.Data MUST be a valid pointer.
		*/
		void addToAudioStreamUpToLimit(AudioOutputTarget& target, AudioTransferTickData& data, uint32_t sampleLimit);

		inline bool isReady() const { return mReady; }
		inline bool hasOutput() const { return mOutputTarget != nullptr && mOutputTarget->isOpen(); }
		inline AudioOutputTarget& getPrimaryAudioOutputTarget() { return *mOutputTarget; }
	private:
		void DEBUG_playNoise(EMusicNote note0, float octave0, EMusicNote note1, float octave1);
	};
}
