#include "dough/rendering/buffer/BufferLayout.h"

namespace DOH {

	BufferLayout::BufferLayout(const std::initializer_list<BufferElement>& elements)
	:	mBufferElements(elements),
		mStride(0)
	{
		calculateOffsetAndStride();
	}

	void BufferLayout::calculateOffsetAndStride() {
		size_t offset = 0;
		mStride = 0;

		for (BufferElement& element : mBufferElements) {
			element.Offset = offset;
			offset += element.Size;
			mStride += element.Size;
		}
	}
}