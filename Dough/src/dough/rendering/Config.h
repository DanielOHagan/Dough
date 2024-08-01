#pragma once

#include "dough/Core.h"
#include "dough/Maths.h"
#include "dough/rendering/buffer/DataType.h"

#include <vulkan/vulkan_core.h>

namespace DOH {

	using ValueUniformInfo = VkDeviceSize; //TODO:: std::pair<VkDeviceSize, VkPipelineStage> to allow for input into different pipeline stages
	using TextureUniformInfo = std::pair<VkImageView, VkSampler>;
	using DescriptorTypeInfo = std::pair<VkDescriptorType, uint32_t>;

	struct SwapChainSupportDetails {
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	struct QueueFamilyIndices {
		std::optional<uint32_t> GraphicsFamily;
		std::optional<uint32_t> PresentFamily;

		bool isComplete() {
			return GraphicsFamily.has_value() && PresentFamily.has_value();
		}
	};

	struct SwapChainCreationInfo {
		SwapChainSupportDetails SupportDetails;
		VkSurfaceKHR Surface;
		QueueFamilyIndices& Indices;
		VkPresentModeKHR DesiredPresentMode;
		bool FallbackToImmediatePresentMode;

		SwapChainCreationInfo(
			SwapChainSupportDetails supportDetails,
			VkSurfaceKHR surface,
			QueueFamilyIndices& indices,
			VkPresentModeKHR desiredPresentMode,
			bool fallBackToImmediatePresentMode
		) : SupportDetails(supportDetails),
			Surface(surface),
			Indices(indices),
			DesiredPresentMode(desiredPresentMode),
			FallbackToImmediatePresentMode(fallBackToImmediatePresentMode)
		{}

		inline uint32_t getWidth() const { return SupportDetails.capabilities.currentExtent.width; }
		inline void setWidth(uint32_t width) { SupportDetails.capabilities.currentExtent.width = width; }
		inline uint32_t getHeight() const { return SupportDetails.capabilities.currentExtent.height; }
		inline void setHeight(uint32_t height) { SupportDetails.capabilities.currentExtent.height = height; }
	};

	//TODO:: better system for this?
	//e.g. Use a bunch of pre-determined sets of structs that are used to store data.
	// Vulkan doesn't require input data to be specifically named so any spare unused data in vertex can be used as padding/saved for any changes later.
	// VERTEX_2D/3D standard, then anything with _TEXTURED_ includes tex coords and texArr index, assumes texture arrays are used all throughout

	constexpr static std::array<const char*, 9> EVertexTypeStrings = {
		"NONE",

		"VERTEX_2D",
		"VERTEX_2D_TEXTURED",
		"VERTEX_2D_TEXTURED_INDEXED",
		"VERTEX_3D",
		"VERTEX_3D_TEXTURED",
		"VERTEX_3D_TEXTURED_INDEXED",
		"VERTEX_3D_LIT_TEXTURED",

		"VERTEX_CIRCLE_3D"
	};

	enum class EVertexType {
		/**
		* Default value so it should never be used as vertex input.
		*/
		NONE = 0,

		/**
		* vec2 float Pos
		* vec4 float Colour
		*/
		VERTEX_2D,

		/**
		* vec2 float Pos
		* vec4 float Colour
		* vec2 float TexCoord
		*/
		VERTEX_2D_TEXTURED,

		/**
		* vec2 float Pos
		* vec4 float Colour
		* vec2 float TexCoord
		* 1 float TexIndex
		*/
		VERTEX_2D_TEXTURED_INDEXED,

		/**
		* vec3 float Pos
		* vec4 float Colour
		*/
		VERTEX_3D,

		/**
		* vec3 float Pos
		* vec4 float Colour
		* vec2 float TexCoord
		*/
		VERTEX_3D_TEXTURED,

		/**
		* vec3 float Pos
		* vec4 float Colour
		* vec2 float TexCoord
		* 1 float TexIndex
		*/
		VERTEX_3D_TEXTURED_INDEXED,

		/**
		* vec3 float Pos
		* vec4 float Colour
		* vec3 float Normal
		* vec2 float TexCoord
		*/
		VERTEX_3D_LIT_TEXTURED,



		//TODO:: Should there be a separate enum for context specific vertex input layouts?
		/**
		* vec3 float Pos
		* vec4 float Colour
		* vec2 float TexCoord
		* vec2 float QuadBound
		* vec2 float Decorations
		*	x Thickness
		*	y Fade IMPORTANT:: Fade value MUST be non-zero. There is a max(Fade, 0.0001) in the shader.
		* 1 float TexIndex
		*/
		VERTEX_CIRCLE_3D

		//TODO:: VERTEX_CIRCLE_3D_DECORATED ? and turn VERTEX_CIRCLE_3D into VERTEX_CIRCLE_3D_SIMPLE
	};

	struct Vertex2d {
		glm::vec2 Pos;
		glm::vec4 Colour;

		constexpr static const uint32_t COMPONENT_COUNT = 6;
		constexpr static const uint32_t BYTE_SIZE = COMPONENT_COUNT * DataType::getDataTypeSize(EDataType::FLOAT);
		constexpr static const EVertexType ENUM = EVertexType::VERTEX_2D;

		static std::vector<VkVertexInputAttributeDescription> asAttributeDescriptions(uint32_t binding) {
			return {
				{ 0, binding, VK_FORMAT_R32G32_SFLOAT		, offsetof(Vertex2d, Pos) },
				{ 1, binding, VK_FORMAT_R32G32B32A32_SFLOAT	, offsetof(Vertex2d, Colour) }
			};
		}

		bool operator==(const Vertex2d& other) const {
			return Pos == other.Pos && Colour == other.Colour;
		}
	};

	struct Vertex2dTextured {
		glm::vec2 Pos;
		glm::vec4 Colour;
		glm::vec2 TexCoords;

		constexpr static const uint32_t COMPONENT_COUNT = 8;
		constexpr static const uint32_t BYTE_SIZE = COMPONENT_COUNT * DataType::getDataTypeSize(EDataType::FLOAT);
		constexpr static const EVertexType ENUM = EVertexType::VERTEX_2D_TEXTURED;

		static std::vector<VkVertexInputAttributeDescription> asAttributeDescriptions(uint32_t binding) {
			return {
				{ 0, binding, VK_FORMAT_R32G32_SFLOAT		, offsetof(Vertex2dTextured, Pos) },
				{ 1, binding, VK_FORMAT_R32G32B32A32_SFLOAT	, offsetof(Vertex2dTextured, Colour) },
				{ 2, binding, VK_FORMAT_R32G32_SFLOAT		, offsetof(Vertex2dTextured, TexCoords) }
			};
		}

		bool operator==(const Vertex2dTextured& other) const {
			return Pos == other.Pos && Colour == other.Colour && TexCoords == other.TexCoords;
		}
	};

	struct Vertex2dTexturedIndexed {
		glm::vec2 Pos;
		glm::vec4 Colour;
		glm::vec2 TexCoords;
		float TexIndex;

		constexpr static const uint32_t COMPONENT_COUNT = 9;
		constexpr static const uint32_t BYTE_SIZE = COMPONENT_COUNT * DataType::getDataTypeSize(EDataType::FLOAT);
		constexpr static const EVertexType ENUM = EVertexType::VERTEX_2D_TEXTURED_INDEXED;

		static std::vector<VkVertexInputAttributeDescription> asAttributeDescriptions(uint32_t binding) {
			return {
				{ 0, binding, VK_FORMAT_R32G32_SFLOAT		, offsetof(Vertex2dTexturedIndexed, Pos) },
				{ 1, binding, VK_FORMAT_R32G32B32A32_SFLOAT	, offsetof(Vertex2dTexturedIndexed, Colour) },
				{ 2, binding, VK_FORMAT_R32G32_SFLOAT		, offsetof(Vertex2dTexturedIndexed, TexCoords) },
				{ 3, binding, VK_FORMAT_R32_SFLOAT			, offsetof(Vertex2dTexturedIndexed, TexIndex) }
			};
		}

		bool operator==(const Vertex2dTexturedIndexed& other) const {
			return Pos == other.Pos && Colour == other.Colour && TexCoords == other.TexCoords && TexIndex == other.TexIndex;
		}
	};

	struct Vertex3d {
		glm::vec3 Pos;
		glm::vec4 Colour;

		constexpr static const uint32_t COMPONENT_COUNT = 7;
		constexpr static const uint32_t BYTE_SIZE = COMPONENT_COUNT * DataType::getDataTypeSize(EDataType::FLOAT);
		constexpr static const EVertexType ENUM = EVertexType::VERTEX_3D;

		static std::vector<VkVertexInputAttributeDescription> asAttributeDescriptions(uint32_t binding) {
			return {
				{ 0, binding, VK_FORMAT_R32G32B32_SFLOAT	, offsetof(Vertex3d, Pos) },
				{ 1, binding, VK_FORMAT_R32G32B32A32_SFLOAT	, offsetof(Vertex3d, Colour) }
			};
		}

		bool operator==(const Vertex3d& other) const {
			return Pos == other.Pos && Colour == other.Colour;
		}
	};

	struct Vertex3dTextured {
		glm::vec3 Pos;
		glm::vec4 Colour;
		glm::vec2 TexCoord;

		constexpr static const uint32_t COMPONENT_COUNT = 9;
		constexpr static const uint32_t BYTE_SIZE = COMPONENT_COUNT * DataType::getDataTypeSize(EDataType::FLOAT);
		constexpr static const EVertexType ENUM = EVertexType::VERTEX_3D_TEXTURED;

		static std::vector<VkVertexInputAttributeDescription> asAttributeDescriptions(uint32_t binding) {
			return {
				{ 0, binding, VK_FORMAT_R32G32B32_SFLOAT	, offsetof(Vertex3dTextured, Pos) },
				{ 1, binding, VK_FORMAT_R32G32B32A32_SFLOAT	, offsetof(Vertex3dTextured, Colour) },
				{ 2, binding, VK_FORMAT_R32G32_SFLOAT		, offsetof(Vertex3dTextured, TexCoord) }
			};
		}

		bool operator==(const Vertex3dTextured& other) const {
			return Pos == other.Pos && Colour == other.Colour && TexCoord == other.TexCoord; //&& TexIndex == other.TexIndex;
		}
	};

	//Expected input for Quad batch rendering
	struct Vertex3dTexturedIndexed {
		glm::vec3 Pos;
		glm::vec4 Colour;
		glm::vec2 TexCoord;
		float TexIndex;

		constexpr static const uint32_t COMPONENT_COUNT = 10;
		constexpr static const uint32_t BYTE_SIZE = COMPONENT_COUNT * DataType::getDataTypeSize(EDataType::FLOAT);
		constexpr static const EVertexType ENUM = EVertexType::VERTEX_3D_TEXTURED_INDEXED;

		static std::vector<VkVertexInputAttributeDescription> asAttributeDescriptions(uint32_t binding) {
			return {
				{ 0, binding, VK_FORMAT_R32G32B32_SFLOAT	, offsetof(Vertex3dTexturedIndexed, Pos) },
				{ 1, binding, VK_FORMAT_R32G32B32A32_SFLOAT	, offsetof(Vertex3dTexturedIndexed, Colour) },
				{ 2, binding, VK_FORMAT_R32G32_SFLOAT		, offsetof(Vertex3dTexturedIndexed, TexCoord) },
				{ 3, binding, VK_FORMAT_R32_SFLOAT			, offsetof(Vertex3dTexturedIndexed, TexIndex) },
			};
		}

		bool operator==(const Vertex3dTexturedIndexed& other) const {
			return Pos == other.Pos && Colour == other.Colour && TexCoord == other.TexCoord && TexIndex == other.TexIndex;
		}
	};

	struct Vertex3dLitTextured {
		glm::vec3 Pos;
		glm::vec4 Colour;
		glm::vec3 Normal;
		glm::vec2 TexCoord;

		constexpr static const uint32_t COMPONENT_COUNT = 12;
		constexpr static const uint32_t BYTE_SIZE = COMPONENT_COUNT * DataType::getDataTypeSize(EDataType::FLOAT);
		constexpr static const EVertexType ENUM = EVertexType::VERTEX_3D_LIT_TEXTURED;

		static std::vector<VkVertexInputAttributeDescription> asAttributeDescriptions(uint32_t binding) {
			return {
				{ 0, binding, VK_FORMAT_R32G32B32_SFLOAT	, offsetof(Vertex3dLitTextured, Pos) },
				{ 1, binding, VK_FORMAT_R32G32B32A32_SFLOAT	, offsetof(Vertex3dLitTextured, Colour) },
				{ 2, binding, VK_FORMAT_R32G32B32A32_SFLOAT	, offsetof(Vertex3dLitTextured, Normal) },
				{ 3, binding, VK_FORMAT_R32G32_SFLOAT		, offsetof(Vertex3dLitTextured, TexCoord) }
			};
		}

		bool operator==(const Vertex3dLitTextured& other) const {
			return Pos == other.Pos && Colour == other.Colour && Normal == other.Normal && TexCoord == other.TexCoord;
		}
	};

	struct VertexCircle3d {
		glm::vec3 Pos;
		glm::vec4 Colour;
		glm::vec2 TexCoord;
		glm::vec2 QuadBound;
		glm::vec2 Decorations;
		float TexIndex;

		constexpr static const uint32_t COMPONENT_COUNT = 14;
		constexpr static const uint32_t BYTE_SIZE = COMPONENT_COUNT * DataType::getDataTypeSize(EDataType::FLOAT);
		constexpr static const EVertexType ENUM = EVertexType::VERTEX_CIRCLE_3D;

		static std::vector<VkVertexInputAttributeDescription> asAttributeDescriptions(uint32_t binding) {
			return {
				{ 0, binding, VK_FORMAT_R32G32B32_SFLOAT	, offsetof(VertexCircle3d, Pos) },
				{ 1, binding, VK_FORMAT_R32G32B32A32_SFLOAT	, offsetof(VertexCircle3d, Colour) },
				{ 2, binding, VK_FORMAT_R32G32_SFLOAT		, offsetof(VertexCircle3d, TexCoord) },
				{ 3, binding, VK_FORMAT_R32G32_SFLOAT		, offsetof(VertexCircle3d, QuadBound) },
				{ 4, binding, VK_FORMAT_R32G32_SFLOAT		, offsetof(VertexCircle3d, Decorations) },
				{ 5, binding, VK_FORMAT_R32_SFLOAT			, offsetof(VertexCircle3d, TexIndex) }
			};
		}

		bool operator==(const VertexCircle3d& other) const {
			return Pos == other.Pos &&
				Colour == other.Colour &&
				TexCoord == other.TexCoord &&
				QuadBound == other.QuadBound &&
				Decorations == other.Decorations &&
				TexIndex == other.TexIndex;
		}
	};

	static std::vector<VkVertexInputAttributeDescription> getVertexTypeAsAttribDesc(
		const EVertexType vertexType,
		const uint32_t binding
	) {
		switch (vertexType) {
			case EVertexType::VERTEX_2D:
				return Vertex2d::asAttributeDescriptions(binding);
			case EVertexType::VERTEX_2D_TEXTURED:
				return Vertex2dTextured::asAttributeDescriptions(binding);
			case EVertexType::VERTEX_2D_TEXTURED_INDEXED:
				return Vertex2dTexturedIndexed::asAttributeDescriptions(binding);
			case EVertexType::VERTEX_3D:
				return Vertex3d::asAttributeDescriptions(binding);
			case EVertexType::VERTEX_3D_TEXTURED:
				return Vertex3dTextured::asAttributeDescriptions(binding);
			case EVertexType::VERTEX_3D_TEXTURED_INDEXED:
				return Vertex3dTexturedIndexed::asAttributeDescriptions(binding);
			case EVertexType::VERTEX_3D_LIT_TEXTURED:
				return Vertex3dLitTextured::asAttributeDescriptions(binding);
			case EVertexType::VERTEX_CIRCLE_3D:
				return VertexCircle3d::asAttributeDescriptions(binding);
			default:
				return {};
		}
	}

	constexpr static uint32_t getVertexTypeComponentCount(const EVertexType vertexType) {
		switch (vertexType) {
			case EVertexType::VERTEX_2D:
				return Vertex2d::COMPONENT_COUNT;
			case EVertexType::VERTEX_2D_TEXTURED:
				return Vertex2dTextured::COMPONENT_COUNT;
			case EVertexType::VERTEX_2D_TEXTURED_INDEXED:
				return Vertex2dTexturedIndexed::COMPONENT_COUNT;
			case EVertexType::VERTEX_3D:
				return Vertex3d::COMPONENT_COUNT;
			case EVertexType::VERTEX_3D_TEXTURED:
				return Vertex3dTextured::COMPONENT_COUNT;
			case EVertexType::VERTEX_3D_TEXTURED_INDEXED:
				return Vertex3dTexturedIndexed::COMPONENT_COUNT;
			case EVertexType::VERTEX_3D_LIT_TEXTURED:
				return Vertex3dLitTextured::COMPONENT_COUNT;
			case EVertexType::VERTEX_CIRCLE_3D:
				return VertexCircle3d::COMPONENT_COUNT;
			default:
				return 0;
		}
	}

	constexpr static uint32_t getVertexTypeSize(const EVertexType vertexType) {
		switch (vertexType) {
			case EVertexType::VERTEX_2D:
				return sizeof(Vertex2d);
			case EVertexType::VERTEX_2D_TEXTURED:
				return sizeof(Vertex2dTextured);
			case EVertexType::VERTEX_2D_TEXTURED_INDEXED:
				return sizeof(Vertex2dTexturedIndexed);
			case EVertexType::VERTEX_3D:
				return sizeof(Vertex3d);
			case EVertexType::VERTEX_3D_TEXTURED:
				return sizeof(Vertex3dTextured);
			case EVertexType::VERTEX_3D_TEXTURED_INDEXED:
				return sizeof(Vertex3dTexturedIndexed);
			case EVertexType::VERTEX_3D_LIT_TEXTURED:
				return sizeof(Vertex3dLitTextured);
			case EVertexType::VERTEX_CIRCLE_3D:
				return sizeof(VertexCircle3d);
			default:
				return 0;
		}
	}

	constexpr static VkVertexInputBindingDescription getVertexTypeBindingDesc(EVertexType vertexType, uint32_t binding, VkVertexInputRate inputRate) {
		VkVertexInputBindingDescription bindDesc = {};
		bindDesc.binding = binding;
		bindDesc.stride = static_cast<uint32_t>(vertexType);
		bindDesc.inputRate = inputRate;
		return bindDesc;
	}
}
