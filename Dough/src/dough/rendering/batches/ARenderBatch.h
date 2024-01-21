#pragma once

#include "dough/Core.h"
#include "dough/scene/geometry/AGeometry.h"
#include "dough/rendering/textures/TextureVulkan.h"
#include "dough/rendering/Config.h"

#include <typeinfo>

//Currently batching has significantly different performance on different configurations
#define MAX_BATCH_QUAD_COUNT
#if defined (_DEBUG)
	#undef MAX_BATCH_QUAD_COUNT
	#define MAX_BATCH_QUAD_COUNT 4
#else
	#undef MAX_BATCH_QUAD_COUNT
	#define MAX_BATCH_QUAD_COUNT 10
#endif

namespace DOH {

	constexpr static EVertexType QUAD_VERTEX_INPUT_TYPE = EVertexType::VERTEX_3D_TEXTURED_INDEXED;

	//BATCH_QUAD_COUNT is given an arbitrary number, it can be higher or lower depending on need.
	//Different batches may even use different sizes and not stick to this pre-determined limit,
	//which isn't even enforced by the engine.
	//Maybe, for optimisation, the renderer might grant a higher limit to scene batches than UI batches.
	enum EBatchSizeLimits {
		MAX_BATCH_COUNT_QUAD = MAX_BATCH_QUAD_COUNT,

		SINGLE_QUAD_VERTEX_COUNT = 4,
		SINGLE_QUAD_INDEX_COUNT = 6,
		SINGLE_QUAD_COMPONENT_COUNT = getVertexTypeComponentCount(QUAD_VERTEX_INPUT_TYPE) * SINGLE_QUAD_VERTEX_COUNT,
		SINGLE_QUAD_BYTE_SIZE = getVertexTypeSize(QUAD_VERTEX_INPUT_TYPE) * SINGLE_QUAD_VERTEX_COUNT,

		BATCH_MAX_GEO_COUNT_QUAD = 10000,
		BATCH_MAX_COUNT_TEXTURE = 8,

		BATCH_QUAD_VERTEX_COUNT = BATCH_MAX_GEO_COUNT_QUAD * SINGLE_QUAD_VERTEX_COUNT,
		BATCH_QUAD_INDEX_COUNT = BATCH_MAX_GEO_COUNT_QUAD * SINGLE_QUAD_INDEX_COUNT,
		BATCH_QUAD_COMPONENT_COUNT = BATCH_MAX_GEO_COUNT_QUAD * SINGLE_QUAD_COMPONENT_COUNT,
		BATCH_QUAD_BYTE_SIZE = BATCH_MAX_GEO_COUNT_QUAD * SINGLE_QUAD_BYTE_SIZE
	};

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
		inline const std::vector<float>& getData() const { return mData; }
	};
}
