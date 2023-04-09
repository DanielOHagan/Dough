#include "dough/rendering/renderer2d/RenderBatchQuad.h"
#include "dough/rendering/Config.h"

namespace DOH {

	std::shared_ptr<StaticVertexInputLayout> RenderBatchQuad::VERTEX_INPUT_LAYOUT = std::make_shared<StaticVertexInputLayout>(RenderBatchQuad::VERTEX_INPUT_TYPE);

	RenderBatchQuad::RenderBatchQuad(const uint32_t maxGeometryCount, const uint32_t maxTextureCount)
	:	ARenderBatch(
			maxGeometryCount,
			getVertexTypeSize(VERTEX_INPUT_TYPE) * 4 /* Renderer2dVulkan::BatchSizeLimits::SINGLE_QUAD_VERTEX_COUNT*/,
			maxTextureCount
		)
	{}

	void RenderBatchQuad::add(const Quad& quad, const uint32_t textureSlotIndex) {
		float texCoordsIndex = 0;
		float textureSlot = static_cast<float>(textureSlotIndex);

		addQuadVertex(
			quad.Position.x,
			quad.Position.y,
			quad.Position.z,
			quad.Colour.x,
			quad.Colour.y,
			quad.Colour.z,
			quad.Colour.w,
			quad.TextureCoords[0],
			quad.TextureCoords[1],
			textureSlot
		);

		addQuadVertex(
			quad.Position.x + quad.Size.x,
			quad.Position.y,
			quad.Position.z,
			quad.Colour.x,
			quad.Colour.y,
			quad.Colour.z,
			quad.Colour.w,
			quad.TextureCoords[2],
			quad.TextureCoords[3],
			textureSlot
		);

		addQuadVertex(
			quad.Position.x + quad.Size.x,
			quad.Position.y + quad.Size.y,
			quad.Position.z,
			quad.Colour.x,
			quad.Colour.y,
			quad.Colour.z,
			quad.Colour.w,
			quad.TextureCoords[4],
			quad.TextureCoords[5],
			textureSlot
		);

		addQuadVertex(
			quad.Position.x,
			quad.Position.y + quad.Size.y,
			quad.Position.z,
			quad.Colour.x,
			quad.Colour.y,
			quad.Colour.z,
			quad.Colour.w,
			quad.TextureCoords[6],
			quad.TextureCoords[7],
			textureSlot
		);

		mGeometryCount++;
	}

	void RenderBatchQuad::addAll(const std::vector<Quad>& quadArr, const uint32_t textureSlotIndex) {
		const float textureSlot = static_cast<float>(textureSlotIndex);

		for (const Quad& quad : quadArr) {
			addQuadVertex(
				quad.Position.x,
				quad.Position.y,
				quad.Position.z,
				quad.Colour.x,
				quad.Colour.y,
				quad.Colour.z,
				quad.Colour.w,
				quad.TextureCoords[0],
				quad.TextureCoords[1],
				textureSlot
			);

			addQuadVertex(
				quad.Position.x + quad.Size.x,
				quad.Position.y,
				quad.Position.z,
				quad.Colour.x,
				quad.Colour.y,
				quad.Colour.z,
				quad.Colour.w,
				quad.TextureCoords[2],
				quad.TextureCoords[3],
				textureSlot
			);

			addQuadVertex(
				quad.Position.x + quad.Size.x,
				quad.Position.y + quad.Size.y,
				quad.Position.z,
				quad.Colour.x,
				quad.Colour.y,
				quad.Colour.z,
				quad.Colour.w,
				quad.TextureCoords[4],
				quad.TextureCoords[5],
				textureSlot
			);

			addQuadVertex(
				quad.Position.x,
				quad.Position.y + quad.Size.y,
				quad.Position.z,
				quad.Colour.x,
				quad.Colour.y,
				quad.Colour.z,
				quad.Colour.w,
				quad.TextureCoords[6],
				quad.TextureCoords[7],
				textureSlot
			);
		}

		mGeometryCount += static_cast<uint32_t>(quadArr.size());
	}

	void RenderBatchQuad::addAll(
		const std::vector<Quad>& quadArr,
		const size_t startIndex,
		const size_t endIndex,
		const uint32_t textureSlotIndex
	) {
		const float textureSlot = static_cast<float>(textureSlotIndex);

		for (size_t i = startIndex; i < endIndex; i++) {

			const Quad& quad = quadArr[i];
			
			addQuadVertex(
				quad.Position.x,
				quad.Position.y,
				quad.Position.z,
				quad.Colour.x,
				quad.Colour.y,
				quad.Colour.z,
				quad.Colour.w,
				quad.TextureCoords[0],
				quad.TextureCoords[1],
				textureSlot
			);

			addQuadVertex(
				quad.Position.x + quad.Size.x,
				quad.Position.y,
				quad.Position.z,
				quad.Colour.x,
				quad.Colour.y,
				quad.Colour.z,
				quad.Colour.w,
				quad.TextureCoords[2],
				quad.TextureCoords[3],
				textureSlot
			);

			addQuadVertex(
				quad.Position.x + quad.Size.x,
				quad.Position.y + quad.Size.y,
				quad.Position.z,
				quad.Colour.x,
				quad.Colour.y,
				quad.Colour.z,
				quad.Colour.w,
				quad.TextureCoords[4],
				quad.TextureCoords[5],
				textureSlot
			);

			addQuadVertex(
				quad.Position.x,
				quad.Position.y + quad.Size.y,
				quad.Position.z,
				quad.Colour.x,
				quad.Colour.y,
				quad.Colour.z,
				quad.Colour.w,
				quad.TextureCoords[6],
				quad.TextureCoords[7],
				textureSlot
			);
		}

		mGeometryCount += static_cast<uint32_t>(endIndex - startIndex);
	}

	void RenderBatchQuad::addQuadVertex(
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
	) {
		mData[mDataIndex + 0] = posX;
		mData[mDataIndex + 1] = posY;
		mData[mDataIndex + 2] = posZ;
		mData[mDataIndex + 3] = colourR;
		mData[mDataIndex + 4] = colourG;
		mData[mDataIndex + 5] = colourB;
		mData[mDataIndex + 6] = colourA;

		mData[mDataIndex + 7] = texCoordU;
		mData[mDataIndex + 8] = texCoordV;
		mData[mDataIndex + 9] = texIndex;

		mDataIndex += 10;
	}
}
