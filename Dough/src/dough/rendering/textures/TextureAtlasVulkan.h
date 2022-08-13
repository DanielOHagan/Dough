#pragma once

#include "dough/Core.h"
#include "dough/rendering/textures/TextureVulkan.h"
#include "dough/Maths.h"

namespace DOH {

	class MonoSpaceTextureAtlasVulkan : public TextureVulkan {

	private:
		const uint32_t mRowCount;
		const uint32_t mColCount;
		float mNormalisedInnerTextureWidth;
		float mNormalisedInnerTextureHeight;

	public:
		MonoSpaceTextureAtlasVulkan() = delete;
		MonoSpaceTextureAtlasVulkan(const MonoSpaceTextureAtlasVulkan& copy) = delete;
		MonoSpaceTextureAtlasVulkan operator=(const MonoSpaceTextureAtlasVulkan& assignment) = delete;

		MonoSpaceTextureAtlasVulkan(
			VkDevice logicDevice,
			VkPhysicalDevice physicalDevice,
			const std::string& filePath,
			const uint32_t rowCount,
			const uint32_t colCount
		);

		//Row & columns range from 0 -> (row or column count - 1).
		glm::vec2 getInnerTextureCoordsOrigin(const uint32_t row, const uint32_t col) const;
		std::array<float, 8> getInnerTextureCoords(const uint32_t row, const uint32_t col) const;
		std::array<float, 8> getInnerTextureCoords(const uint32_t rowTop, const uint32_t rowBot, const uint32_t colLeft, const uint32_t colRight) const;
		const uint32_t getRowCount() const { return mRowCount; }
		const uint32_t getColCount() const { return mColCount; }
	};
}
