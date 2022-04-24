#pragma once

#include "dough/Core.h"
#include "dough/scene/geometry/AGeometry.h"
#include "dough/rendering/TextureVulkan.h"

#include <typeinfo>

namespace DOH {

	template<typename T, typename = std::enable_if<std::is_base_of<AGeometry, T>::value>>
	class ARenderBatch {

	private:
		ARenderBatch operator=(const ARenderBatch& assignment) = delete;

	protected:
		const uint32_t MAX_GEOMETRY_COUNT;
		const uint32_t MAX_TEXTURE_COUNT;

		ARenderBatch(const uint32_t maxGeometryCount, const uint32_t geoByteSize, const uint32_t maxTextureCount)
		:	MAX_GEOMETRY_COUNT(maxGeometryCount),
			MAX_TEXTURE_COUNT(maxTextureCount),
			mData(maxGeometryCount * geoByteSize),
			mDataIndex(0),
			mGeometryCount(0),
			mNextTextureSlotIndex(0)
		{}

		std::vector<float> mData;
		uint32_t mDataIndex;
		uint32_t mGeometryCount;

		std::vector<std::reference_wrapper<TextureVulkan>> mTextureSlots;
		uint32_t mNextTextureSlotIndex;

	public:
		virtual void add(const T& geo, const uint32_t textureSlotIndex) = 0;
		virtual void addAll(const std::vector<T>& geoArray, const uint32_t textureSlotIndex) = 0;
		virtual void addAll(
			const std::vector<T>& geoArr,
			const size_t startIndex,
			const size_t endIndex,
			const uint32_t textureSlotIndex
		) = 0;

		bool hasTextureId(const uint32_t textureId) {
			for (TextureVulkan& texture : mTextureSlots) {
				if (texture.getId() == textureId) {
					return true;
				}
			}

			return false;
		}

		uint32_t getTextureSlotIndex(const uint32_t textureId) {
			for (size_t i = 0; i < mTextureSlots.size(); i++) {
				if (mTextureSlots[i].get().getId() == textureId) {
					return static_cast<uint32_t>(i);
				}
			}

			return -1;
		}

		uint32_t addNewTexture(TextureVulkan& texture) {
			uint32_t slotIndex = -1;

			if (hasTextureSlotAvailable()) {
				mTextureSlots.push_back(texture);
				slotIndex = mNextTextureSlotIndex;
				mNextTextureSlotIndex++;
			}

			return slotIndex;
		}

		inline void reset() {
			mDataIndex = 0;
			mGeometryCount = 0;
			mTextureSlots.clear();
			mNextTextureSlotIndex = 0;
		}

		inline uint32_t getDataIndex() const { return mDataIndex; }
		inline bool hasSpace(size_t geoCount) const { return mGeometryCount + geoCount <= MAX_GEOMETRY_COUNT; }
		inline size_t getRemainingGeometrySpace() const { return MAX_GEOMETRY_COUNT - mGeometryCount; }
		inline size_t getGeometryCount() const { return mGeometryCount; }
		inline const std::vector<float> getData() const { return mData; }
		inline const std::vector<std::reference_wrapper<TextureVulkan>> getTextureSlots() const { return mTextureSlots; }
		inline bool hasTextureSlotAvailable() { return mNextTextureSlotIndex < MAX_TEXTURE_COUNT; }
	};

}
