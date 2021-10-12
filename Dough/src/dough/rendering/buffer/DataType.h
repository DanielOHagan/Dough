#pragma once

#include <cstdint>

namespace DOH {

	enum class EDataType {
		NONE = 0,

		FLOAT,
		FLOAT2,
		FLOAT3,
		FLOAT4,

		MAT3,
		MAT4,

		INT,
		INT2,
		INT3,
		INT4,

		BOOL
	};

	class DataType {

	public:
		static uint32_t getComponentCount(EDataType dataType) {
			switch (dataType) {
				case EDataType::FLOAT: return 1;
				case EDataType::FLOAT2: return 2;
				case EDataType::FLOAT3: return 3;
				case EDataType::FLOAT4: return 4;

				case EDataType::MAT3: return 3 * 3;
				case EDataType::MAT4: return 4 * 4;

				case EDataType::INT: return 1;
				case EDataType::INT2: return 2;
				case EDataType::INT3: return 3;
				case EDataType::INT4: return 4;

				case EDataType::BOOL: return 1;

				default: return 0;
			}
		}

		static size_t getDataTypeSize(EDataType dataType) {
			switch (dataType) {
				case EDataType::FLOAT: return 4;
				case EDataType::FLOAT2: return 4 * 2;
				case EDataType::FLOAT3: return 4 * 3;
				case EDataType::FLOAT4: return 4 * 4;

				case EDataType::MAT3: return 4 * 3 * 3;
				case EDataType::MAT4: return 4 * 4 * 4;

				case EDataType::INT: return 4;
				case EDataType::INT2: return 4 * 2;
				case EDataType::INT3: return 4 * 3;
				case EDataType::INT4: return 4 * 4;

				case EDataType::BOOL: return 1;
			}

			return 0;
		}
	};
}
