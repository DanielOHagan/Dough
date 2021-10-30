#pragma once

#include "dough/Utils.h"
#include "dough/rendering/buffer/VertexArrayVulkan.h"
#include "dough/rendering/shader/ShaderVulkan.h"
#include "dough/rendering/TextureVulkan.h"

namespace DOH {

	class ObjInit {

	private:
		ObjInit() = delete;
		ObjInit(const ObjInit& copy) = delete;
		ObjInit operator=(const ObjInit& assignment) = delete;

	public:

		//-----Pipeline-----


		//-----VAO-----
		static std::shared_ptr<VertexArrayVulkan> vertexArray() {
			return std::make_shared<VertexArrayVulkan>(VertexArrayVulkan());
		}


		//-----Buffer-----
		static std::shared_ptr<BufferVulkan> buffer(
			VkDevice logicDevice,
			VkPhysicalDevice physicalDevice,
			VkDeviceSize size,
			VkBufferUsageFlags usage,
			VkMemoryPropertyFlags props
		) {
			return std::make_shared<BufferVulkan>(BufferVulkan::createBuffer(
				logicDevice,
				physicalDevice,
				size,
				usage,
				props
			));
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
			return std::make_shared<BufferVulkan>(BufferVulkan::createStagedBuffer(
				logicDevice,
				physicalDevice,
				cmdPool,
				graphicsQueue,
				data,
				size,
				usage,
				props
			));
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
			return stagedBuffer(
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
			return std::make_shared<VertexBufferVulkan>(VertexBufferVulkan::createStagedVertexBuffer(
				elements,
				logicDevice,
				physicalDevice,
				cmdPool,
				graphicsQueue,
				data,
				size,
				VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
			));
		};

		static std::shared_ptr<IndexBufferVulkan> indexBuffer(
			VkDevice logicDevice,
			VkPhysicalDevice physicalDevice,
			VkDeviceSize size,
			uint32_t count
		) {
			return std::make_shared<IndexBufferVulkan>(IndexBufferVulkan::createIndexBuffer(
				logicDevice,
				physicalDevice,
				size,
				count
			));
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
			return std::make_shared<IndexBufferVulkan>(IndexBufferVulkan::createStagedIndexBuffer(
				logicDevice,
				physicalDevice,
				cmdPool,
				graphicsQueue,
				data,
				size,
				count
			));
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
				(const void*) data,
				size,
				count
			);
		}


		//-----Shader-----
		static std::shared_ptr<ShaderVulkan> shader(
			VkDevice logicDevice,
			EShaderType type,
			const std::string& filePath
		) {
			return std::make_shared<ShaderVulkan>(ShaderVulkan::createShader(
				logicDevice,
				type,
				filePath
			));
		};


		//-----Texture-----
		static std::shared_ptr<TextureVulkan> texture(
			VkDevice logicDevice,
			VkPhysicalDevice physicalDevice,
			VkCommandPool cmdPool,
			VkQueue graphicsQueue,
			std::string& filePath
		) {
			return std::make_shared<TextureVulkan>(TextureVulkan::createTexture(
				logicDevice,
				physicalDevice,
				cmdPool,
				graphicsQueue,
				filePath
			));
		};


	};
}
