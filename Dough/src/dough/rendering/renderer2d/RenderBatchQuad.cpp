#include "dough/rendering/renderer2d/RenderBatchQuad.h"
#include "dough/rendering/Config.h"

namespace DOH {

	RenderBatchQuad::RenderBatchQuad(const uint32_t maxGeometryCount, const uint32_t maxTextureCount)
	:	ARenderBatch(
			maxGeometryCount,
			Vertex3dTextured::COMPONENT_COUNT * 4 /* Renderer2dVulkan::BatchSizeLimits::SINGLE_QUAD_INDEX_COUNT*/,
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
			//quad.TextureCoordsU[texCoordsIndex],
			//quad.TextureCoordsV[texCoordsIndex],
			0.0f,
			1.0f,
			textureSlot
		);
		texCoordsIndex++;

		addQuadVertex(
			quad.Position.x + quad.Size.x,
			quad.Position.y,
			quad.Position.z,
			quad.Colour.x,
			quad.Colour.y,
			quad.Colour.z,
			quad.Colour.w,
			//quad.TextureCoordsU[texCoordsIndex],
			//quad.TextureCoordsV[texCoordsIndex],
			1.0f,
			1.0f,
			textureSlot
		);
		texCoordsIndex++;

		addQuadVertex(
			quad.Position.x + quad.Size.x,
			quad.Position.y + quad.Size.y,
			quad.Position.z,
			quad.Colour.x,
			quad.Colour.y,
			quad.Colour.z,
			quad.Colour.w,
			//quad.TextureCoordsU[texCoordsIndex],
			//quad.TextureCoordsV[texCoordsIndex],
			1.0f,
			0.0f,
			textureSlot
		);
		texCoordsIndex++;

		addQuadVertex(
			quad.Position.x,
			quad.Position.y + quad.Size.y,
			quad.Position.z,
			quad.Colour.x,
			quad.Colour.y,
			quad.Colour.z,
			quad.Colour.w,
			//quad.TextureCoordsU[texCoordsIndex],
			//quad.TextureCoordsV[texCoordsIndex],
			0.0f,
			0.0f,
			textureSlot
		);

		mGeometryCount++;
	}

	void RenderBatchQuad::addAll(const std::vector<Quad>& quadArr, const uint32_t textureSlotIndex) {
		const float textureSlot = static_cast<float>(textureSlotIndex);

		for (const Quad& quad : quadArr) {
			float texCoordsIndex = 0;

			addQuadVertex(
				quad.Position.x,
				quad.Position.y,
				quad.Position.z,
				quad.Colour.x,
				quad.Colour.y,
				quad.Colour.z,
				quad.Colour.w,
				//quad.TextureCoordsU[texCoordsIndex],
				//quad.TextureCoordsV[texCoordsIndex],
				0.0f,
				1.0f,
				textureSlot
			);
			texCoordsIndex++;

			addQuadVertex(
				quad.Position.x + quad.Size.x,
				quad.Position.y,
				quad.Position.z,
				quad.Colour.x,
				quad.Colour.y,
				quad.Colour.z,
				quad.Colour.w,
				//quad.TextureCoordsU[texCoordsIndex],
				//quad.TextureCoordsV[texCoordsIndex],
				1.0f,
				1.0f,
				textureSlot
			);
			texCoordsIndex++;

			addQuadVertex(
				quad.Position.x + quad.Size.x,
				quad.Position.y + quad.Size.y,
				quad.Position.z,
				quad.Colour.x,
				quad.Colour.y,
				quad.Colour.z,
				quad.Colour.w,
				//quad.TextureCoordsU[texCoordsIndex],
				//quad.TextureCoordsV[texCoordsIndex],
				1.0f,
				0.0f,
				textureSlot
			);
			texCoordsIndex++;

			addQuadVertex(
				quad.Position.x,
				quad.Position.y + quad.Size.y,
				quad.Position.z,
				quad.Colour.x,
				quad.Colour.y,
				quad.Colour.z,
				quad.Colour.w,
				//quad.TextureCoordsU[texCoordsIndex],
				//quad.TextureCoordsV[texCoordsIndex],
				0.0f,
				0.0f,
				textureSlot
			);
		}

		mGeometryCount += static_cast<uint32_t>(quadArr.size());
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
		//mDataIndex++;
		mData[mDataIndex + 1] = posY;
		//mDataIndex++;
		mData[mDataIndex + 2] = posZ;
		//mDataIndex++;
		mData[mDataIndex + 3] = colourR;
		//mDataIndex++;
		mData[mDataIndex + 4] = colourG;
		//mDataIndex++;
		mData[mDataIndex + 5] = colourB;
		//mDataIndex++;
		mData[mDataIndex + 6] = colourA;
		//mDataIndex++;

		mData[mDataIndex + 7] = texCoordU;
		//mDataIndex++;
		mData[mDataIndex + 8] = texCoordV;
		//mDataIndex++;
		mData[mDataIndex + 9] = texIndex;
		//mDataIndex++;

		mDataIndex += 10;
	}
}
