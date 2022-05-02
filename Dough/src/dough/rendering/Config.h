#pragma once

#include "dough/Core.h"
#include "dough/Maths.h"
#include "dough/rendering/buffer/DataType.h"

#include <vulkan/vulkan_core.h>

namespace DOH {

	using ValueUniformInfo = VkDeviceSize; //TODO:: std::pair<VkDeviceSize, VkPipelineStage> to allow for input into different pipeline stages
	using TextureUniformInfo = std::pair<VkImageView, VkSampler>;
	using PushConstantInfo = VkPushConstantRange;
	using DescriptorTypeInfo = std::pair<VkDescriptorType, uint32_t>;

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

	static VkVertexInputBindingDescription createBindingDescription(uint32_t binding, uint32_t stride, VkVertexInputRate inputRate) {
		VkVertexInputBindingDescription bindDesc = {};
		bindDesc.binding = binding;
		bindDesc.stride = stride;
		bindDesc.inputRate = inputRate;
		return bindDesc;
	}

	struct Vertex2d {
		glm::vec2 Pos;
		glm::vec4 Colour;

		static std::vector<VkVertexInputAttributeDescription> asAttributeDescriptions(uint32_t binding) {
			std::vector<VkVertexInputAttributeDescription> attribDesc = {};
			attribDesc.push_back({ 0, binding, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex2d, Pos) });
			attribDesc.push_back({ 1, binding, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(Vertex2d, Colour) });
			return attribDesc;
		}
	};

	struct Vertex3d {
		glm::vec3 Pos;
		glm::vec4 Colour;

		static std::vector<VkVertexInputAttributeDescription> asAttributeDescriptions(uint32_t binding) {
			std::vector<VkVertexInputAttributeDescription> attribDesc = {};
			attribDesc.push_back({ 0, binding, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex3d, Pos) });
			attribDesc.push_back({ 1, binding, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(Vertex3d, Colour) });
			return attribDesc;
		}
	};

	//Expected input for Quad batch rendering
	struct Vertex3dTextured {
		glm::vec3 Pos;
		glm::vec4 Colour;
		glm::vec2 TexCoord;
		float TexIndex;

		constexpr static const uint32_t COMPONENT_COUNT = 10;
		constexpr static const uint32_t BYTE_SIZE = COMPONENT_COUNT * DataType::getDataTypeSize(EDataType::FLOAT);

		static std::vector<VkVertexInputAttributeDescription> asAttributeDescriptions(uint32_t binding) {
			std::vector<VkVertexInputAttributeDescription> attribDesc = {};
			attribDesc.push_back({ 0, binding, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex3dTextured, Pos) });
			attribDesc.push_back({ 1, binding, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(Vertex3dTextured, Colour) });
			attribDesc.push_back({ 2, binding, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex3dTextured, TexCoord) });
			attribDesc.push_back({ 3, binding, VK_FORMAT_R32_SFLOAT, offsetof(Vertex3dTextured, TexIndex) });
			return attribDesc;
		}
	};
}
