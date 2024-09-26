#pragma once

#include "dough/rendering/batches/ARenderBatch.h"
#include "dough/scene/geometry/primitives/Circle.h"
#include "dough/rendering/Config.h"

namespace DOH {

	class RenderBatchCircle : public ARenderBatch<Circle> {
	public:
		RenderBatchCircle(const uint32_t maxGeometryCount, const uint32_t maxTextureCount);

		virtual void add(const Circle& geo, const uint32_t textureSlotIndex) override;
		virtual void addAll(const std::vector<Circle>& geoArr, const uint32_t textureSlotIndex) override;
		void addAll(
			const std::vector<Circle>& geoArr,
			const size_t startIndex,
			const size_t endIndex,
			const uint32_t textureSlotIndex
		);

	private:
		RenderBatchCircle operator=(const RenderBatchCircle& assignment) = delete;

		inline void addCircleVertex(
			const float posX,
			const float posY,
			const float posZ,
			const float colourR,
			const float colourG,
			const float colourB,
			const float colourA,
			const float texCoordU,
			const float texCoordV,
			const float quadBoundX,
			const float quadBoundY,
			const float thickness,
			const float fade,
			const float texIndex
		);
	};
}
