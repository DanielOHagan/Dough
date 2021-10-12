#pragma once

#include "dough/Utils.h"
#include "dough/rendering/buffer/IndexBufferVulkan.h"
#include "dough/rendering/TextureVulkan.h"
#include "dough/rendering/pipeline/GraphicsPipelineVulkan.h"

namespace DOH {

	const std::string vertShaderPath = "res/shaders/vert.spv";
	const std::string fragShaderPath = "res/shaders/frag.spv";
	const std::string testTexturePath = "res/images/testTexture.jpg";
	const std::vector<Vertex> vertices {
		{{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
		{{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
		{{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
		{{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}
	};
	const std::vector<uint16_t> indices {
		0, 1, 2, 2, 3, 0
	};
	struct UniformBufferObject {
		alignas(16) glm::mat4 model;
		alignas(16) glm::mat4 view;
		alignas(16) glm::mat4 proj;
	};

	class RenderingContextVulkan {

	private:
		
		//Shared device handles for convenience
		VkDevice mLogicDevice;
		VkPhysicalDevice mPhysicalDevice;
		std::unique_ptr<VkPhysicalDeviceProperties> mPhysicalDeviceProperties;
		
		VkQueue mGraphicsQueue;
		VkQueue mPresentQueue;

		std::unique_ptr<GraphicsPipelineVulkan> mGraphicsPipeline;

		//Allocators
		VkDescriptorPool mDescriptorPool;
		VkCommandPool mCommandPool;

		//TEMP:: For developing impl of VAO
		std::unique_ptr<BufferVulkan> mVertexBuffer;
		std::unique_ptr<IndexBufferVulkan> mIndexBuffer;

		//TEMP::
		std::unique_ptr<TextureVulkan> mTestTexture;

		const int MAX_FRAMES_IN_FLIGHT = 2;
		size_t mCurrentFrame;
		std::vector<VkSemaphore> mImageAvailableSemaphores;
		std::vector<VkSemaphore> mRenderFinishedSemaphores;
		std::vector<VkFence> mInFlightFences;
		std::vector<VkFence> mImageFencesInFlight;

	public:
		RenderingContextVulkan(VkDevice logicDevice, VkPhysicalDevice physicalDevice);

		void init(
			SwapChainSupportDetails& scSupport,
			VkSurfaceKHR surface,
			QueueFamilyIndices& queueFamilyIndices,
			uint32_t width = 100,
			uint32_t height = 100
		);

		void close();

		void resizeSwapChain(
			SwapChainSupportDetails& scSupport,
			VkSurfaceKHR surface,
			QueueFamilyIndices& queueFamilyIndices,
			uint32_t width,
			uint32_t height
		);

		void drawFrame();
		void updateUniformBuffer(uint32_t currentImage);

		VkCommandBuffer beginSingleTimeCommands();
		void endSingleTimeCommands(VkCommandBuffer cmdBuffer);

		void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

		VkImageView createImageView(VkImage image, VkFormat format);
		VkSampler createSampler();

		void setLogicDevice(VkDevice logicDevice) { mLogicDevice = logicDevice; }
		void setPhysicalDevice(VkPhysicalDevice physicalDevice);

	private:

		//NOTE:: Convenience function, used in initial creation of Pipeline and in resizing
		void preparePipeline();

		void createQueues(QueueFamilyIndices& queueFamilyIndices);

		void createCommandPool(QueueFamilyIndices& queueFamilyIndices);
		void createDescriptorPool();

		void createSyncObjects();
	};
}
