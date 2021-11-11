#pragma once

#include "dough/rendering/buffer/BufferElement.h"

namespace DOH {

	class BufferLayout {

	private:
		std::vector<BufferElement> mBufferElements;
		uint32_t mStride;

	public:
		BufferLayout() = delete;
		BufferLayout(const BufferLayout& copy) = delete;
		BufferLayout operator=(const BufferLayout& assignment) = delete;

		BufferLayout(const std::initializer_list<BufferElement>& elements);

		inline std::vector<BufferElement> const getBufferElements() { return mBufferElements; }
		inline uint32_t getStride() const { return mStride; }

	private:
		void calculateOffsetAndStride();
	};
}
