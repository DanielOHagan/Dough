#pragma once

#include "dough/Core.h"
#include "dough/scene/geometry/AGeometry.h"
#include "dough/rendering/TextureVulkan.h"

#include <typeinfo>

namespace DOH {

	template<typename T, typename = std::enable_if<std::is_base_of<AGeometry, T>::value>>
	class ARenderBatch {

	private:
		ARenderBatch(const ARenderBatch& copy) = delete;
		ARenderBatch operator=(const ARenderBatch& assignment) = delete;

	protected:
		const uint32_t MAX_GEOMETRY_COUNT;
		const uint32_t MAX_TEXTURE_COUNT;

		ARenderBatch(uint32_t maxGeometryCount, uint32_t geoComponentCount, uint32_t maxTextureCount)
		:	MAX_GEOMETRY_COUNT(maxGeometryCount),
			MAX_TEXTURE_COUNT(maxTextureCount),
			mData(maxGeometryCount* geoComponentCount),
			mDataIndex(0),
			mGeometryCount(0),
			mNextTextureSlotIndex(0)
		{
			//mData.resize(maxGeometryCount * geoComponentCount);
		}

		std::vector<float> mData;
		uint32_t mDataIndex;
		uint32_t mGeometryCount;

		std::vector<std::reference_wrapper<TextureVulkan>> mTextureSlots;
		uint32_t mNextTextureSlotIndex;

	public:
		virtual void add(T& geo, uint32_t textureSlotIndex) = 0;
		virtual void addAll(std::vector<T> geoArray, uint32_t textureSlotIndex) = 0;

		bool hasTextureId(uint32_t textureId) {
			for (TextureVulkan& texture : mTextureSlots) {
				if (texture.getId() == textureId) {
					return true;
				}
			}

			return false;
		}

		uint32_t getTextureSlotIndex(uint32_t textureId) {
			for (size_t i = 0; i < mTextureSlots.size(); i++) {
				if (mTextureSlots[i].getId() == textureId) {
					return static_cast<uint32_t>(i);
				}
			}
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
		inline bool hasSpace(uint32_t geoCount) const { return getRemainingGeometrySpace() > 0; }
		inline uint32_t getRemainingGeometrySpace() const { return MAX_GEOMETRY_COUNT - mGeometryCount; }
		inline uint32_t getGeometryCount() const { return mGeometryCount; }
		inline std::vector<float> getData() { return mData; }
		inline bool hasTextureSlotAvailable() { return mNextTextureSlotIndex < MAX_TEXTURE_COUNT; }
	};

}
