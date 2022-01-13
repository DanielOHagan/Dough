#pragma once

#include "dough/Utils.h"
#include "dough/ResourceHandler.h"
#include "dough/rendering/shader/ShaderVulkan.h"
#include "dough/rendering/pipeline/GraphicsPipelineVulkan.h"

namespace DOH {

	class ObjInit {

	private:
		ObjInit() = delete;
		ObjInit(const ObjInit& copy) = delete;
		ObjInit operator=(const ObjInit& assignment) = delete;

	public:

		//-----Pipeline-----
		static std::shared_ptr<GraphicsPipelineVulkan> graphicsPipeline(
			SwapChainCreationInfo& swapChainCreate,
			ShaderProgramVulkan& shaderProgram
		);

		static std::shared_ptr<SwapChainVulkan> swapChain(
			SwapChainCreationInfo& swapChainCreate
		);

		static std::shared_ptr<RenderPassVulkan> renderPass(VkFormat imageFormat);


		//-----VAO-----
		static std::shared_ptr<VertexArrayVulkan> vertexArray();

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

		static std::shared_ptr<IndexBufferVulkan> indexBuffer(
			VkDeviceSize size,
			uint32_t count
		);
		static std::shared_ptr<IndexBufferVulkan> stagedIndexBuffer(
			void* data,
			VkDeviceSize size,
			uint32_t count
		);
		static std::shared_ptr<IndexBufferVulkan> stagedIndexBuffer(
			const void* data,
			VkDeviceSize size,
			uint32_t count
		);


		//-----Buffer-----
		static std::shared_ptr<BufferVulkan> buffer(
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
		static std::shared_ptr<ShaderVulkan> shader(EShaderType type, std::string& filePath);


		//-----Texture-----
		static std::shared_ptr<TextureVulkan> texture(std::string& filePath);

	};
}
