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
			VkDevice logicDevice,
			VkCommandPool cmdPool,
			SwapChainCreationInfo swapChainCreate,
			ShaderProgramVulkan& shaderProgram
		) {
			return std::make_shared<GraphicsPipelineVulkan>(
				logicDevice,
				cmdPool,
				swapChainCreate,
				shaderProgram
			);
		}

		static std::shared_ptr<SwapChainVulkan> swapChain(
			VkDevice logicDevice,
			SwapChainCreationInfo& swapChainCreate
		) {
			return std::make_shared<SwapChainVulkan>(
				logicDevice,
				swapChainCreate.SupportDetails,
				swapChainCreate.Surface,
				swapChainCreate.Indices,
				swapChainCreate.Width,
				swapChainCreate.Height
			);
		}

		static std::shared_ptr<RenderPassVulkan> renderPass(
			VkDevice logicDevice,
			VkFormat imageFormat
		) {
			return std::make_shared<RenderPassVulkan>(logicDevice, imageFormat);
		}


		//-----VAO-----
		static std::shared_ptr<VertexArrayVulkan> vertexArray() {
			return std::make_shared<VertexArrayVulkan>();
		}

		static std::shared_ptr<VertexBufferVulkan> vertexBuffer(
			const std::initializer_list<BufferElement>& elements,
			VkDevice logicDevice,
			VkPhysicalDevice physicalDevice,
			VkDeviceSize size,
			VkBufferUsageFlags usage,
			VkMemoryPropertyFlags props
		) {
			return std::make_shared<VertexBufferVulkan>(
				elements,
				logicDevice,
				physicalDevice,
				size,
				usage,
				props
			);
		}
		static std::shared_ptr<VertexBufferVulkan> stagedVertexBuffer(
			const std::initializer_list<BufferElement>& elements,
			VkDevice logicDevice,
			VkPhysicalDevice physicalDevice,
			VkCommandPool cmdPool,
			VkQueue graphicsQueue,
			void* data,
			VkDeviceSize size,
			VkBufferUsageFlags usage,
			VkMemoryPropertyFlags props
		) {
			return std::make_shared<VertexBufferVulkan>(
				elements,
				logicDevice,
				physicalDevice,
				cmdPool,
				graphicsQueue,
				(const void*) data,
				size,
				usage,
				props
			);
		}
		static std::shared_ptr<VertexBufferVulkan> stagedVertexBuffer(
			const std::initializer_list<BufferElement>& elements,
			VkDevice logicDevice,
			VkPhysicalDevice physicalDevice,
			VkCommandPool cmdPool,
			VkQueue graphicsQueue,
			const void* data,
			VkDeviceSize size,
			VkBufferUsageFlags usage,
			VkMemoryPropertyFlags props
		) {
			return std::make_shared<VertexBufferVulkan>(
				elements,
				logicDevice,
				physicalDevice,
				cmdPool,
				graphicsQueue,
				data,
				size,
				VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
			);
		};

		static std::shared_ptr<IndexBufferVulkan> indexBuffer(
			VkDevice logicDevice,
			VkPhysicalDevice physicalDevice,
			VkDeviceSize size,
			uint32_t count
		) {
			return std::make_shared<IndexBufferVulkan>(
				logicDevice,
				physicalDevice,
				size,
				count
			);
		}
		static std::shared_ptr<IndexBufferVulkan> stagedIndexBuffer(
			VkDevice logicDevice,
			VkPhysicalDevice physicalDevice,
			VkCommandPool cmdPool,
			VkQueue graphicsQueue,
			const void* data,
			VkDeviceSize size,
			uint32_t count
		) {
			return std::make_shared<IndexBufferVulkan>(
				logicDevice,
				physicalDevice,
				cmdPool,
				graphicsQueue,
				data,
				size,
				count
			);
		}
		static std::shared_ptr<IndexBufferVulkan> stagedIndexBuffer(
			VkDevice logicDevice,
			VkPhysicalDevice physicalDevice,
			VkCommandPool cmdPool,
			VkQueue graphicsQueue,
			void* data,
			VkDeviceSize size,
			uint32_t count
		) {
			return stagedIndexBuffer(
				logicDevice,
				physicalDevice,
				cmdPool,
				graphicsQueue,
				(const void*)data,
				size,
				count
			);
		}


		//-----Buffer-----
		static std::shared_ptr<BufferVulkan> buffer(
			VkDevice logicDevice,
			VkPhysicalDevice physicalDevice,
			VkDeviceSize size,
			VkBufferUsageFlags usage,
			VkMemoryPropertyFlags props
		) {
			return std::make_shared<BufferVulkan>(
				logicDevice,
				physicalDevice,
				size,
				usage,
				props
			);
		}
		static std::shared_ptr<BufferVulkan> stagedBuffer(
			VkDevice logicDevice,
			VkPhysicalDevice physicalDevice,
			VkCommandPool cmdPool,
			VkQueue graphicsQueue,
			const void* data,
			VkDeviceSize size,
			VkBufferUsageFlags usage,
			VkMemoryPropertyFlags props
		) {
			return std::make_shared<BufferVulkan>(
				logicDevice,
				physicalDevice,
				cmdPool,
				graphicsQueue,
				data,
				size,
				usage,
				props
			);
		}
		static std::shared_ptr<BufferVulkan> stagedBuffer(
			VkDevice logicDevice,
			VkPhysicalDevice physicalDevice,
			VkCommandPool cmdPool,
			VkQueue graphicsQueue,
			void* data,
			VkDeviceSize size,
			VkBufferUsageFlags usage,
			VkMemoryPropertyFlags props
		) {
			return std::make_shared<BufferVulkan>(
				logicDevice,
				physicalDevice,
				cmdPool,
				graphicsQueue,
				(const void*) data,
				size,
				usage,
				props
			);
		}


		//-----Shader-----
		static std::shared_ptr<ShaderProgramVulkan> shaderProgram(
			std::shared_ptr<ShaderVulkan> vertShader,
			std::shared_ptr<ShaderVulkan> fragShader
		) {
			return std::make_shared<ShaderProgramVulkan>(vertShader, fragShader);
		}
		static std::shared_ptr<ShaderVulkan> shader(
			VkDevice logicDevice,
			EShaderType type,
			std::string& filePath
		) {
			return std::make_shared<ShaderVulkan>(type, filePath);
		};


		//-----Texture-----
		static std::shared_ptr<TextureVulkan> texture(
			VkDevice logicDevice,
			VkPhysicalDevice physicalDevice,
			VkCommandPool cmdPool,
			VkQueue graphicsQueue,
			std::string& filePath
		) {
			return std::make_shared<TextureVulkan>(
				logicDevice,
				physicalDevice,
				cmdPool,
				graphicsQueue,
				filePath
			);
		};


	};
}
