#pragma once

#include "dough/rendering/Config.h"

namespace DOH {

	struct BufferElement {

	public:
		EDataType DataType;
		std::string Name;
		size_t Offset;
		uint32_t Size;
		bool Normalised;

	public:
		BufferElement() = default;
		BufferElement(
			EDataType dataType,
			const std::string& name,
			bool normalised = false
		) :	DataType(dataType),
			Name(name),
			Offset(0),
			Size(getDataTypeSize(dataType)),
			Normalised(normalised)
		{};
	};
}
