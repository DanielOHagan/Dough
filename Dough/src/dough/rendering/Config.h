#pragma once

#include "dough/Core.h"
#include "dough/Maths.h"

#include <vulkan/vulkan_core.h>

namespace DOH {

	using ValueUniformInfo = VkDeviceSize;
	using TextureUniformInfo = std::pair<VkImageView, VkSampler>;
	using PushConstantInfo = VkPushConstantRange;

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

	static uint32_t getDataTypeSize(EDataType dataType) {
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
	};

	static uint32_t getDataTypeComponentCount(EDataType dataType) {
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
		}

		return 0;
	};

	struct SwapChainSupportDetails {
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	struct QueueFamilyIndices {
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;

		bool isComplete() {
			return graphicsFamily.has_value() && presentFamily.has_value();
		}
	};

	struct SwapChainCreationInfo {
		SwapChainCreationInfo(
			SwapChainSupportDetails supportDetails,
			VkSurfaceKHR surface,
			QueueFamilyIndices& indices
		) :	SupportDetails(supportDetails),
			Surface(surface),
			Indices(indices)
		{}

		SwapChainSupportDetails SupportDetails;
		VkSurfaceKHR Surface;
		QueueFamilyIndices& Indices;

		inline uint32_t getWidth() const { return SupportDetails.capabilities.currentExtent.width; }
		inline void setWidth(uint32_t width) { SupportDetails.capabilities.currentExtent.width = width; }
		inline uint32_t getHeight() const { return SupportDetails.capabilities.currentExtent.height; }
		inline void setHeight(uint32_t height) {SupportDetails.capabilities.currentExtent.height = height;}
	};

	//Scene Vertex3D
	struct Vertex3D {
		glm::vec3 Pos;
		glm::vec3 Colour;
		glm::vec2 TexCoord;
		float TexIndex;

		static VkVertexInputBindingDescription getBindingDescription() {
			VkVertexInputBindingDescription bindDesc = {};

			bindDesc.binding = 0;
			bindDesc.stride = sizeof(Vertex3D);
			bindDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

			return bindDesc;
		}

		static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions() {
			std::vector<VkVertexInputAttributeDescription> attribDesc = {};

			attribDesc.push_back({0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex3D, Pos)});
			attribDesc.push_back({1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex3D, Colour)});
			attribDesc.push_back({2, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex3D, TexCoord)});
			attribDesc.push_back({3, 0, VK_FORMAT_R32_SFLOAT, offsetof(Vertex3D, TexIndex)});

			return attribDesc;
		}
	};

	//Ui Vertex2D
	struct VertexUi2D {
		glm::vec2 Pos;
		glm::vec3 Colour;

		static VkVertexInputBindingDescription getBindingDescription() {
			VkVertexInputBindingDescription bindDesc = {};

			bindDesc.binding = 0;
			bindDesc.stride = sizeof(VertexUi2D);
			bindDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

			return bindDesc;
		}

		static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions() {
			std::vector<VkVertexInputAttributeDescription> attribDesc = {};

			attribDesc.push_back({ 0, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(VertexUi2D, Pos) });
			attribDesc.push_back({ 1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(VertexUi2D, Colour) });

			return attribDesc;
		}
	};
}
