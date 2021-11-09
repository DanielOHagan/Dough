#pragma once

#include "dough/Utils.h"
#include "dough/rendering/pipeline/GraphicsPipelineVulkan.h"

namespace DOH {

	//TODO:: learn strings properly
	const std::unique_ptr<std::string> vertShaderPath = std::make_unique<std::string>("res/shaders/vert.spv");
	const std::unique_ptr<std::string> fragShaderPath = std::make_unique<std::string>("res/shaders/frag.spv");
	const std::unique_ptr<std::string> testTexturePath = std::make_unique<std::string>("res/images/testTexture.jpg");
	const std::unique_ptr<std::string> testTexture2Path = std::make_unique<std::string>("res/images/testTexture2.jpg");

	const std::vector<Vertex> vertices {
		{{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f}},
		{{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}, {0.0f}},
		{{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}, {0.0f}},
		{{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}, {0.0f}},

		{{-0.25f, -0.25f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f}},
		{{0.75f, -0.25f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}, {1.0f}},
		{{0.75f, 0.75f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}, {1.0f}},
		{{-0.25f, 0.75f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}, {1.0f}}
	};
	const std::vector<uint16_t> indices {
		0, 1, 2, 2, 3, 0,
		4, 5, 6, 6, 7, 4
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

		std::shared_ptr<GraphicsPipelineVulkan> mGraphicsPipeline;

		//Allocators
		VkDescriptorPool mDescriptorPool;
		VkCommandPool mCommandPool;

		std::shared_ptr<ShaderProgramVulkan> mShaderProgram;

		std::shared_ptr<VertexArrayVulkan> m_TestVAO_VertexArray;

		std::shared_ptr<TextureVulkan> mTestTexture1;
		std::shared_ptr<TextureVulkan> mTestTexture2;

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
