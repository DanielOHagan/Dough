#include "dough/rendering/ObjInit.h"

#include "dough/application/Application.h"

#define CONTEXT Application::get().getRenderer().getContext()

namespace DOH {

	//-----Pipeline-----
	std::shared_ptr<GraphicsPipelineVulkan> ObjInit::graphicsPipeline(
		SwapChainCreationInfo& swapChainCreate,
		ShaderProgramVulkan& shaderProgram
	) {
		return CONTEXT.createGraphicsPipeline(swapChainCreate, shaderProgram);
	}

	std::shared_ptr<SwapChainVulkan> ObjInit::swapChain(SwapChainCreationInfo& swapChainCreate) {
		return CONTEXT.createSwapChain(swapChainCreate);
	}

	std::shared_ptr<RenderPassVulkan> ObjInit::renderPass(VkFormat imageFormat) {
		return CONTEXT.createRenderPass(imageFormat);
	}


	//-----VAO-----
	std::shared_ptr<VertexArrayVulkan> ObjInit::vertexArray() {
		return CONTEXT.createVertexArray();
	}

	std::shared_ptr<VertexBufferVulkan> ObjInit::vertexBuffer(
		const std::initializer_list<BufferElement>& elements,
		VkDeviceSize size,
		VkBufferUsageFlags usage,
		VkMemoryPropertyFlags props
	) {
		return CONTEXT.createVertexBuffer(elements, size, usage, props);
	}
	std::shared_ptr<VertexBufferVulkan> ObjInit::stagedVertexBuffer(
		const std::initializer_list<BufferElement>& elements,
		void* data,
		VkDeviceSize size,
		VkBufferUsageFlags usage,
		VkMemoryPropertyFlags props
	) {
		return CONTEXT.createStagedVertexBuffer(elements, (const void*) data, size, usage, props);
	}
	std::shared_ptr<VertexBufferVulkan> ObjInit::stagedVertexBuffer(
		const std::initializer_list<BufferElement>& elements,
		const void* data,
		VkDeviceSize size,
		VkBufferUsageFlags usage,
		VkMemoryPropertyFlags props
	) {
		return CONTEXT.createStagedVertexBuffer(elements, data, size, usage, props);
	}

	std::shared_ptr<IndexBufferVulkan> ObjInit::indexBuffer(
		VkDeviceSize size,
		uint32_t count
	) {
		return CONTEXT.createIndexBuffer(size, count);
	}
	std::shared_ptr<IndexBufferVulkan> ObjInit::stagedIndexBuffer(
		void* data,
		VkDeviceSize size,
		uint32_t count
	) {
		return CONTEXT.createStagedIndexBuffer((const void*) data, size, count);
	}
	std::shared_ptr<IndexBufferVulkan> ObjInit::stagedIndexBuffer(
		const void* data,
		VkDeviceSize size,
		uint32_t count
	) {
		return CONTEXT.createStagedIndexBuffer(data, size, count);
	}


	//-----Buffer-----
	std::shared_ptr<BufferVulkan> ObjInit::buffer(
		VkDeviceSize size,
		VkBufferUsageFlags usage,
		VkMemoryPropertyFlags props
	) {
		return CONTEXT.createBuffer(size, usage, props);
	}
	std::shared_ptr<BufferVulkan> ObjInit::stagedBuffer(
		const void* data,
		VkDeviceSize size,
		VkBufferUsageFlags usage,
		VkMemoryPropertyFlags props
	) {
		return CONTEXT.createStagedBuffer(data, size, usage, props);
	}
	std::shared_ptr<BufferVulkan> ObjInit::stagedBuffer(
		void* data,
		VkDeviceSize size,
		VkBufferUsageFlags usage,
		VkMemoryPropertyFlags props
	) {
		return CONTEXT.createStagedBuffer((const void*) data, size, usage, props);
	}


	//-----Shader-----
	std::shared_ptr<ShaderProgramVulkan> ObjInit::shaderProgram(
		//TODO:: Maybe use std::move shared ptr here so shader program instance has ownership of shaders
		std::shared_ptr<ShaderVulkan> vertShader,
		std::shared_ptr<ShaderVulkan> fragShader
	) {
		return CONTEXT.createShaderProgram(vertShader, fragShader);
	}
	std::shared_ptr<ShaderVulkan> ObjInit::shader(EShaderType type, std::string& filePath) {
		return CONTEXT.createShader(type, filePath);
	}


	//-----Texture-----
	std::shared_ptr<TextureVulkan> ObjInit::texture(std::string& filePath) {
		return CONTEXT.createTexture(filePath);
	}

}
