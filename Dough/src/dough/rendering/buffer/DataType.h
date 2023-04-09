#pragma once

#include <cstdint>

#include <vulkan/vulkan_core.h>

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

		UINT,
		UINT2,
		UINT3,
		UINT4,

		BOOL
	};

	static std::array<const char*, 16> EDataTypeStrings = {
		"NONE",

		"FLOAT",
		"FLOAT2",
		"FLOAT3",
		"FLOAT4",

		"MAT3",
		"MAT4",

		"INT",
		"INT2",
		"INT3",
		"INT4",

		"UINT",
		"UINT2",
		"UINT3",
		"UINT4",

		"BOOL"
	};

	class DataType {
	public:
		constexpr static uint32_t getComponentCount(EDataType dataType) {
			switch (dataType) {
				case EDataType::FLOAT:	return 1;
				case EDataType::FLOAT2:	return 2;
				case EDataType::FLOAT3:	return 3;
				case EDataType::FLOAT4:	return 4;

				case EDataType::MAT3:	return 3 * 3;
				case EDataType::MAT4:	return 4 * 4;

				case EDataType::INT:
				case EDataType::UINT:
					return 1;
				case EDataType::INT2:
				case EDataType::UINT2:
					return 2;
				case EDataType::INT3:
				case EDataType::UINT3:
					return 3;
				case EDataType::INT4:
				case EDataType::UINT4:
					return 4;

				case EDataType::BOOL:	return 1;

				default: return 0;
			}
		}

		constexpr static uint32_t getDataTypeSize(EDataType dataType) {
			switch (dataType) {
				case EDataType::FLOAT:	return 4;
				case EDataType::FLOAT2:	return getDataTypeSize(EDataType::FLOAT) * 2;
				case EDataType::FLOAT3:	return getDataTypeSize(EDataType::FLOAT) * 3;
				case EDataType::FLOAT4:	return getDataTypeSize(EDataType::FLOAT) * 4;

				case EDataType::MAT3:	return getDataTypeSize(EDataType::FLOAT) * 3 * 3;
				case EDataType::MAT4:	return getDataTypeSize(EDataType::FLOAT) * 4 * 4;

				case EDataType::INT:
				case EDataType::UINT:
					return 4;
				case EDataType::INT2:
				case EDataType::UINT2:
					return getDataTypeSize(EDataType::INT) * 2;
				case EDataType::INT3:
				case EDataType::UINT3:
					return getDataTypeSize(EDataType::INT) * 3;
				case EDataType::INT4:
				case EDataType::UINT4:
					return getDataTypeSize(EDataType::INT) * 4;

				case EDataType::BOOL:	return 1;

				default: return 0;
			}

			return 0;
		}

		constexpr static VkFormat getDataTypeFormat(EDataType dataType) {
			switch (dataType) {
				case EDataType::FLOAT:	return VK_FORMAT_R32_SFLOAT;
				case EDataType::FLOAT2:	return VK_FORMAT_R32G32_SFLOAT;
				case EDataType::FLOAT3:	return VK_FORMAT_R32G32B32_SFLOAT;
				case EDataType::FLOAT4:	return VK_FORMAT_R32G32B32A32_SFLOAT;

					//case EDataType::MAT3:
					//case EDataType::MAT4:

				case EDataType::INT:	return VK_FORMAT_R32_SINT;
				case EDataType::INT2:	return VK_FORMAT_R32G32_SINT;
				case EDataType::INT3:	return VK_FORMAT_R32G32B32_SINT;
				case EDataType::INT4:	return VK_FORMAT_R32G32B32A32_SINT;

				case EDataType::UINT:	return VK_FORMAT_R32_UINT;
				case EDataType::UINT2:	return VK_FORMAT_R32G32_UINT;
				case EDataType::UINT3:	return VK_FORMAT_R32G32B32_UINT;
				case EDataType::UINT4:	return VK_FORMAT_R32G32B32A32_UINT;

					//case EDataType::BOOL:

				default:
					return VK_FORMAT_UNDEFINED;
			}
		}
	};
}
