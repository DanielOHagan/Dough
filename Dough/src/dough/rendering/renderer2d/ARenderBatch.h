#pragma once

#include "dough/Core.h"
#include "dough/scene/geometry/AGeometry.h"
#include "dough/rendering/textures/TextureVulkan.h"

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
			mGeometryCount(0)
		{}

		std::vector<float> mData;
		uint32_t mDataIndex;
		uint32_t mGeometryCount;

	public:
		virtual void add(const T& geo, const uint32_t textureSlotIndex) = 0;
		virtual void addAll(const std::vector<T>& geoArray, const uint32_t textureSlotIndex) = 0;
		virtual void addAll(
			const std::vector<T>& geoArr,
			const size_t startIndex,
			const size_t endIndex,
			const uint32_t textureSlotIndex
		) = 0;

		inline void reset() {
			mDataIndex = 0;
			mGeometryCount = 0;
		}

		inline uint32_t getDataIndex() const { return mDataIndex; }
		inline bool hasSpace(size_t geoCount) const { return mGeometryCount + geoCount <= MAX_GEOMETRY_COUNT; }
		inline size_t getRemainingGeometrySpace() const { return MAX_GEOMETRY_COUNT - mGeometryCount; }
		inline size_t getGeometryCount() const { return mGeometryCount; }
		inline const std::vector<float> getData() const { return mData; }
	};

}
