#pragma once

#include <vector>
#include <optional>
#include <array>
#include <string>

#include <vulkan/vulkan_core.h>
#include <glm/glm.hpp>

namespace DOH {

	using TextureDescriptorInfo = std::pair<VkImageView, VkSampler>;

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
			QueueFamilyIndices& indices,
			uint32_t width,
			uint32_t height
		) :	SupportDetails(supportDetails),
			Surface(surface),
			Indices(indices),
			Width(width),
			Height(height)
		{}

		SwapChainSupportDetails SupportDetails;
		VkSurfaceKHR Surface;
		QueueFamilyIndices& Indices;
		uint32_t Width;
		uint32_t Height;
	};

	struct Vertex {
		glm::vec2 Pos;
		glm::vec3 Colour;
		glm::vec2 TexCoord;
		float TexIndex;

		static VkVertexInputBindingDescription getBindingDescription() {
			VkVertexInputBindingDescription bindDesc = {};

			bindDesc.binding = 0;
			bindDesc.stride = sizeof(Vertex);
			bindDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

			return bindDesc;
		}

		static std::array<VkVertexInputAttributeDescription, 4> getAttributeDescriptions() {
			std::array<VkVertexInputAttributeDescription, 4> attribDesc = {};

			attribDesc[0].binding = 0;
			attribDesc[0].location = 0;
			attribDesc[0].format = VK_FORMAT_R32G32_SFLOAT;
			attribDesc[0].offset = offsetof(Vertex, Pos);

			attribDesc[1].binding = 0;
			attribDesc[1].location = 1;
			attribDesc[1].format = VK_FORMAT_R32G32B32_SFLOAT;
			attribDesc[1].offset = offsetof(Vertex, Colour);

			attribDesc[2].binding = 0;
			attribDesc[2].location = 2;
			attribDesc[2].format = VK_FORMAT_R32G32_SFLOAT;
			attribDesc[2].offset = offsetof(Vertex, TexCoord);

			attribDesc[3].binding = 0;
			attribDesc[3].location = 3;
			attribDesc[3].format = VK_FORMAT_R32_SFLOAT;
			attribDesc[3].offset = offsetof(Vertex, TexIndex);

			return attribDesc;
		}
	};
}
