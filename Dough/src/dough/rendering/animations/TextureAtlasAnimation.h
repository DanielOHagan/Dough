#pragma once

#include "dough/Core.h"

namespace DOH {

	struct InnerTexture;

	/**
	* Different types of ___Animation and ___AnimationController classes may be made to support different needs.
	* E.g. Instead of using time and duration a controller could be made to use a .nextFrame() to cylce through
	* the frames of an animation.
	* 
	* TODO:: ^^ FrameCycle animation controller
	*/

	/**
	* ___Animation objects store the data of an animation, they are not meant to be interacted with by an
	* ___AnimationController instance.
	*/
	class TextureAtlasAnimation {
	private:
		std::vector<InnerTexture> mInnerTextures;
		float mDuration;
		bool mLooping;

	public:
		TextureAtlasAnimation(const std::vector<InnerTexture>& innerTextures, float duration, bool looping);
		
		inline const std::vector<InnerTexture>& getInnerTextures() const { return mInnerTextures; }
		inline float getDuration() const { return mDuration; }
		inline bool isLooping() const { return mLooping; }
	};

	/**
	* ___AniamtionControllers are objects that add logic to an ___Animation object to form an animation API depending
	* on how the controller is supposed to work.
	* 
	* ___AnimationControllers represent 1 instance of an animation in use. E.g, if there are two world objects that 
	* are using the same animation but are at different intervals then there will be two instances of an ___AnimationController
	* that hold a reference to the same ___Animation object.
	*/
	class TextureAtlasAnimationController {
	private:
		const TextureAtlasAnimation& mAnimation;

		float mFrameTime;
		float mCurrentTime;
		uint32_t mCurrentTextureIndex;
		bool mPlaying;

	public:
		TextureAtlasAnimationController(const TextureAtlasAnimation& animation);

		inline void play() { setPlaying(true); }
		inline void pause() { setPlaying(false); }
		inline void setPlaying(bool playing) { mPlaying = playing; }
		bool update(float delta);
		void reset();

		inline const InnerTexture& getCurrentInnerTexture() const { return mAnimation.getInnerTextures()[mCurrentTextureIndex]; }
		inline uint32_t getCurrentTextureIndex() const { return mCurrentTextureIndex; }
		inline bool isPlaying() const { return mPlaying; }
	};
}
