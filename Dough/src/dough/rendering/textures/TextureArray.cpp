#include "dough/rendering/textures/TextureArray.h"

namespace DOH {

	TextureArray::TextureArray(const uint32_t maxTextureCount, TextureVulkan& fallbackTexture)
	:	MAX_TEXTURE_COUNT(maxTextureCount),
		FALLBACK_TEXTURE(fallbackTexture),
		mNextTextureSlotIndex(0u)
	{}

	TextureArray::TextureArray(
		const uint32_t maxTextureCount,
		TextureVulkan& fallbackTexture,
		std::initializer_list<std::reference_wrapper<TextureVulkan>> textures
	) : MAX_TEXTURE_COUNT(maxTextureCount),
		mTextureSlots(textures),
		FALLBACK_TEXTURE(fallbackTexture),
		mNextTextureSlotIndex(static_cast<uint32_t>(textures.size()))
	{}

	TextureArray::TextureArray(
		const uint32_t maxTextureCount,
		TextureVulkan& fallbackTexture,
		std::vector<std::reference_wrapper<TextureVulkan>>& textures
	) : MAX_TEXTURE_COUNT(maxTextureCount),
		FALLBACK_TEXTURE(fallbackTexture),
		mNextTextureSlotIndex(0u)
	{
		for (TextureVulkan& texture : textures) {
			mTextureSlots.push_back(texture);
			mNextTextureSlotIndex++;
		}
	}

	bool TextureArray::hasTextureId(const uint32_t textureId) const {
		for (TextureVulkan& texture : mTextureSlots) {
			if (texture.getId() == textureId) {
				return true;
			}
		}

		return false;
	}

	uint32_t TextureArray::getTextureSlotIndex(const uint32_t textureId) const {
		for (size_t i = 0; i < mTextureSlots.size(); i++) {
			if (mTextureSlots[i].get().getId() == textureId) {
				return static_cast<uint32_t>(i);
			}
		}

		return -1;
	}

	uint32_t TextureArray::addNewTexture(TextureVulkan& texture) {
		const int slot = isTextureInUse(texture.getId());
		if (slot != -1) {
			return static_cast<uint32_t>(slot);
		}

		uint32_t slotIndex = 0;
		if (hasTextureSlotAvailable()) {
			mTextureSlots.push_back(texture);
			slotIndex = mNextTextureSlotIndex;
			mNextTextureSlotIndex++;
		}

		return slotIndex;
	}

	void TextureArray::removeTexture(TextureVulkan& texture) {
		if (mNextTextureSlotIndex > 0u) {
			for (
				std::vector<std::reference_wrapper<TextureVulkan>>::iterator itr = mTextureSlots.begin();
				itr != mTextureSlots.end();
				++itr
			) {
				if (itr->get().getId() == texture.getId()) {
					mTextureSlots.erase(itr);
					break;
				}
			}
			mNextTextureSlotIndex -= 1u;
		}
	}

	void TextureArray::removeTextures(std::initializer_list<std::reference_wrapper<TextureVulkan>>& textures) {
		
		//Single mTextureSlot pass (faster when mTextureSlots is LARGER than textures)
		//NOTE:: Using .erase() invaidates iterators so I'd have to write something that can remove x amount of textures and repacks them
		//	in the vector.
		//for (
		//	std::vector<std::reference_wrapper<TextureVulkan>>::iterator itr = mTextureSlots.begin();
		//	itr != mTextureSlots.end();
		//	++itr
		//) {
		//	for (const std::reference_wrapper<TextureVulkan>& textureToRemove : textures) {
		//		if (itr->get().getId() == textureToRemove.get().getId()) {
		//			mTextureSlots.erase(itr);
		//			break;
		//		}
		//	}
		//}

		//Single textures pass (faster when mTextureSlots is SMALLER than textures)
		for (const std::reference_wrapper<TextureVulkan>& textureToRemove : textures) {
			for (
				std::vector<std::reference_wrapper<TextureVulkan>>::iterator itr = mTextureSlots.begin();
				itr != mTextureSlots.end();
				++itr
			) {
				if (itr->get().getId() == textureToRemove.get().getId()) {
					mTextureSlots.erase(itr);
					break;
				}
			}
		}
	}

	const int TextureArray::isTextureInUse(const uint32_t textureId) const {
		for (int i = 0; i < mTextureSlots.size(); i++) {
			if (mTextureSlots[i].get().getId() == textureId) {
				return i;
			}
		}
		return -1;
	}
}
