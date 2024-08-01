#pragma once

#include "dough/Core.h"
#include "dough/rendering/textures/TextureVulkan.h"

namespace DOH {

	class TextureArray {

	private:
		const uint32_t MAX_TEXTURE_COUNT;
		const TextureVulkan& FALLBACK_TEXTURE;

		std::vector<std::reference_wrapper<TextureVulkan>> mTextureSlots;
		uint32_t mNextTextureSlotIndex;

	public:
		TextureArray(
			const uint32_t maxTextureCount,
			TextureVulkan& fallbackTexture
		);
		TextureArray(
			const uint32_t maxTextureCount,
			TextureVulkan& fallbackTexture,
			std::initializer_list<std::reference_wrapper<TextureVulkan>> textures
		);
		TextureArray(
			const uint32_t maxTextureCount,
			TextureVulkan& fallbackTexture,
			std::vector<std::reference_wrapper<TextureVulkan>>& texture
		);

		bool hasTextureId(const uint32_t textureId) const;
		//Search through texture slots for matching texture id, else return -1.
		uint32_t getTextureSlotIndex(const uint32_t textureId) const;

		//TODO:: Sometimes when this is called it is after hasTextureSlotAvailable() has already been called,
		// decide whether to assume this has already been called or whether to make it so it doesn't completely matter

		// Attemp to add a new texture, if successful return its index, else return 0
		uint32_t addNewTexture(TextureVulkan& texture);

		inline void reset() {
			mTextureSlots.clear();
			mNextTextureSlotIndex = 0;
		}

		inline const std::vector<std::reference_wrapper<TextureVulkan>> getTextureSlots() const { return mTextureSlots; }
		inline bool hasTextureSlotAvailable() const { return mNextTextureSlotIndex < MAX_TEXTURE_COUNT; }
		inline bool hasTextureSlotsAvailable(uint32_t slotCount) const { return (mNextTextureSlotIndex + slotCount) < MAX_TEXTURE_COUNT - 1; }
		inline const uint32_t getMaxTextureCount() const { return MAX_TEXTURE_COUNT; }
		inline const uint32_t getCurrentTextureCount() const { return static_cast<uint32_t>(mTextureSlots.size()); }
		inline const uint32_t getNextTextureSlotIndex() const { return mNextTextureSlotIndex; }
		inline const TextureVulkan& getFallbackTexture() const { return FALLBACK_TEXTURE; }

	private:
		//Internal function to find whether a texture is in use
		// returns texture slot index as int, or -1 if not found
		const int isTextureInUse(const uint32_t textureId) const;
	};
}
