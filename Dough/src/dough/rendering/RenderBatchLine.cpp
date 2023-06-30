#include "dough/rendering/RenderBatchLine.h"

#include "dough/Logging.h"

#include <cmath>

namespace DOH {

	RenderBatchLineList::RenderBatchLineList(const EVertexType vertexType, const uint32_t maxLineCount)
	:	mVertexType(vertexType),
		mComponentCount( //Only VERTEX_3D and VERTEX_2D are supported, else set to 0
			vertexType == EVertexType::VERTEX_3D ? LINE_3D_DATA_COMPONENT_COUNT :
				(vertexType == EVertexType::VERTEX_2D ? LINE_2D_DATA_COMPONENT_COUNT : 0)
		),
		mMaxLineCount(mComponentCount > 0 ? (maxLineCount > MAX_LINE_COUNT ? MAX_LINE_COUNT : maxLineCount) : 0), //If given vertexType is not supported set mMaxLineCount to 0
		mData((mMaxLineCount * 2) * getVertexTypeComponentCount(vertexType)),
		mDataIndex(0),
		mLineCount(0)
	{
		if (mComponentCount == 0) {
			LOG_ERR("VertexType given not supported: " << EVertexTypeStrings[static_cast<uint32_t>(vertexType)]);
		}
	}

	void RenderBatchLineList::add2d(const glm::vec2& start, const glm::vec2& end, const glm::vec4& colour) {
		mData[mDataIndex + 0] = start.x;
		mData[mDataIndex + 1] = start.y;
		mData[mDataIndex + 2] = colour.r;
		mData[mDataIndex + 3] = colour.g;
		mData[mDataIndex + 4] = colour.b;
		mData[mDataIndex + 5] = colour.a;
		mData[mDataIndex + 6] = end.x;
		mData[mDataIndex + 7] = end.y;
	
		//TODO:: Use same colour for whole line, allow for start/end colour later
		mData[mDataIndex + 8] = colour.r;
		mData[mDataIndex + 9] = colour.g;
		mData[mDataIndex + 10] = colour.b;
		mData[mDataIndex + 11] = colour.a;
	
		mLineCount++;
		mDataIndex += mComponentCount;
	}

	void RenderBatchLineList::add3d(const glm::vec3& start, const glm::vec3& end, const glm::vec4& colour) {
		mData[mDataIndex + 0] = start.x;
		mData[mDataIndex + 1] = start.y;
		mData[mDataIndex + 2] = start.z;
		mData[mDataIndex + 3] = colour.r;
		mData[mDataIndex + 4] = colour.g;
		mData[mDataIndex + 5] = colour.b;
		mData[mDataIndex + 6] = colour.a;
		mData[mDataIndex + 7] = end.x;
		mData[mDataIndex + 8] = end.y;
		mData[mDataIndex + 9] = end.z;

		//TODO:: Use same colour for whole line, allow for start/end colour later
		mData[mDataIndex + 10] = colour.r;
		mData[mDataIndex + 11] = colour.g;
		mData[mDataIndex + 12] = colour.b;
		mData[mDataIndex + 13] = colour.a;

		mLineCount++;
		mDataIndex += mComponentCount;
	}
}
