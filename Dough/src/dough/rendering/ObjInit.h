#pragma once

#include "dough/Utils.h"
#include "dough/ResourceHandler.h"
#include "dough/rendering/pipeline/shader/ShaderVulkan.h"
#include "dough/rendering/pipeline/GraphicsPipelineVulkan.h"
#include "dough/rendering/textures/TextureAtlasVulkan.h"

namespace DOH {

	class ObjInit {

	private:
		ObjInit() = delete;
		ObjInit(const ObjInit& copy) = delete;
		ObjInit operator=(const ObjInit& assignment) = delete;

	public:

		//-----Context-----
		static std::shared_ptr<SwapChainVulkan> swapChain(
			SwapChainCreationInfo& swapChainCreate
		);

		//static std::shared_ptr<RenderPassVulkan> renderPass(
		//	VkFormat imageFormat,
		//	VkImageLayout initialLayout,
		//	VkImageLayout finalLayout,
		//	VkAttachmentLoadOp loadOp,
		//	bool enableClearColour,
		//	VkClearValue clearColour = {0.264f, 0.328f, 0.484f, 1.0f}
		//);


		//-----VAO-----
		static std::shared_ptr<VertexArrayVulkan> vertexArray();

		//TODO:: Use BufferElements for custom vertex buffers and EVertexTypes for pre-defined vertex input

		static std::shared_ptr<VertexBufferVulkan> vertexBuffer(
			const std::initializer_list<BufferElement>& elements,
			VkDeviceSize size,
			VkBufferUsageFlags usage,
			VkMemoryPropertyFlags props
		);
		static std::shared_ptr<VertexBufferVulkan> stagedVertexBuffer(
			const std::initializer_list<BufferElement>& elements,
			void* data,
			VkDeviceSize size,
			VkBufferUsageFlags usage,
			VkMemoryPropertyFlags props
		);
		static std::shared_ptr<VertexBufferVulkan> stagedVertexBuffer(
			const std::initializer_list<BufferElement>& elements,
			const void* data,
			VkDeviceSize size,
			VkBufferUsageFlags usage,
			VkMemoryPropertyFlags props
		);
		static std::shared_ptr<VertexBufferVulkan> stagedVertexBuffer(
			const std::vector<BufferElement>& elements,
			const void* data,
			VkDeviceSize size,
			VkBufferUsageFlags usage,
			VkMemoryPropertyFlags props
		);
		static std::shared_ptr<VertexBufferVulkan> stagedVertexBuffer(
			const EVertexType vertexType,
			const void* data,
			VkDeviceSize size,
			VkBufferUsageFlags usage,
			VkMemoryPropertyFlags props
		);

		static std::shared_ptr<IndexBufferVulkan> indexBuffer(VkDeviceSize size);
		static std::shared_ptr<IndexBufferVulkan> stagedIndexBuffer(void* data, VkDeviceSize size);
		static std::shared_ptr<IndexBufferVulkan> stagedIndexBuffer(const void* data, VkDeviceSize size);


		//-----Buffer-----
		static std::shared_ptr<BufferVulkan> buffer(
			VkDeviceSize size,
			VkBufferUsageFlags usage,
			VkMemoryPropertyFlags props
		);
		static std::shared_ptr<VertexBufferVulkan> vertexBuffer(
			const EVertexType vertexType,
			VkDeviceSize size,
			VkBufferUsageFlags usage,
			VkMemoryPropertyFlags props
		);
		static std::shared_ptr<BufferVulkan> stagedBuffer(
			const void* data,
			VkDeviceSize size,
			VkBufferUsageFlags usage,
			VkMemoryPropertyFlags props
		);
		static std::shared_ptr<BufferVulkan> stagedBuffer(
			void* data,
			VkDeviceSize size,
			VkBufferUsageFlags usage,
			VkMemoryPropertyFlags props
		);


		//-----Shader-----
		static std::shared_ptr<ShaderProgramVulkan> shaderProgram(
			//TODO:: Maybe use std::move shared ptr here so shader program instance has ownership of shaders
			std::shared_ptr<ShaderVulkan> vertShader,
			std::shared_ptr<ShaderVulkan> fragShader
		);
		static std::shared_ptr<ShaderVulkan> shader(EShaderType type, const std::string& filePath);


		//-----Texture-----
		static std::shared_ptr<TextureVulkan> texture(const std::string& filePath);
		//RGBA values range from 0 - 255
		static std::shared_ptr<TextureVulkan> texture(float r, float g, float b, float a, bool colourRgbaNormalised = false);
		static std::shared_ptr<MonoSpaceTextureAtlasVulkan> monoSpaceTextureAtlas(
			const std::string& filePath,
			const uint32_t rowCount,
			const uint32_t columnCount
		);
	};
}
