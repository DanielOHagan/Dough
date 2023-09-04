#include "dough/rendering/animations/TextureAtlasAnimation.h"

#include "dough/rendering/textures/TextureAtlas.h"

namespace DOH {

	TextureAtlasAnimation::TextureAtlasAnimation(
		const std::vector<InnerTexture>& innerTextures,
		float duration,
		bool looping
	) : mInnerTextures(innerTextures),
		mDuration(duration),
		mLooping(looping)
	{}

	TextureAtlasAnimationController::TextureAtlasAnimationController(const TextureAtlasAnimation& animation)
	:	mAnimation(animation),
		mFrameTime(animation.getDuration() / static_cast<float>(animation.getInnerTextures().size())),
		mCurrentTime(0.0f),
		mCurrentTextureIndex(0),
		mPlaying(false)
	{}

	bool TextureAtlasAnimationController::update(float delta) {
		if (mPlaying) {
			mCurrentTime += delta;

			if (mCurrentTime > mFrameTime) {
				mCurrentTime -= mFrameTime;

				mCurrentTextureIndex++;
				const uint32_t lastIndex = static_cast<uint32_t>(mAnimation.getInnerTextures().size());

				if (mCurrentTextureIndex < lastIndex) {
					return true;
				} else if (mCurrentTextureIndex == lastIndex && mAnimation.isLooping()) {
					mCurrentTextureIndex = 0;
					return true;
				} else {
					mCurrentTextureIndex--;
				}
			}
		}

		return false;
	}
	
	void TextureAtlasAnimationController::reset() {
		mCurrentTime = 0.0f;
		mCurrentTextureIndex = 0;
	}
}
