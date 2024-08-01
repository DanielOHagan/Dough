#pragma once

#include "dough/rendering/batches/ARenderBatch.h"
#include "dough/scene/geometry/primitives/Quad.h"
#include "dough/rendering/Config.h"

namespace DOH {

	class RenderBatchQuad : public ARenderBatch<Quad> {
	public:
		RenderBatchQuad(const uint32_t maxGeometryCount, const uint32_t maxTextureCount);

		virtual void add(const Quad& geo, const uint32_t textureSlotIndex) override;
		virtual void addAll(const std::vector<Quad>& geoArray, const uint32_t textureSlotIndex) override;
		void addAll(
			const std::vector<Quad>& quadArr,
			const size_t startIndex,
			const size_t endIndex,
			const uint32_t textureSlotIndex
		);

	private:
		RenderBatchQuad operator=(const RenderBatchQuad& assignment) = delete;

		inline void addQuadVertex(
			const float posX,
			const float posY,
			const float posZ,
			const float colourR,
			const float colourG,
			const float colourB,
			const float colourA,
			const float texCoordU,
			const float texCoordV,
			const float texIndex
		);
	};
}
