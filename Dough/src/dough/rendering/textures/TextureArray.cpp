#include "dough/rendering/textures/TextureArray.h"

namespace DOH {

	TextureArray::TextureArray(const uint32_t maxTextureCount, TextureVulkan& fallbackTexture)
	:	MAX_TEXTURE_COUNT(maxTextureCount),
		FALLBACK_TEXTURE(fallbackTexture),
		mNextTextureSlotIndex(0)
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
		mNextTextureSlotIndex(0)
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

		return 0;
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

	const int TextureArray::isTextureInUse(const uint32_t textureId) const {
		for (int i = 0; i < mTextureSlots.size(); i++) {
			if (mTextureSlots[i].get().getId() == textureId) {
				return i;
			}
		}

		return -1;
	}
}
