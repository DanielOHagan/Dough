#pragma once

#include "dough/rendering/renderer2d/ARenderBatch.h"
#include "dough/scene/geometry/Quad.h"


namespace DOH {

	class RenderBatchQuad : public ARenderBatch<Quad> {

	public:
		RenderBatchQuad(uint32_t maxGeometryCount, uint32_t geoComponentCount, uint32_t maxTextureCount);

		virtual void add(Quad& geo, uint32_t textureSlotIndex) override;
		virtual void addAll(std::vector<Quad> geoArray, uint32_t textureSlotIndex) override;

	private:
		RenderBatchQuad(const RenderBatchQuad& copy) = delete;
		RenderBatchQuad operator=(const RenderBatchQuad& assignment) = delete;

		void addQuadVertex(
			float posX,
			float posY,
			float posZ,
			float colourR,
			float colourG,
			float colourB,
			float colourA,
			float texCoordU,
			float texCoordV,
			float texIndex
		);
	};
}

