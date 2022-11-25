#pragma once

#include "dough/Core.h"
#include "dough/rendering/textures/TextureVulkan.h"
#include "dough/Maths.h"

namespace DOH {

	class MonoSpaceTextureAtlas : public TextureVulkan {

	private:
		const uint32_t mRowCount;
		const uint32_t mColCount;
		const float mNormalisedInnerTextureWidth;
		const float mNormalisedInnerTextureHeight;

	public:
		MonoSpaceTextureAtlas() = delete;
		MonoSpaceTextureAtlas(const MonoSpaceTextureAtlas& copy) = delete;
		MonoSpaceTextureAtlas operator=(const MonoSpaceTextureAtlas& assignment) = delete;

		MonoSpaceTextureAtlas(
			VkDevice logicDevice,
			VkPhysicalDevice physicalDevice,
			const std::string& filePath,
			const uint32_t rowCount,
			const uint32_t colCount
		);

		//Row & columns range from 0 -> (row or column count - 1).
		inline glm::vec2 getInnerTextureCoordsOrigin(const uint32_t row, const uint32_t col) const {
			return { row * mNormalisedInnerTextureWidth, col * mNormalisedInnerTextureHeight };
		}
		std::array<float, 8> getInnerTextureCoords(const uint32_t row, const uint32_t col) const;
		std::array<float, 8> getInnerTextureCoords(const uint32_t rowTop, const uint32_t rowBot, const uint32_t colLeft, const uint32_t colRight) const;
		const uint32_t getRowCount() const { return mRowCount; }
		const uint32_t getColCount() const { return mColCount; }
	};
}
