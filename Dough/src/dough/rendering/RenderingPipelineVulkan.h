#pragma once

#include "dough/rendering/SwapChainVulkan.h"
#include "dough/rendering/buffer/BufferVulkan.h"

namespace DOH {

	const std::vector<Vertex> vertices {
		{{0.0f, -0.5f}, {1.0f, 1.0f, 1.0f}},
		{{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
		{{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
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

		BufferVulkan mVertexBuffer;

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

		//TODO:: If engine only supports one device in use at a time,
		// then do checks to make sure that if it is switched mid-application then the previously used device is properly closed.
		void setLogicDevice(VkDevice logicDevice) { mLogicDevice = logicDevice; }
		void setPhysicalDevice(VkPhysicalDevice physicalDevice) { mPhysicalDevice = physicalDevice; }

	private:

		void createQueues(QueueFamilyIndices& queueFamilyIndices);

		void createRenderPass();
		void createGraphicsPipeline();

		void createCommandPool(QueueFamilyIndices& queueFamilyIndices);
		void createCommandBuffers();

		void createSyncObjects();

		void closeOldSwapChain();
	};
}