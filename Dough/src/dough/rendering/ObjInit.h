#pragma once

#include "dough/Utils.h"
#include "dough/files/ResourceHandler.h"
#include "dough/rendering/pipeline/ShaderVulkan.h"
#include "dough/rendering/pipeline/GraphicsPipelineVulkan.h"
#include "dough/rendering/pipeline/ShaderDescriptorSetLayoutsVulkan.h"
#include "dough/rendering/textures/TextureAtlas.h"
#include "dough/rendering/SwapChainVulkan.h"
#include "dough/rendering/text/FontBitmap.h"
#include "dough/rendering/buffer/VertexArrayVulkan.h"

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


		//-----VAO-----
		static std::shared_ptr<VertexArrayVulkan> vertexArray();


		static std::shared_ptr<VertexBufferVulkan> vertexBuffer(
			const AVertexInputLayout& vertexInputLayout,
			VkDeviceSize size,
			VkBufferUsageFlags usage,
			VkMemoryPropertyFlags props
		);
		static std::shared_ptr<VertexBufferVulkan> stagedVertexBuffer(
			const AVertexInputLayout& vertexInputLayout,
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
		static std::shared_ptr<ShaderProgram> shaderProgram(
			std::shared_ptr<ShaderVulkan> vertShader,
			std::shared_ptr<ShaderVulkan> fragShader,
			std::shared_ptr<ShaderDescriptorSetLayoutsVulkan> descriptorSetLayouts
		);
		static std::shared_ptr<ShaderVulkan> shader(EShaderStage stage, const char* filePath);


		//-----Texture-----
		static std::shared_ptr<TextureVulkan> texture(const std::string& filePath);
		//RGBA values range from 0 - 255
		static std::shared_ptr<TextureVulkan> texture(float r, float g, float b, float a, bool colourRgbaNormalised = false, const char* name = "Un-named Texture");
		static std::shared_ptr<MonoSpaceTextureAtlas> monoSpaceTextureAtlas(
			const std::string& filePath,
			const uint32_t rowCount,
			const uint32_t columnCount
		);
		static std::shared_ptr<IndexedTextureAtlas> indexedTextureAtlas(const char* atlasInfoFilePath, const char* atlasTextureDir);

		//-----Font-----
		static std::shared_ptr<FontBitmap> fontBitmap(const char* filepath, const char* imageDir, ETextRenderMethod textRenderMethod);
	};
}
