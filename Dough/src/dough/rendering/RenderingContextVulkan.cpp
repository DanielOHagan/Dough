#include "dough/rendering/RenderingContextVulkan.h"

#include "dough/rendering/ObjInit.h"
#include "dough/rendering/shader/ShaderVulkan.h"

#include <chrono>
#include <glm/gtc/matrix_transform.hpp>

namespace DOH {

	RenderingContextVulkan::RenderingContextVulkan(VkDevice logicDevice, VkPhysicalDevice physicalDevice)
	:	mLogicDevice(logicDevice),
		mPhysicalDevice(physicalDevice),
		mGraphicsQueue(VK_NULL_HANDLE),
		mPresentQueue(VK_NULL_HANDLE),
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
		mPhysicalDeviceProperties = std::make_unique<VkPhysicalDeviceProperties>();
		vkGetPhysicalDeviceProperties(mPhysicalDevice, mPhysicalDeviceProperties.get());

		createQueues(queueFamilyIndices);

		createCommandPool(queueFamilyIndices);

		mTestTexture1 = ObjInit::texture(
			mLogicDevice,
			mPhysicalDevice,
			mCommandPool,
			mGraphicsQueue,
			*testTexturePath
		);
		mTestTexture2 = ObjInit::texture(
			mLogicDevice,
			mPhysicalDevice,
			mCommandPool,
			mGraphicsQueue,
			*testTexture2Path
		);

		mShaderProgram = ObjInit::shaderProgram(
			ObjInit::shader(mLogicDevice, EShaderType::VERTEX, *vertShaderPath),
			ObjInit::shader(mLogicDevice, EShaderType::FRAGMENT, *fragShaderPath)
		);

		ShaderUniformLayout& layout = mShaderProgram->getUniformLayout();
		layout.setValue(0, sizeof(UniformBufferObject));
		layout.setTexture(1, {mTestTexture1->getImageView(), mTestTexture1->getSampler()});

		//Create a vector of textures that are to be used in pipeline
		SwapChainCreationInfo swapChainCreate = {scSupport, surface, queueFamilyIndices, width, height};
		mGraphicsPipeline = ObjInit::graphicsPipeline(
			mLogicDevice,
			mCommandPool,
			swapChainCreate,
			*mShaderProgram
		);

		m_TestVAO_VertexArray = ObjInit::vertexArray();
		std::shared_ptr<VertexBufferVulkan> testVAO_VertexBuffer = ObjInit::stagedVertexBuffer(
			{
				{EDataType::FLOAT2, "mVertPos"},
				{EDataType::FLOAT3, "mColour"},
				{EDataType::FLOAT2, "mTexCoord"},
				{EDataType::FLOAT, "mTexIndex"}
			},
			mLogicDevice,
			mPhysicalDevice,
			mCommandPool,
			mGraphicsQueue,
			vertices.data(),
			sizeof(vertices[0]) * vertices.size(),
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		);
		m_TestVAO_VertexArray->addVertexBuffer(testVAO_VertexBuffer);
		std::shared_ptr<IndexBufferVulkan> testVAO_IndexBuffer = ObjInit::stagedIndexBuffer(
			mLogicDevice,
			mPhysicalDevice,
			mCommandPool,
			mGraphicsQueue,
			indices.data(),
			sizeof(indices[0]) * indices.size(),
			static_cast<uint32_t>(indices.size())
		);
		m_TestVAO_VertexArray->setIndexBuffer(testVAO_IndexBuffer);

		preparePipeline();

		createSyncObjects();
	}

	void RenderingContextVulkan::close() {
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			vkDestroySemaphore(mLogicDevice, mImageAvailableSemaphores[i], nullptr);
			vkDestroySemaphore(mLogicDevice, mRenderFinishedSemaphores[i], nullptr);
			vkDestroyFence(mLogicDevice, mInFlightFences[i], nullptr);
		}

		m_TestVAO_VertexArray->close(mLogicDevice);

		mTestTexture1->close(mLogicDevice);
		mTestTexture2->close(mLogicDevice);

		mShaderProgram->close(mLogicDevice);
		
		mGraphicsPipeline->close(mLogicDevice);

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
		if (mGraphicsPipeline->getSwapChain().isResizable()) {
			vkDeviceWaitIdle(mLogicDevice);

			mGraphicsPipeline->close(mLogicDevice);
			
			/**
			* !! IMPORTNANT !! 
			*	I think a memory leak is happening during swap chain recreation.
			* 
			* Things to check:
			*	ALL .close()
			*	shader path strings
			* 
			*/
			SwapChainCreationInfo swapChainCreate = {scSupport, surface, queueFamilyIndices, width, height};
			mGraphicsPipeline = ObjInit::graphicsPipeline(
				mLogicDevice,
				mCommandPool,
				swapChainCreate,
				*mShaderProgram
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
			mGraphicsPipeline->getSwapChain().get(),
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
		submitInfo.pCommandBuffers = &mGraphicsPipeline->getCommandBuffers()[imageIndex];//&mCommandBuffers[imageIndex];

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

		VkSwapchainKHR swapChains[] = {mGraphicsPipeline->getSwapChain().get()};
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
		float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count() / 4; //Divide to slow rotation speed
		const float aspectRatio = 
			(float) mGraphicsPipeline->getSwapChain().getExtent().width /
			mGraphicsPipeline->getSwapChain().getExtent().height;

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
			glm::radians(90.0f),
			aspectRatio,
			0.1f,
			10.0f
		);

		//NOTE:: GLM was designed for OpenGL where the y clip coord is inverted. This fixes it for Vulkan:
		ubo.proj[1][1] *= -1;

		//TODO:: make uniform assignment easier:
		//	shaderDescriptor.setData(mLogicDevice, binding OR "uniformName", &ubo, sizeof(ubo));
		const uint32_t uboBinding = 0;
		mGraphicsPipeline->getShaderDescriptor().getBuffersFromBinding(uboBinding)[currentImage]->
			setData(mLogicDevice, &ubo, sizeof(ubo));
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
		uint32_t imageCount = static_cast<uint32_t>(mGraphicsPipeline->getSwapChain().getImageCount());

		std::array<VkDescriptorPoolSize, 2> poolSizes{};
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[0].descriptorCount = imageCount;
		poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[1].descriptorCount = imageCount;

		VkDescriptorPoolCreateInfo poolCreateInfo = {};
		poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolCreateInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		poolCreateInfo.pPoolSizes = poolSizes.data();
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
		mImageFencesInFlight.resize(mGraphicsPipeline->getSwapChain().getImageCount(), VK_NULL_HANDLE);

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

		mGraphicsPipeline->setDescriptorPool(mDescriptorPool);
		mGraphicsPipeline->uploadShaderUniforms(mLogicDevice, mPhysicalDevice);

		mGraphicsPipeline->createCommandBuffers(mLogicDevice, *m_TestVAO_VertexArray);
	}

	VkCommandBuffer RenderingContextVulkan::beginSingleTimeCommands() {
		VkCommandBufferAllocateInfo allocation{};
		allocation.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocation.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocation.commandPool = mCommandPool;
		allocation.commandBufferCount = 1;

		VkCommandBuffer cmdBuffer;
		vkAllocateCommandBuffers(mLogicDevice, &allocation, &cmdBuffer);

		VkCommandBufferBeginInfo begin{};
		begin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		begin.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(cmdBuffer, &begin);

		return cmdBuffer;
	}

	void RenderingContextVulkan::endSingleTimeCommands(VkCommandBuffer cmdBuffer) {
		vkEndCommandBuffer(cmdBuffer);

		VkSubmitInfo submit{};
		submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit.commandBufferCount = 1;
		submit.pCommandBuffers = &cmdBuffer;

		vkQueueSubmit(mGraphicsQueue, 1, &submit, VK_NULL_HANDLE);
		vkQueueWaitIdle(mGraphicsQueue);

		vkFreeCommandBuffers(mLogicDevice, mCommandPool, 1, &cmdBuffer);
	}

	void RenderingContextVulkan::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
		VkCommandBuffer cmdBuffer = beginSingleTimeCommands();

		VkBufferImageCopy region{};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;
		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;
		region.imageOffset = {0, 0, 0};
		region.imageExtent = {
			width,
			height,
			1
		};

		vkCmdCopyBufferToImage(
			cmdBuffer,
			buffer,
			image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1,
			&region
		);

		endSingleTimeCommands(cmdBuffer);
	}

	VkImageView RenderingContextVulkan::createImageView(VkImage image, VkFormat format) {
		VkImageViewCreateInfo view{};
		view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		view.image = image;
		view.viewType = VK_IMAGE_VIEW_TYPE_2D;
		view.format = format;
		view.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		view.subresourceRange.baseMipLevel = 0;
		view.subresourceRange.levelCount = 1;
		view.subresourceRange.baseArrayLayer = 0;
		view.subresourceRange.layerCount = 1;

		VkImageView imageView;
		TRY(
			vkCreateImageView(mLogicDevice, &view, nullptr, &imageView) != VK_SUCCESS,
			"Failed to create image view"
		);
		return imageView;
	}

	VkSampler RenderingContextVulkan::createSampler() {
		VkSamplerCreateInfo samplerCreate{};
		samplerCreate.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerCreate.magFilter = VK_FILTER_LINEAR;
		samplerCreate.minFilter = VK_FILTER_LINEAR;
		samplerCreate.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerCreate.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerCreate.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerCreate.anisotropyEnable = VK_TRUE;
		samplerCreate.maxAnisotropy = mPhysicalDeviceProperties->limits.maxSamplerAnisotropy; //TODO:: Make this a variable instead of always max possible value device allows
		samplerCreate.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerCreate.unnormalizedCoordinates = VK_FALSE;
		samplerCreate.compareEnable = VK_FALSE;
		samplerCreate.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerCreate.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerCreate.mipLodBias = 0.0f;
		samplerCreate.minLod = 0.0f;
		samplerCreate.maxLod = 0.0f;

		VkSampler sampler;
		TRY(
			vkCreateSampler(mLogicDevice, &samplerCreate, nullptr, &sampler),
			"Failed to create sampler"
		);
		return sampler;
	}

	void RenderingContextVulkan::setPhysicalDevice(VkPhysicalDevice physicalDevice) {
		mPhysicalDevice = physicalDevice;
		vkGetPhysicalDeviceProperties(mPhysicalDevice, mPhysicalDeviceProperties.get());
	}
}
