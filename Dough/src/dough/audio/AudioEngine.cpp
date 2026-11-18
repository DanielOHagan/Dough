#include "dough/audio/AudioEngine.h"

#include "dough/Logging.h"

//TEMP:: For testing sound
#include "dough/input/Input.h"
#include "dough/input/InputCodes.h"
#include "dough/time/Time.h"

#include <tracy/public/tracy/Tracy.hpp>

namespace DOH {

	AudioOutputTarget::AudioOutputTarget(SDL_AudioFormat format, int channelCount, int frequency, int bufferSampleCount, uint32_t totalBuffersSampleCount)
	:	Spec({ format, channelCount, frequency }),
		AudioStream(nullptr),
		BufferSampleCount(bufferSampleCount),
		BufferSizeBytes(bufferSampleCount * sizeof(float)),
		TotalBuffersSampleCount(totalBuffersSampleCount)
	{}

	void AudioOutputTarget::open() {
		ZoneScoped;

		AudioStream = SDL_OpenAudioDeviceStream(
			SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK,
			&Spec,
			NULL,
			NULL
		);
		if (AudioStream == nullptr) {
			LOG_ERR("Failed to create SDL audio stream: " << SDL_GetError());
			return;
		}

		SDL_ResumeAudioStreamDevice(AudioStream);
	}

	uint32_t AudioOutputTarget::getDesiredSampleCountToAdd() {
		if (getQueuedSampleCount() > TotalBuffersSampleCount) {
			return 0u;
		} else {
			return static_cast<uint32_t>(BufferSampleCount);
		}
	}

	AudioEngine::AudioEngine()
	:	mReady(false)
	{}

	void AudioEngine::init() {
		ZoneScoped;

		if (!SDL_Init(SDL_INIT_AUDIO)) {
			LOG_ERR("Failed to initialise SDL Audio");
			mReady = false;
			return;
		}

		mOutputTarget = std::make_unique<AudioOutputTarget>(
			SDL_AudioFormat::SDL_AUDIO_F32,
			AudioEngine::CHANNEL_COUNT,
			AudioEngine::FREQUENCY,
			AudioEngine::BUFFER_SAMPLE_COUNT,
			AudioEngine::TOTAL_BUFFER_SAMPLE_COUNT
		);
		mOutputTarget->open();

		mReady = mOutputTarget->isOpen();
	}

	void AudioEngine::update() {
		ZoneScoped;

		if (Input::getInputLayers().size() > 0) {
			auto& inputLayer = Input::getInputLayers().front();

			static EMusicNote note0 = EMusicNote::C;
			static float octave0 = 4.0f;
			static EMusicNote note1 = EMusicNote::C;
			static float octave1 = 4.0f;

			EMusicNote* targetNote = &note0;
			float* targetOctave = &octave0;

			if (inputLayer->isKeyPressed(DOH_KEY_LEFT_SHIFT)) {
				targetNote = &note1;
				targetOctave = &octave1;
			} else {
				targetNote = &note0;
				targetOctave = &octave0;
			}

			if (inputLayer->isKeyPressedConsume(DOH_KEY_UP)) *targetOctave += 1.0f;
			if (inputLayer->isKeyPressedConsume(DOH_KEY_DOWN)) *targetOctave -= 1.0f;
			if (inputLayer->isKeyPressedConsume(DOH_KEY_M)) *targetOctave = 4.0f;

			if (inputLayer->isKeyPressedConsume(DOH_KEY_Q)) *targetNote = EMusicNote::A;
			if (inputLayer->isKeyPressedConsume(DOH_KEY_W)) *targetNote = EMusicNote::A_SHARP;
			if (inputLayer->isKeyPressedConsume(DOH_KEY_E)) *targetNote = EMusicNote::B;
			if (inputLayer->isKeyPressedConsume(DOH_KEY_R)) *targetNote = EMusicNote::C;
			if (inputLayer->isKeyPressedConsume(DOH_KEY_T)) *targetNote = EMusicNote::C_SHARP;
			if (inputLayer->isKeyPressedConsume(DOH_KEY_Y)) *targetNote = EMusicNote::D;
			if (inputLayer->isKeyPressedConsume(DOH_KEY_U)) *targetNote = EMusicNote::D_SHARP;
			if (inputLayer->isKeyPressedConsume(DOH_KEY_I)) *targetNote = EMusicNote::E;
			if (inputLayer->isKeyPressedConsume(DOH_KEY_O)) *targetNote = EMusicNote::F;
			if (inputLayer->isKeyPressedConsume(DOH_KEY_P)) *targetNote = EMusicNote::F_SHARP;
			if (inputLayer->isKeyPressedConsume(DOH_KEY_A)) *targetNote = EMusicNote::G;
			if (inputLayer->isKeyPressedConsume(DOH_KEY_S)) *targetNote = EMusicNote::G_SHARP;

			//Log note/octave change.
			static EMusicNote notePlaying = note0;
			static float octavePlaying = octave0;
			static bool logFirstNote = true;
			if (notePlaying != note0 || octavePlaying != octave0 || logFirstNote) {
				notePlaying = note0;
				octavePlaying = octave0;
				float noteFreq = getNoteFreq(note0, octave0);
				LOG_INFO("Note0: " << EMusicNote_Strings[static_cast<size_t>(note0)] << " \t\tFreq: " << noteFreq);
				logFirstNote = false;
			}

			if (inputLayer->isKeyPressed(DOH_KEY_SPACE)) {
				DEBUG_playNoise(note0, octave0, note1, octave1);
			}
		}
	}

	void AudioEngine::addToAudioStream(AudioOutputTarget& target, AudioTransferTickData& data) {
		ZoneScoped;

		//TODO:: This will be called upto, ideally, 200 times a second so this has to be fast.

		//TODO:: inline this and make it work for different sample types.
		SDL_PutAudioStreamData(
			mOutputTarget->AudioStream,
			data.Data,
			data.QueuedSampleCount * sizeof(float)
		);
	}

	void AudioEngine::addToAudioStreamUpToLimit(AudioOutputTarget& target, AudioTransferTickData& data, uint32_t sampleLimit) {
		ZoneScoped;

		//TODO:: ~5.8ms of samples is added every 5ms, I want to keep track of that 0.8ms extra and when that reaches a certain amount
		// I only ask AppLogic for a smaller amount of samples once, this is to use up the upcoming samples and reduce audio lag.

		//TODO:: Should I have the sampleLimit or equivalent as a member of AudioOutputTarget ?

		const int alreadyQueuedSampleBytes = target.getQueuedSampleSizeBytes();
		const int desiredSamplesBytes = static_cast<int>(data.QueuedSampleCount) * sizeof(float);
		//TODO:: Currently only float32 is supported.
		const int sampleLimitBytes = static_cast<int>(sampleLimit) * sizeof(float);
		const int samplesToAddBytes = std::min(sampleLimitBytes, alreadyQueuedSampleBytes + desiredSamplesBytes);

		if (samplesToAddBytes > 0) {
			//Add all of audio tick data to queue, this may go over the  size which is fine but will mean that
			//	we'll just have to wait until those extra samples are finished with.
			if (data.QueuedSampleCount > 0u) {
				SDL_PutAudioStreamData(
					target.AudioStream,
					data.Data,
					samplesToAddBytes
				);
			// 	//LOG_INFO("Put Samples: " << data.QueuedSampleCount);
			}
		}
	}

	void AudioEngine::close() {
		ZoneScoped;

		if (mOutputTarget != nullptr && mOutputTarget->isOpen()) {
			SDL_DestroyAudioStream(mOutputTarget->AudioStream);
			mOutputTarget = nullptr;
		}
	}

	void AudioEngine::DEBUG_playNoise(EMusicNote note0, float octave0, EMusicNote note1, float octave1) {
		ZoneScoped;

		//My own attempt
		//Add current "time slot of samples" and next slot's as a buffer
 		const int queuedSampleBytes = mOutputTarget->getQueuedSampleSizeBytes();
		const int thresholdToAddSamples = mOutputTarget->BufferSizeBytes + AudioEngine::EXTRA_BUFFERS_SIZE_BYTES;
		if (queuedSampleBytes <= thresholdToAddSamples) {
			static float samples[AudioEngine::BUFFER_SAMPLE_COUNT];
			const float noteFreq0 = getNoteFreq(note0, octave0);
			const float noteFreq1 = getNoteFreq(note1, octave1);
			
			//TODO:: pretty sure the frequent popping is caused by this in some way. Maybe if this number gets high enough (it can get upto FREQUENCY - 1).
			static int currentSineSample = 1;
		
			//TODO:: I think I'll want to always add the whole chunk because the idea will be that one-chunk = one target update time.
			//	However, if this is a "buffered" adding then we might only want to add what is available incase the application
			//	gives more data to add next "audio transfer". Maybe... it might be a problem if only 1 byte is added to the buffer leavning very little
			//	as an actual buffer (a.k.a wriggle room).
			constexpr bool const addOnlyUsedPartOfChunk = false;
			const int addedBytes = addOnlyUsedPartOfChunk ? 
				mOutputTarget->BufferSizeBytes - queuedSampleBytes :
				mOutputTarget->BufferSizeBytes;
		
			//Generate a sine wave of noteFreq.
			const int sampleCountToAdd = (addedBytes / sizeof(float));
			for (int i = 0; i < sampleCountToAdd; i += mOutputTarget->Spec.channels) {
				//Channel 0
				const float phase0 = currentSineSample * noteFreq0 / static_cast<float>(mOutputTarget->Spec.freq);
				const float val0 = SDL_sinf(phase0 * 2 * SDL_PI_F);
				samples[i] = val0;

				//Channel 1
				const float phase1 = currentSineSample * noteFreq1 / static_cast<float>(mOutputTarget->Spec.freq);
				const float val1 = SDL_sinf(phase1 * 2 * SDL_PI_F);
				samples[i + 1] = val1;
				currentSineSample++;
			}
			//int
			//Prevent sine sample from equaling 0
			currentSineSample %= mOutputTarget->Spec.freq; // wrapping around to avoid floating-point errors
			if (currentSineSample > 0) {} else currentSineSample++;

			SDL_PutAudioStreamData(mOutputTarget->AudioStream, samples, addedBytes);
		}
	}
}
