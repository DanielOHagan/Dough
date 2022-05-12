#include "dough/rendering/ObjInit.h"

#include "dough/application/Application.h"

#define CONTEXT Application::get().getRenderer().getContext()

namespace DOH {

	//-----Pipeline-----
	std::shared_ptr<GraphicsPipelineVulkan> ObjInit::graphicsPipeline(
		VkExtent2D extent,
		VkRenderPass renderPass,
		ShaderProgramVulkan& shaderProgram,
		VkVertexInputBindingDescription vertexInputBindingDesc,
		std::vector<VkVertexInputAttributeDescription>& vertexAttributes
	) {
		return CONTEXT.createGraphicsPipeline(extent, renderPass, shaderProgram, vertexInputBindingDesc, vertexAttributes);
	}

	//-----Context-----
	std::shared_ptr<SwapChainVulkan> ObjInit::swapChain(SwapChainCreationInfo& swapChainCreate) {
		return CONTEXT.createSwapChain(swapChainCreate);
	}

	std::shared_ptr<RenderPassVulkan> ObjInit::renderPass(
		VkFormat imageFormat,
		bool hasPassBefore,
		bool hasPassAfter,
		bool enableClearColour,
		VkClearValue clearColour
	) {
		return CONTEXT.createRenderPass(imageFormat, hasPassBefore, hasPassAfter, enableClearColour, clearColour);
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
	std::shared_ptr<VertexBufferVulkan> ObjInit::stagedVertexBuffer(
		const std::vector<BufferElement>& elements,
		const void* data,
		VkDeviceSize size,
		VkBufferUsageFlags usage,
		VkMemoryPropertyFlags props
	) {
		return CONTEXT.createStagedVertexBuffer(elements, data, size, usage, props);
	}

	std::shared_ptr<IndexBufferVulkan> ObjInit::indexBuffer(VkDeviceSize size) {
		return CONTEXT.createIndexBuffer(size);
	}
	std::shared_ptr<IndexBufferVulkan> ObjInit::stagedIndexBuffer(void* data, VkDeviceSize size) {
		return CONTEXT.createStagedIndexBuffer((const void*) data, size);
	}
	std::shared_ptr<IndexBufferVulkan> ObjInit::stagedIndexBuffer(const void* data, VkDeviceSize size) {
		return CONTEXT.createStagedIndexBuffer(data, size);
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
	std::shared_ptr<ShaderVulkan> ObjInit::shader(EShaderType type, const std::string& filePath) {
		return CONTEXT.createShader(type, filePath);
	}


	//-----Texture-----
	std::shared_ptr<TextureVulkan> ObjInit::texture(const std::string& filePath) {
		return CONTEXT.createTexture(filePath);
	}

	std::shared_ptr<TextureVulkan> ObjInit::texture(float r, float g, float b, float a, bool colourRgbaNormalised) {
		return CONTEXT.createTexture(r, g, b, a, colourRgbaNormalised);
	}
}
