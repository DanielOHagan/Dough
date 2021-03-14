#pragma once

#include <vector>
#include <optional>
#include <array>

#include <vulkan/vulkan_core.h>
#include <glm/glm.hpp>

namespace DOH {

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

	struct Vertex {
		glm::vec2 Pos;
		glm::vec3 Colour;

		static VkVertexInputBindingDescription getBindingDescription() {
			VkVertexInputBindingDescription bindDesc = {};

			bindDesc.binding = 0;
			bindDesc.stride = sizeof(Vertex);
			bindDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

			return bindDesc;
		}

		static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions() {
			std::array<VkVertexInputAttributeDescription, 2> attribDesc = {};

			attribDesc[0].binding = 0;
			attribDesc[0].location = 0;
			attribDesc[0].format = VK_FORMAT_R32G32_SFLOAT;
			attribDesc[0].offset = offsetof(Vertex, Pos);

			attribDesc[1].binding = 0;
			attribDesc[1].location = 1;
			attribDesc[1].format = VK_FORMAT_R32G32B32_SFLOAT;
			attribDesc[1].offset = offsetof(Vertex, Colour);

			return attribDesc;
		}
	};
}