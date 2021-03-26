#pragma once

#include "dough/rendering/SwapChainVulkan.h"
#include "dough/rendering/buffer/BufferVulkan.h"
#include "dough/rendering/buffer/IndexBufferVulkan.h"

namespace DOH {

	const std::vector<Vertex> vertices {
		{{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
		{{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
		{{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
		{{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
	};
	const std::vector<uint16_t> indices {
		0, 1, 2, 2, 3, 0
	};
	struct UniformBufferObject {
		glm::mat4 model;
		glm::mat4 view;
		glm::mat4 proj;
	};

	class RenderingPipelineVulkan {

	private:

		VkDevice mLogicDevice;
		VkPhysicalDevice mPhysicalDevice;

		VkRenderPass mRenderPass;
		VkPipelineLayout mPipelineLayout;
		VkPipeline mGraphicsPipeline;

		VkQueue mGraphicsQueue;
		VkQueue mPresentQueue;

		VkCommandPool mCommandPool;
		std::vector<VkCommandBuffer> mCommandBuffers;

		SwapChainVulkan mSwapChain;

		const int MAX_FRAMES_IN_FLIGHT = 2;
		size_t mCurrentFrame;
		std::vector<VkSemaphore> mImageAvailableSemaphores;
		std::vector<VkSemaphore> mRenderFinishedSemaphores;
		std::vector<VkFence> mInFlightFences;
		std::vector<VkFence> mImageFencesInFlight;

		//TEMP:: For developing implementation of VAO
		BufferVulkan mVertexBuffer;
		IndexBufferVulkan mIndexBuffer;

		VkDescriptorSetLayout mDescriptorSetLayout;
		std::vector<BufferVulkan> mUniformBuffers;
		VkDescriptorPool mDescriptorPool;
		std::vector<VkDescriptorSet> mDescriptorSets;

	public:

		RenderingPipelineVulkan();

		void init(
			VkDevice logicDevice,
			VkPhysicalDevice physicalDevice,
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

		//TODO:: If engine only supports one device in use at a time,
		// then do checks to make sure that if it is switched mid-application then the previously used device is properly closed.
		void setLogicDevice(VkDevice logicDevice) { mLogicDevice = logicDevice; }
		void setPhysicalDevice(VkPhysicalDevice physicalDevice) { mPhysicalDevice = physicalDevice; }

	private:

		void createQueues(QueueFamilyIndices& queueFamilyIndices);

		void createRenderPass();
		void createDescriptorSetLayout();
		void createGraphicsPipeline();

		void createCommandPool(QueueFamilyIndices& queueFamilyIndices);
		void createCommandBuffers();

		void createUniformBuffers();
		void createDescriptorPool();
		void createDescriptorSets();

		void createSyncObjects();

		void closeOldSwapChain();
	};
}