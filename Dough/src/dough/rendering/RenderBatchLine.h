#pragma once

#include "dough/rendering/Config.h"

namespace DOH {

	static constexpr uint32_t LINE_BATCH_MAX_LINE_COUNT = 10000;

	//For definition of Line List: https://registry.khronos.org/vulkan/specs/1.3-extensions/html/vkspec.html#drawing-line-lists
	//For definition of Line Strip: https://registry.khronos.org/vulkan/specs/1.3-extensions/html/vkspec.html#drawing-line-strips

	//NOTE:: Despite being named similar to ARenderBatch____ this class does not inherit from it
	// since that abstract class and it's childen are designed for triangle list geometry whereas this line batch is for line list.
	//
	//VertexType is stored and dictates whether the RenderBatchLineList instance is for lines in 2D or 3D space
	class RenderBatchLineList {

	private:
		const EVertexType mVertexType;
		const uint32_t mComponentCount;
		const uint32_t mMaxLineCount;

		std::vector<float> mData;
		uint32_t mDataIndex;
		uint32_t mLineCount;

	public:
		static constexpr uint32_t LINE_2D_DATA_COMPONENT_COUNT = 12;
		static constexpr uint32_t LINE_3D_DATA_COMPONENT_COUNT = 14;

		static constexpr uint32_t MAX_LINE_COUNT = UINT32_MAX / 2; //Lines in a Line List consist of 2 vertices each

		//All components are floats
		static constexpr size_t LINE_2D_SIZE = LINE_2D_DATA_COMPONENT_COUNT * 4;
		static constexpr size_t LINE_3D_SIZE = LINE_3D_DATA_COMPONENT_COUNT * 4;

		RenderBatchLineList(const EVertexType vertexType, const uint32_t maxLineCount);

		RenderBatchLineList(const RenderBatchLineList& copy) = delete;
		RenderBatchLineList operator=(const RenderBatchLineList& assignment) = delete;

		void add2d(const glm::vec2& start, const glm::vec2& end, const glm::vec4& colour);
		void add3d(const glm::vec3& start, const glm::vec3& end, const glm::vec4& colour);

		inline const EVertexType getVertexType() const { return mVertexType; }
		inline const uint32_t getMaxLineCount() const { return mMaxLineCount; }
		inline uint32_t getLineCount() const { return mLineCount; }
		inline uint32_t getVertexCount() const { return mLineCount * 2; } //Lines in a Line List consist of 2 vertices each

		inline void reset() {
			mDataIndex = 0;
			mLineCount = 0;
		}
		inline bool hasSpace(const uint32_t count = 1) const { return (mLineCount + count) < mMaxLineCount; }

		inline const std::vector<float>& getData() const { return mData; }
	};


	//class RenderBatchLineStrip {
	//
	//};
}
