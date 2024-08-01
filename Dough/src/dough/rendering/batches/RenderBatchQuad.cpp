#include "dough/rendering/batches/RenderBatchQuad.h"

#include "dough/rendering/Config.h"

namespace DOH {

	RenderBatchQuad::RenderBatchQuad(const uint32_t maxGeometryCount, const uint32_t maxTextureCount)
	:	ARenderBatch(
			maxGeometryCount,
			Quad::BYTE_SIZE,
			maxTextureCount
		)
	{}

	void RenderBatchQuad::add(const Quad& quad, const uint32_t textureSlotIndex) {
		float texCoordsIndex = 0;
		float textureSlot = static_cast<float>(textureSlotIndex);

		//Bot Left
		addQuadVertex(
			quad.Position.x,
			quad.Position.y,
			quad.Position.z,
			quad.Colour.x,
			quad.Colour.y,
			quad.Colour.z,
			quad.Colour.w,
			quad.getTextureCoordsBotLeftX(),
			quad.getTextureCoordsBotLeftY(),
			textureSlot
		);

		//Bot Right
		addQuadVertex(
			quad.Position.x + quad.Size.x,
			quad.Position.y,
			quad.Position.z,
			quad.Colour.x,
			quad.Colour.y,
			quad.Colour.z,
			quad.Colour.w,
			quad.getTextureCoordsTopRightX(),
			quad.getTextureCoordsBotLeftY(),
			textureSlot
		);

		//Top Right
		addQuadVertex(
			quad.Position.x + quad.Size.x,
			quad.Position.y + quad.Size.y,
			quad.Position.z,
			quad.Colour.x,
			quad.Colour.y,
			quad.Colour.z,
			quad.Colour.w,
			quad.getTextureCoordsTopRightX(),
			quad.getTextureCoordsTopRightY(),
			textureSlot
		);

		//Top Left
		addQuadVertex(
			quad.Position.x,
			quad.Position.y + quad.Size.y,
			quad.Position.z,
			quad.Colour.x,
			quad.Colour.y,
			quad.Colour.z,
			quad.Colour.w,
			quad.getTextureCoordsBotLeftX(),
			quad.getTextureCoordsTopRightY(),
			textureSlot
		);

		mGeometryCount++;
	}

	void RenderBatchQuad::addAll(const std::vector<Quad>& quadArr, const uint32_t textureSlotIndex) {
		const float textureSlot = static_cast<float>(textureSlotIndex);

		for (const Quad& quad : quadArr) {
			//Bot Left
			addQuadVertex(
				quad.Position.x,
				quad.Position.y,
				quad.Position.z,
				quad.Colour.x,
				quad.Colour.y,
				quad.Colour.z,
				quad.Colour.w,
				quad.getTextureCoordsBotLeftX(),
				quad.getTextureCoordsBotLeftY(),
				textureSlot
			);

			//Bot Right
			addQuadVertex(
				quad.Position.x + quad.Size.x,
				quad.Position.y,
				quad.Position.z,
				quad.Colour.x,
				quad.Colour.y,
				quad.Colour.z,
				quad.Colour.w,
				quad.getTextureCoordsTopRightX(),
				quad.getTextureCoordsBotLeftY(),
				textureSlot
			);

			//Top Right
			addQuadVertex(
				quad.Position.x + quad.Size.x,
				quad.Position.y + quad.Size.y,
				quad.Position.z,
				quad.Colour.x,
				quad.Colour.y,
				quad.Colour.z,
				quad.Colour.w,
				quad.getTextureCoordsTopRightX(),
				quad.getTextureCoordsTopRightY(),
				textureSlot
			);

			//Top Left
			addQuadVertex(
				quad.Position.x,
				quad.Position.y + quad.Size.y,
				quad.Position.z,
				quad.Colour.x,
				quad.Colour.y,
				quad.Colour.z,
				quad.Colour.w,
				quad.getTextureCoordsBotLeftX(),
				quad.getTextureCoordsTopRightY(),
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
			//Bot Left
			addQuadVertex(
				quad.Position.x,
				quad.Position.y,
				quad.Position.z,
				quad.Colour.x,
				quad.Colour.y,
				quad.Colour.z,
				quad.Colour.w,
				quad.getTextureCoordsBotLeftX(),
				quad.getTextureCoordsBotLeftY(),
				textureSlot
			);

			//Bot Right
			addQuadVertex(
				quad.Position.x + quad.Size.x,
				quad.Position.y,
				quad.Position.z,
				quad.Colour.x,
				quad.Colour.y,
				quad.Colour.z,
				quad.Colour.w,
				quad.getTextureCoordsTopRightX(),
				quad.getTextureCoordsBotLeftY(),
				textureSlot
			);

			//Top Right
			addQuadVertex(
				quad.Position.x + quad.Size.x,
				quad.Position.y + quad.Size.y,
				quad.Position.z,
				quad.Colour.x,
				quad.Colour.y,
				quad.Colour.z,
				quad.Colour.w,
				quad.getTextureCoordsTopRightX(),
				quad.getTextureCoordsTopRightY(),
				textureSlot
			);

			//Top Left
			addQuadVertex(
				quad.Position.x,
				quad.Position.y + quad.Size.y,
				quad.Position.z,
				quad.Colour.x,
				quad.Colour.y,
				quad.Colour.z,
				quad.Colour.w,
				quad.getTextureCoordsBotLeftX(),
				quad.getTextureCoordsTopRightY(),
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
