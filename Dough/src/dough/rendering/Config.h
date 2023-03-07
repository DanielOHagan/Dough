#pragma once

#include "dough/Core.h"
#include "dough/Maths.h"
#include "dough/rendering/buffer/DataType.h"
#include "dough/rendering/buffer/BufferElement.h"

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
		inline void setHeight(uint32_t height) {SupportDetails.capabilities.currentExtent.height = height;}
	};

	//TODO:: Some kind of Vertex Input builder?
	//	Instead of using pre-defined structs (Vertex2d, Vertex3d, etc...)

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

		constexpr static const uint32_t COMPONENT_COUNT = 6;
		constexpr static const uint32_t BYTE_SIZE = COMPONENT_COUNT * DataType::getDataTypeSize(EDataType::FLOAT);

		static std::vector<VkVertexInputAttributeDescription> asAttributeDescriptions(uint32_t binding) {
			return {
				{ 0, binding, VK_FORMAT_R32G32_SFLOAT		, offsetof(Vertex2d, Pos) },
				{ 1, binding, VK_FORMAT_R32G32B32A32_SFLOAT	, offsetof(Vertex2d, Colour) }
			};
		}

		static std::initializer_list<BufferElement> asBufferElements() {
			return { EDataType::FLOAT2, EDataType::FLOAT4 };
		}

		bool operator==(const Vertex2d& other) const {
			return Pos == other.Pos && Colour == other.Colour;
		}
	};

	struct Vertex3d {
		glm::vec3 Pos;
		glm::vec4 Colour;

		constexpr static const uint32_t COMPONENT_COUNT = 7;
		constexpr static const uint32_t BYTE_SIZE = COMPONENT_COUNT * DataType::getDataTypeSize(EDataType::FLOAT);

		static std::vector<VkVertexInputAttributeDescription> asAttributeDescriptions(uint32_t binding) {
			return {
				{ 0, binding, VK_FORMAT_R32G32B32_SFLOAT	, offsetof(Vertex3d, Pos) },
				{ 1, binding, VK_FORMAT_R32G32B32A32_SFLOAT	, offsetof(Vertex3d, Colour) }
			};
		}

		static std::initializer_list<BufferElement> asBufferElements() {
			return { EDataType::FLOAT3, EDataType::FLOAT4 };
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

		static std::vector<VkVertexInputAttributeDescription> asAttributeDescriptions(uint32_t binding) {
			return {
				{ 0, binding, VK_FORMAT_R32G32B32_SFLOAT	, offsetof(Vertex3dTextured, Pos) },
				{ 1, binding, VK_FORMAT_R32G32B32A32_SFLOAT	, offsetof(Vertex3dTextured, Colour) },
				{ 2, binding, VK_FORMAT_R32G32_SFLOAT		, offsetof(Vertex3dTextured, TexCoord) }
			};
		}

		static std::initializer_list<BufferElement> asBufferElements() {
			return { EDataType::FLOAT3, EDataType::FLOAT4, EDataType::FLOAT2 }; //, EDataType::FLOAT };
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

		static std::vector<VkVertexInputAttributeDescription> asAttributeDescriptions(uint32_t binding) {
			return {
				{ 0, binding, VK_FORMAT_R32G32B32_SFLOAT	, offsetof(Vertex3dTexturedIndexed, Pos) },
				{ 1, binding, VK_FORMAT_R32G32B32A32_SFLOAT	, offsetof(Vertex3dTexturedIndexed, Colour) },
				{ 2, binding, VK_FORMAT_R32G32_SFLOAT		, offsetof(Vertex3dTexturedIndexed, TexCoord) },
				{ 3, binding, VK_FORMAT_R32_SFLOAT			, offsetof(Vertex3dTexturedIndexed, TexIndex) },
			};
		}

		static std::initializer_list<BufferElement> asBufferElements() {
			return { EDataType::FLOAT3, EDataType::FLOAT4, EDataType::FLOAT2, EDataType::FLOAT };
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

		static std::vector<VkVertexInputAttributeDescription> asAttributeDescriptions(uint32_t binding) {
			return {
				{ 0, binding, VK_FORMAT_R32G32B32_SFLOAT	, offsetof(Vertex3dLitTextured, Pos) },
				{ 1, binding, VK_FORMAT_R32G32B32A32_SFLOAT	, offsetof(Vertex3dLitTextured, Colour) },
				{ 2, binding, VK_FORMAT_R32G32B32A32_SFLOAT	, offsetof(Vertex3dLitTextured, Normal) },
				{ 3, binding, VK_FORMAT_R32G32_SFLOAT		, offsetof(Vertex3dLitTextured, TexCoord) }
			};
		}

		static std::initializer_list<BufferElement> asBufferElements() {
			return { EDataType::FLOAT3, EDataType::FLOAT4, EDataType::FLOAT3, EDataType::FLOAT2 };
		}

		bool operator==(const Vertex3dLitTextured& other) const {
			return Pos == other.Pos && Colour == other.Colour && Normal == other.Normal && TexCoord == other.TexCoord;
		}
	};

	enum class EVertexType {
		/** 
		* Default value so itshould never be used as vertex input.
		*/
		NONE = 0,

		/**
		* vec2 float Pos
		* vec4 float Colour
		*/
		VERTEX_2D = sizeof(Vertex2d),

		/**
		* vec3 float Pos
		* vec4 float Colour
		*/
		VERTEX_3D = sizeof(Vertex3d),

		/**
		* vec3 float Pos
		* vec4 float Colour
		* vec2 float TexCoord
		*/
		VERTEX_3D_TEXTURED = sizeof(Vertex3dTextured),

		/**
		* vec3 float Pos
		* vec4 float Colour
		* vec2 float TexCoord
		* 1 float TexIndex
		*/
		VERTEX_3D_TEXTURED_INDEXED = sizeof(Vertex3dTexturedIndexed),

		/**
		* vec3 float Pos
		* vec4 float Colour
		* vec3 float Normal
		* vec2 float TexCoord
		*/
		VERTEX_3D_LIT_TEXTURED = sizeof(Vertex3dLitTextured)
	};

	static std::vector<VkVertexInputAttributeDescription> getVertexTypeAsAttribDesc(
		const EVertexType vertexType,
		const uint32_t binding
	) {
		switch (vertexType) {
			case EVertexType::VERTEX_2D:
				return Vertex2d::asAttributeDescriptions(binding);

			case EVertexType::VERTEX_3D:
				return Vertex3d::asAttributeDescriptions(binding);

			case EVertexType::VERTEX_3D_TEXTURED:
				return Vertex3dTextured::asAttributeDescriptions(binding);

			case EVertexType::VERTEX_3D_TEXTURED_INDEXED:
				return Vertex3dTexturedIndexed::asAttributeDescriptions(binding);

			case EVertexType::VERTEX_3D_LIT_TEXTURED:
				return Vertex3dLitTextured::asAttributeDescriptions(binding);

			default:
				return {};
		}
	}

	constexpr static uint32_t getVertexTypeComponentCount(const EVertexType vertexType) {
		switch (vertexType) {
			case EVertexType::VERTEX_2D:
				return Vertex2d::COMPONENT_COUNT;

			case EVertexType::VERTEX_3D:
				return Vertex3d::COMPONENT_COUNT;

			case EVertexType::VERTEX_3D_TEXTURED:
				return Vertex3dTextured::COMPONENT_COUNT;

			case EVertexType::VERTEX_3D_TEXTURED_INDEXED:
				return Vertex3dTexturedIndexed::COMPONENT_COUNT;

			case EVertexType::VERTEX_3D_LIT_TEXTURED:
				return Vertex3dLitTextured::COMPONENT_COUNT;

			default:
				return 0;
		}
	}

	constexpr static uint32_t getVertexTypeSize(const EVertexType vertexType) {
		switch (vertexType) {
			case EVertexType::VERTEX_2D:
				return sizeof(Vertex2d);

			case EVertexType::VERTEX_3D:
				return sizeof(Vertex3d);

			case EVertexType::VERTEX_3D_TEXTURED:
				return sizeof(Vertex3dTextured);

			case EVertexType::VERTEX_3D_TEXTURED_INDEXED:
				return sizeof(Vertex3dTexturedIndexed);

			case EVertexType::VERTEX_3D_LIT_TEXTURED:
				return sizeof(Vertex3dLitTextured);

			default:
				return 0;
		}
	}

	constexpr static std::initializer_list<BufferElement> getVertexTypeAsBufferElements(const EVertexType vertexType) {
		switch (vertexType) {
			case EVertexType::VERTEX_2D:
				return Vertex2d::asBufferElements();

			case EVertexType::VERTEX_3D:
				return Vertex3d::asBufferElements();

			case EVertexType::VERTEX_3D_TEXTURED:
				return Vertex3dTextured::asBufferElements();

			case EVertexType::VERTEX_3D_TEXTURED_INDEXED:
				return Vertex3dTexturedIndexed::asBufferElements();

			case EVertexType::VERTEX_3D_LIT_TEXTURED:
				return Vertex3dLitTextured::asBufferElements();

			default:
				return {};
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
