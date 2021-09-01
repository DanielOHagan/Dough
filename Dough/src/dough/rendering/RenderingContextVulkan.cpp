#include "dough/rendering/RenderingContextVulkan.h"

#include "dough/rendering/shader/ShaderVulkan.h"

#include <chrono>
#include <glm/gtc/matrix_transform.hpp>

namespace DOH {

	RenderingContextVulkan::RenderingContextVulkan(VkDevice logicDevice, VkPhysicalDevice physicalDevice)
	:	mLogicDevice(logicDevice),
		mPhysicalDevice(physicalDevice),
		mGraphicsQueue(VK_NULL_HANDLE),
		mPresentQueue(VK_NULL_HANDLE),
		mGraphicsPipeline(GraphicsPipelineVulkan::createNonInit()),
		mDescriptorPool(VK_NULL_HANDLE),
		mCommandPool(VK_NULL_HANDLE),
		mCurrentFrame(0)
	{
	}

	void RenderingContextVulkan::init(
		SwapChainSupportDetails& scSupport,
		VkSurfaceKHR surface,
		QueueFamilyIndices& queueFamilyIndices,
		uint32_t width,
		uint32_t height
	) {
		createQueues(queueFamilyIndices);

		createCommandPool(queueFamilyIndices);

		GraphicsPipelineVulkan::create(
			mLogicDevice,
			scSupport,
			surface,
			queueFamilyIndices,
			width,
			height,
			(std::string&) vertShaderPath,
			(std::string&) fragShaderPath,
			mCommandPool,
			sizeof(UniformBufferObject),
			&mGraphicsPipeline
		);

		mVertexBuffer = BufferVulkan::createStagedBuffer(
			mLogicDevice,
			mPhysicalDevice,
			mCommandPool,
			mGraphicsQueue,
			vertices.data(),
			sizeof(vertices[0]) * vertices.size(),
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		);
		mIndexBuffer = IndexBufferVulkan::createStagedIndexBuffer(
			mLogicDevice,
			mPhysicalDevice,
			mCommandPool,
			mGraphicsQueue,
			indices.data(),
			sizeof(indices[0]) * indices.size(),
			static_cast<uint32_t>(indices.size())
		);

		preparePipeline();

		createSyncObjects();
	}

	void RenderingContextVulkan::close() {
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			vkDestroySemaphore(mLogicDevice, mImageAvailableSemaphores[i], nullptr);
			vkDestroySemaphore(mLogicDevice, mRenderFinishedSemaphores[i], nullptr);
			vkDestroyFence(mLogicDevice, mInFlightFences[i], nullptr);
		}

		mVertexBuffer.close(mLogicDevice);
		mIndexBuffer.close(mLogicDevice);
		
		mGraphicsPipeline.close(mLogicDevice);

		vkDestroyCommandPool(mLogicDevice, mCommandPool, nullptr);
		vkDestroyDescriptorPool(mLogicDevice, mDescriptorPool, nullptr);
	}

	void RenderingContextVulkan::resizeSwapChain(
		SwapChainSupportDetails& scSupport,
		VkSurfaceKHR surface,
		QueueFamilyIndices& queueFamilyIndices,
		uint32_t width,
		uint32_t height
	) {
		if (mGraphicsPipeline.getSwapChain().isResizable()) {
			vkDeviceWaitIdle(mLogicDevice);

			mGraphicsPipeline.close(mLogicDevice);

			GraphicsPipelineVulkan::create(
				mLogicDevice,
				scSupport,
				surface,
				queueFamilyIndices,
				width,
				height,
				(std::string&) vertShaderPath,
				(std::string&) fragShaderPath,
				mCommandPool,
				sizeof(UniformBufferObject),
				&mGraphicsPipeline
			);

			vkDestroyDescriptorPool(mLogicDevice, mDescriptorPool, nullptr);

			preparePipeline();
		}
	}

	void RenderingContextVulkan::drawFrame() {
		//Wait for the fences on the GPU to flag as finished
		vkWaitForFences(mLogicDevice, 1, &mInFlightFences[mCurrentFrame], VK_TRUE, UINT64_MAX);

		uint32_t imageIndex;
		vkAcquireNextImageKHR(
			mLogicDevice,
			mGraphicsPipeline.getSwapChain().get(),
			UINT64_MAX,
			mImageAvailableSemaphores[mCurrentFrame],
			VK_NULL_HANDLE,
			&imageIndex
		);

		//TODO:: Check result of vkAcquireNextImageKHR here

		updateUniformBuffer(imageIndex);

		//Check if a previous frame is using this image (i.e. there is its fence to wait on)
		if (mImageFencesInFlight[imageIndex] != VK_NULL_HANDLE) {
			vkWaitForFences(mLogicDevice, 1, &mImageFencesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
		}

		//Mark the image as now ben in use by this frame
		mImageFencesInFlight[imageIndex] = mInFlightFences[mCurrentFrame];

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkSemaphore waitSemaphores[] = {mImageAvailableSemaphores[mCurrentFrame]};
		VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &mGraphicsPipeline.getCommandBuffers()[imageIndex];//&mCommandBuffers[imageIndex];

		VkSemaphore signalSemaphores[] = {mRenderFinishedSemaphores[mCurrentFrame]};
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		vkResetFences(mLogicDevice, 1, &mInFlightFences[mCurrentFrame]);

		TRY(
			vkQueueSubmit(mGraphicsQueue, 1, &submitInfo, mInFlightFences[mCurrentFrame]) != VK_SUCCESS,
			"Failed to submit Draw Command Buffer."
		);

		VkPresentInfoKHR present = {};
		present.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		present.waitSemaphoreCount = 1;
		present.pWaitSemaphores = signalSemaphores;

		VkSwapchainKHR swapChains[] = {mGraphicsPipeline.getSwapChain().get()};
		present.swapchainCount = 1;
		present.pSwapchains = swapChains;
		present.pImageIndices = &imageIndex;

		vkQueuePresentKHR(mPresentQueue, &present);

		//TODO:: Check result of vkQueuePresentKHR here

		mCurrentFrame = (mCurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
	}

	void RenderingContextVulkan::updateUniformBuffer(uint32_t currentImage) {
		static auto startTime = std::chrono::high_resolution_clock::now();

		auto currentTime = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

		UniformBufferObject ubo = {};
		ubo.model = glm::rotate(
			glm::mat4(1.0f),
			time * glm::radians(90.0f),
			glm::vec3(0.0f, 0.0f, 1.0f)
		);
		ubo.view = glm::lookAt(
			glm::vec3(2.0f, 2.0f, 2.0f),
			glm::vec3(0.0f, 0.0f, 0.0f),
			glm::vec3(0.0f, 0.0f, 1.0f)
		);
		ubo.proj = glm::perspective(
			glm::radians(45.0f),
			mGraphicsPipeline.getSwapChain().getExtent().width /
				(float) mGraphicsPipeline.getSwapChain().getExtent().height,
			0.1f,
			10.0f
		);

		//NOTE:: GLM was designed for OpenGL where the y clip coord is inverted. This fixes it for Vulkan:
		ubo.proj[1][1] *= -1;

		mGraphicsPipeline.getUniformDescriptor().getBuffers()[currentImage].setData(mLogicDevice, &ubo, sizeof(ubo));
	}

	void RenderingContextVulkan::createQueues(QueueFamilyIndices& queueFamilyIndices) {
		vkGetDeviceQueue(mLogicDevice, queueFamilyIndices.graphicsFamily.value(), 0, &mGraphicsQueue);
		vkGetDeviceQueue(mLogicDevice, queueFamilyIndices.presentFamily.value(), 0, &mPresentQueue);
	}

	void RenderingContextVulkan::createCommandPool(QueueFamilyIndices& queueFamilyIndices) {
		VkCommandPoolCreateInfo cmdPoolCreateInfo = {};
		cmdPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		cmdPoolCreateInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

		TRY(
			vkCreateCommandPool(mLogicDevice, &cmdPoolCreateInfo, nullptr, &mCommandPool) != VK_SUCCESS,
			"Failed to create Command Pool."
		);
	}

	void RenderingContextVulkan::createDescriptorPool() {
		uint32_t imageCount = static_cast<uint32_t>(mGraphicsPipeline.getSwapChain().getImageCount());

		VkDescriptorPoolSize poolSize = {};
		poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSize.descriptorCount = imageCount;

		VkDescriptorPoolCreateInfo poolCreateInfo = {};
		poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolCreateInfo.poolSizeCount = 1;
		poolCreateInfo.pPoolSizes = &poolSize;
		poolCreateInfo.maxSets = imageCount;

		TRY(
			vkCreateDescriptorPool(mLogicDevice, &poolCreateInfo, nullptr, &mDescriptorPool) != VK_SUCCESS,
			"Failed to create descriptor pool."
		);
	}

	void RenderingContextVulkan::createSyncObjects() {
		mImageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		mRenderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		mInFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
		mImageFencesInFlight.resize(mGraphicsPipeline.getSwapChain().getImageCount(), VK_NULL_HANDLE);

		VkSemaphoreCreateInfo semaphore = {};
		semaphore.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fence = {};
		fence.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fence.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			TRY(
				vkCreateSemaphore(mLogicDevice, &semaphore, nullptr, &mImageAvailableSemaphores[i]) != VK_SUCCESS ||
				vkCreateSemaphore(mLogicDevice, &semaphore, nullptr, &mRenderFinishedSemaphores[i]) != VK_SUCCESS,
				"Failed to create Semaphores for a frame."
			);

			TRY(
				vkCreateFence(mLogicDevice, &fence, nullptr, &mInFlightFences[i]) != VK_SUCCESS,
				"Failed to create Fence."
			);
		}
	}

	void RenderingContextVulkan::preparePipeline() {
		createDescriptorPool();

		mGraphicsPipeline.setDescriptorPool(mDescriptorPool);
		mGraphicsPipeline.uploadShaderUBO(mLogicDevice, mPhysicalDevice);

		mGraphicsPipeline.createCommandBuffers(mLogicDevice, mVertexBuffer.getBuffer(), mIndexBuffer.getBuffer(), mIndexBuffer.getCount());
	}
}