#include "dough/rendering/batches/RenderBatchCircle.h"

#include "dough/rendering/Config.h"

namespace DOH {

	RenderBatchCircle::RenderBatchCircle(const uint32_t maxGeometryCount, const uint32_t maxTextureCount)
	: ARenderBatch(
		maxGeometryCount,
		Circle::BYTE_SIZE,
		maxTextureCount
	) {}

	void RenderBatchCircle::add(const Circle& circle, const uint32_t textureSlotIndex) {
		float texCoordsIndex = 0;
		float textureSlot = static_cast<float>(textureSlotIndex);

		//Bot Left
		addCircleVertex(
			circle.Position.x,
			circle.Position.y,
			circle.Position.z,
			circle.Colour.x,
			circle.Colour.y,
			circle.Colour.z,
			circle.Colour.w,
			circle.getTextureCoordsBotLeftX(),
			circle.getTextureCoordsBotLeftY(),
			-1.0f,
			-1.0f,
			circle.getThickness(),
			circle.getFade(),
			textureSlot
		);

		//Bot Right
		addCircleVertex(
			circle.Position.x + circle.Size.x,
			circle.Position.y,
			circle.Position.z,
			circle.Colour.x,
			circle.Colour.y,
			circle.Colour.z,
			circle.Colour.w,
			circle.getTextureCoordsTopRightX(),
			circle.getTextureCoordsBotLeftY(),
			1.0f,
			-1.0f,
			circle.getThickness(),
			circle.getFade(),
			textureSlot
		);

		//Top Right
		addCircleVertex(
			circle.Position.x + circle.Size.x,
			circle.Position.y + circle.Size.y,
			circle.Position.z,
			circle.Colour.x,
			circle.Colour.y,
			circle.Colour.z,
			circle.Colour.w,
			circle.getTextureCoordsTopRightX(),
			circle.getTextureCoordsTopRightY(),
			1.0f,
			1.0f,
			circle.getThickness(),
			circle.getFade(),
			textureSlot
		);

		//Top Left
		addCircleVertex(
			circle.Position.x,
			circle.Position.y + circle.Size.y,
			circle.Position.z,
			circle.Colour.x,
			circle.Colour.y,
			circle.Colour.z,
			circle.Colour.w,
			circle.getTextureCoordsBotLeftX(),
			circle.getTextureCoordsTopRightY(),
			-1.0f,
			1.0f,
			circle.getThickness(),
			circle.getFade(),
			textureSlot
		);

		mGeometryCount++;
	}

	void RenderBatchCircle::addAll(const std::vector<Circle>& circleArr, const uint32_t textureSlotIndex) {
		const float textureSlot = static_cast<float>(textureSlotIndex);

		for (const Circle& circle : circleArr) {
			//Bot Left
			addCircleVertex(
				circle.Position.x,
				circle.Position.y,
				circle.Position.z,
				circle.Colour.x,
				circle.Colour.y,
				circle.Colour.z,
				circle.Colour.w,
				circle.getTextureCoordsBotLeftX(),
				circle.getTextureCoordsBotLeftY(),
				-1.0f,
				-1.0f,
				circle.getThickness(),
				circle.getFade(),
				textureSlot
			);

			//Bot Right
			addCircleVertex(
				circle.Position.x + circle.Size.x,
				circle.Position.y,
				circle.Position.z,
				circle.Colour.x,
				circle.Colour.y,
				circle.Colour.z,
				circle.Colour.w,
				circle.getTextureCoordsTopRightX(),
				circle.getTextureCoordsBotLeftY(),
				1.0f,
				-1.0f,
				circle.getThickness(),
				circle.getFade(),
				textureSlot
			);

			//Top Right
			addCircleVertex(
				circle.Position.x + circle.Size.x,
				circle.Position.y + circle.Size.y,
				circle.Position.z,
				circle.Colour.x,
				circle.Colour.y,
				circle.Colour.z,
				circle.Colour.w,
				circle.getTextureCoordsTopRightX(),
				circle.getTextureCoordsTopRightY(),
				1.0f,
				1.0f,
				circle.getThickness(),
				circle.getFade(),
				textureSlot
			);

			//Top Left
			addCircleVertex(
				circle.Position.x,
				circle.Position.y + circle.Size.y,
				circle.Position.z,
				circle.Colour.x,
				circle.Colour.y,
				circle.Colour.z,
				circle.Colour.w,
				circle.getTextureCoordsBotLeftX(),
				circle.getTextureCoordsTopRightY(),
				-1.0f,
				1.0f,
				circle.getThickness(),
				circle.getFade(),
				textureSlot
			);
		}

		mGeometryCount += static_cast<uint32_t>(circleArr.size());
	}

	void RenderBatchCircle::addAll(
		const std::vector<Circle>& circleArr,
		const size_t startIndex,
		const size_t endIndex,
		const uint32_t textureSlotIndex
	) {
		const float textureSlot = static_cast<float>(textureSlotIndex);

		for (size_t i = startIndex; i < endIndex; i++) {
			const Circle& circle = circleArr[i];

			//Bot Left
			addCircleVertex(
				circle.Position.x,
				circle.Position.y,
				circle.Position.z,
				circle.Colour.x,
				circle.Colour.y,
				circle.Colour.z,
				circle.Colour.w,
				circle.getTextureCoordsBotLeftX(),
				circle.getTextureCoordsBotLeftY(),
				-1.0f,
				-1.0f,
				circle.getThickness(),
				circle.getFade(),
				textureSlot
			);

			//Bot Right
			addCircleVertex(
				circle.Position.x + circle.Size.x,
				circle.Position.y,
				circle.Position.z,
				circle.Colour.x,
				circle.Colour.y,
				circle.Colour.z,
				circle.Colour.w,
				circle.getTextureCoordsTopRightX(),
				circle.getTextureCoordsBotLeftY(),
				1.0f,
				-1.0f,
				circle.getThickness(),
				circle.getFade(),
				textureSlot
			);

			//Top Right
			addCircleVertex(
				circle.Position.x + circle.Size.x,
				circle.Position.y + circle.Size.y,
				circle.Position.z,
				circle.Colour.x,
				circle.Colour.y,
				circle.Colour.z,
				circle.Colour.w,
				circle.getTextureCoordsTopRightX(),
				circle.getTextureCoordsTopRightY(),
				1.0f,
				1.0f,
				circle.getThickness(),
				circle.getFade(),
				textureSlot
			);

			//Top Left
			addCircleVertex(
				circle.Position.x,
				circle.Position.y + circle.Size.y,
				circle.Position.z,
				circle.Colour.x,
				circle.Colour.y,
				circle.Colour.z,
				circle.Colour.w,
				circle.getTextureCoordsBotLeftX(),
				circle.getTextureCoordsTopRightY(),
				-1.0f,
				1.0f,
				circle.getThickness(),
				circle.getFade(),
				textureSlot
			);
		}

		mGeometryCount += static_cast<uint32_t>(endIndex - startIndex);
	}

	void RenderBatchCircle::addCircleVertex(
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
		mData[mDataIndex + 9] = quadBoundX;
		mData[mDataIndex + 10] = quadBoundY;
		mData[mDataIndex + 11] = thickness;
		mData[mDataIndex + 12] = fade;
		mData[mDataIndex + 13] = texIndex;

		mDataIndex += VertexCircle3d::COMPONENT_COUNT;
	}
}
