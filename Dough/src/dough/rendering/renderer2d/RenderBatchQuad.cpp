#include "dough/rendering/renderer2d/RenderBatchQuad.h"

namespace DOH {

	RenderBatchQuad::RenderBatchQuad(uint32_t maxGeometryCount, uint32_t geoComponentCount, uint32_t maxTextureCount)
	:	ARenderBatch(maxGeometryCount, geoComponentCount, maxTextureCount)
	{}

	void RenderBatchQuad::add(Quad& quad, uint32_t textureSlotIndex) {
		float texCoordsIndex = 0;
		float textureSlot = static_cast<float>(textureSlotIndex);

		addQuadVertex(
			quad.getPosition().x,
			quad.getPosition().y,
			quad.getPosition().z,
			quad.getColour().x,
			quad.getColour().y,
			quad.getColour().z,
			quad.getColour().w,
			//quad.mTextureCoordsU[texCoordsIndex],
			//quad.mTextureCoordsV[texCoordsIndex],
			0.0f,
			1.0f,
			textureSlot
		);
		texCoordsIndex++;

		addQuadVertex(
			quad.getPosition().x + quad.getSize().x,
			quad.getPosition().y,
			quad.getPosition().z,
			quad.getColour().x,
			quad.getColour().y,
			quad.getColour().z,
			quad.getColour().w,
			//quad.mTextureCoordsU[texCoordsIndex],
			//quad.mTextureCoordsV[texCoordsIndex],
			1.0f,
			1.0f,
			textureSlot
		);
		texCoordsIndex++;

		addQuadVertex(
			quad.getPosition().x + quad.getSize().x,
			quad.getPosition().y + quad.getSize().y,
			quad.getPosition().z,
			quad.getColour().x,
			quad.getColour().y,
			quad.getColour().z,
			quad.getColour().w,
			//quad.mTextureCoordsU[texCoordsIndex],
			//quad.mTextureCoordsV[texCoordsIndex],
			1.0f,
			0.0f,
			textureSlot
		);
		texCoordsIndex++;

		addQuadVertex(
			quad.getPosition().x,
			quad.getPosition().y + quad.getSize().y,
			quad.getPosition().z,
			quad.getColour().x,
			quad.getColour().y,
			quad.getColour().z,
			quad.getColour().w,
			//quad.mTextureCoordsU[texCoordsIndex],
			//quad.mTextureCoordsV[texCoordsIndex],
			0.0f,
			0.0f,
			textureSlot
		);

		mGeometryCount++;
	}

	void RenderBatchQuad::addAll(std::vector<Quad> quadArr, uint32_t textureSlotIndex) {
		for (Quad& quad : quadArr) {
			add(quad, textureSlotIndex);
		}
	}

	void RenderBatchQuad::addQuadVertex(
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
	) {
		mData[mDataIndex] = posX;
		mDataIndex++;
		mData[mDataIndex] = posY;
		mDataIndex++;
		mData[mDataIndex] = posZ;
		mDataIndex++;
		mData[mDataIndex] = colourR;
		mDataIndex++;
		mData[mDataIndex] = colourG;
		mDataIndex++;
		mData[mDataIndex] = colourB;
		mDataIndex++;
		mData[mDataIndex] = colourA;
		mDataIndex++;

		mData[mDataIndex] = texCoordU;
		mDataIndex++;
		mData[mDataIndex] = texCoordV;
		mDataIndex++;
		mData[mDataIndex] = texIndex;
		mDataIndex++;
	}
}
