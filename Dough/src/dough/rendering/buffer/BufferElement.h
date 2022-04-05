#pragma once

#include "dough/rendering/buffer/DataType.h"

namespace DOH {

	struct BufferElement {

	public:
		EDataType DataType;
		size_t Offset;
		uint32_t Size;
		bool Normalised;

	public:
		BufferElement() = default;
		BufferElement(EDataType dataType, bool normalised = false)
		:	DataType(dataType),
			Offset(0),
			Size(DataType::getDataTypeSize(dataType)),
			Normalised(normalised)
		{};
	};
}
