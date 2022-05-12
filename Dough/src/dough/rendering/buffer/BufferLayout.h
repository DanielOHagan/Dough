#pragma once

#include "dough/Core.h"
#include "dough/rendering/buffer/BufferElement.h"

namespace DOH {

	class BufferLayout {

	private:
		std::vector<BufferElement> mBufferElements;
		uint32_t mStride;

	public:
		BufferLayout(const std::initializer_list<BufferElement>& elements);
		BufferLayout(const std::vector<BufferElement>& elements);

		inline std::vector<BufferElement> const getBufferElements() { return mBufferElements; }
		inline uint32_t getStride() const { return mStride; }

	private:
		BufferLayout() = default;
		void calculateOffsetAndStride();
	};
}
