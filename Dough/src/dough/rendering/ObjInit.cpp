#include "dough/rendering/ObjInit.h"

#include "dough/application/Application.h"

#define CONTEXT Application::get().getRenderer().getContext()
//#define CONTEXT DOH::getContext()

namespace DOH {

	//static RenderingContextVulkan& getContext() {
	//	static RenderingContextVulkan& context = Application::get().getRenderer().getContext();
	//	return context;
	//}

	//-----Context-----
	std::shared_ptr<SwapChainVulkan> ObjInit::swapChain(SwapChainCreationInfo& swapChainCreate) {
		return CONTEXT.createSwapChain(swapChainCreate);
	}

	//TODO:: Provide a render pass creation method here?
	//std::shared_ptr<RenderPassVulkan> ObjInit::renderPass()


	//-----VAO-----
	std::shared_ptr<VertexArrayVulkan> ObjInit::vertexArray() {
		return CONTEXT.createVertexArray();
	}

	std::shared_ptr<VertexBufferVulkan> ObjInit::vertexBuffer(
		const AVertexInputLayout& vertexInputLayout,
		VkDeviceSize size,
		VkBufferUsageFlags usage,
		VkMemoryPropertyFlags props
	) {
		return CONTEXT.createVertexBuffer(vertexInputLayout, size, usage, props);
	}
	std::shared_ptr<VertexBufferVulkan> ObjInit::stagedVertexBuffer(
		const AVertexInputLayout& vertexInputLayout,
		const void* data,
		VkDeviceSize size,
		VkBufferUsageFlags usage,
		VkMemoryPropertyFlags props
	) {
		return CONTEXT.createStagedVertexBuffer(vertexInputLayout, data, size, usage, props);
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

	std::shared_ptr<TextureVulkan> ObjInit::texture(
		float r,
		float g,
		float b,
		float a,
		bool colourRgbaNormalised,
		const char* name
	) {
		return CONTEXT.createTexture(r, g, b, a, colourRgbaNormalised, name);
	}
	
	std::shared_ptr<MonoSpaceTextureAtlas> ObjInit::monoSpaceTextureAtlas(
		const std::string& filePath,
		const uint32_t rowCount,
		const uint32_t colCount
	) {
		return CONTEXT.createMonoSpaceTextureAtlas(filePath, rowCount, colCount);
	}

	std::shared_ptr<IndexedTextureAtlas> ObjInit::indexedTextureAtlas(const char* atlasInfoFilePath, const char* atlasTextureDir) {
		return CONTEXT.createIndexedTextureAtlas(atlasInfoFilePath, atlasTextureDir);
	}

	std::shared_ptr<FontBitmap> ObjInit::fontBitmap(const char* filepath, const char* imageDir, ETextRenderMethod textRenderMethod) {
		return CONTEXT.createFontBitmap(filepath, imageDir, textRenderMethod);
	}
}
