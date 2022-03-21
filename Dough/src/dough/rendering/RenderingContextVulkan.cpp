#include "dough/rendering/RenderingContextVulkan.h"

#include "dough/rendering/ObjInit.h"
#include "dough/Logging.h"

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
		mUbo({ glm::mat4x4(1.0f) }),
		
		//TEMP:: Hard coded values until custom UI pipeline is available
		mAppUiProjection(glm::ortho(-1920.0f / 1080.0f, 1920.0f / 1080.0f, -1.0f, 1.0f, -1.0f, 1.0f))
	{
		//Invert z clip
		mAppUiProjection[1][1] *= -1;
	}

	bool RenderingContextVulkan::isReady() const {
		if (mSceneGraphicsPipeline == nullptr) {
			LOG_ERR(VAR_NAME(mSceneGraphicsPipeline) << " null on ready check");
			return false;
		}
		if (mAppUiGraphicsPipeline == nullptr) {
			LOG_ERR(VAR_NAME(mAppUiGraphicsPipeline) << " null on ready check");
			return false;
		}
		if (mPhysicalDeviceProperties == nullptr) {
			LOG_ERR(VAR_NAME(mPhysicalDeviceProperties) << " null on ready check");
			return false;
		}
		if (mSwapChainCreationInfo == nullptr) {
			LOG_ERR(VAR_NAME(mPhysicalDeviceProperties) << " null on ready check");
			return false;
		}
		if (mSwapChain == nullptr) {
			LOG_ERR(VAR_NAME(mSwapChain) << " null on ready check");
			return false;
		}

		return true;
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

		mSwapChainCreationInfo = std::make_unique<SwapChainCreationInfo>(scSupport, surface, queueFamilyIndices);
		mSwapChain = ObjInit::swapChain(*mSwapChainCreationInfo);

		createCommandBuffers();
		createSyncObjects();

		//TODO:: allow for custom UI pipeline & shader
		mAppUiShaderProgram = ObjInit::shaderProgram(
			ObjInit::shader(EShaderType::VERTEX, *mAppUiShaderVertPath),
			ObjInit::shader(EShaderType::FRAGMENT, *mAppUiShaderFragPath)
		);
		mAppUiShaderProgram->getUniformLayout().setValue(0, sizeof(UniformBufferObject));

		mAppUiVao = ObjInit::vertexArray();
		std::shared_ptr<VertexBufferVulkan> appUiVb = ObjInit::stagedVertexBuffer(
			{
				{EDataType::FLOAT3, "mVertPos"},
				{EDataType::FLOAT3, "mColour"}
			},
			mAppUiVertices.data(),
			sizeof(mAppUiVertices[0]) * mAppUiVertices.size(),
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		);
		mAppUiVao->addVertexBuffer(appUiVb);
		std::shared_ptr<IndexBufferVulkan> appUiIb = ObjInit::stagedIndexBuffer(
			mAppUiIndices.data(),
			sizeof(mAppUiIndices[0]) * mAppUiIndices.size(),
			static_cast<uint32_t>(mAppUiIndices.size())
		);
		mAppUiVao->setIndexBuffer(appUiIb);

		prepareAppUiPipeline(false);
	}

	void RenderingContextVulkan::close() {
		//Remove any app resources added after last frame was presented
		for (IGPUResourceVulkan& res : mToReleaseGpuResources) {
			res.close(mLogicDevice);
		}

		vkFreeCommandBuffers(
			mLogicDevice,
			mCommandPool,
			static_cast<uint32_t>(mCommandBuffers.size()),
			mCommandBuffers.data()
		);

		closeSyncObjects();
		mSwapChain->close(mLogicDevice);
		mSceneGraphicsPipeline->close(mLogicDevice);

		mAppUiShaderProgram->close(mLogicDevice);
		mAppUiGraphicsPipeline->close(mLogicDevice);
		mAppUiVao->close(mLogicDevice);

		vkDestroyCommandPool(mLogicDevice, mCommandPool, nullptr);
		vkDestroyDescriptorPool(mLogicDevice, mDescriptorPool, nullptr);
	}

	/**
	* Prepare renderer for work after App has defined a scene and UI pipeline
	*/
	void RenderingContextVulkan::setupPostAppLogicInit() {
		std::vector<VkDescriptorType> descTypes;
		for (VkDescriptorType descType : mSceneGraphicsPipeline->getShaderProgram().getUniformLayout().asDescriptorTypes()) {
			descTypes.push_back(descType);
		}
		for (VkDescriptorType descType : mAppUiGraphicsPipeline->getShaderProgram().getUniformLayout().asDescriptorTypes()) {
			descTypes.push_back(descType);
		}
		createDescriptorPool(descTypes);
		createPipelineUniformObjects(*mSceneGraphicsPipeline);
		createPipelineUniformObjects(*mAppUiGraphicsPipeline);
	}

	void RenderingContextVulkan::updateUiProjectionMatrix(float aspectRatio) {
		mAppUiProjection = glm::ortho(-aspectRatio, aspectRatio, -1.0f, 1.0f, -1.0f, 1.0f);
		mAppUiProjection[1][1] *= -1;
	}

	void RenderingContextVulkan::resizeSwapChain(
		SwapChainSupportDetails& scSupport,
		VkSurfaceKHR surface,
		QueueFamilyIndices& queueFamilyIndices,
		uint32_t width,
		uint32_t height
	) {
		if (mSwapChain->isResizable()) {
			VK_TRY(
				vkDeviceWaitIdle(mLogicDevice),
				"Device failed to wait idle for swap chain recreation"
			)

			ShaderProgramVulkan& sceneShaderProgram = mSceneGraphicsPipeline->getShaderProgram();
			sceneShaderProgram.closePipelineSpecificObjects(mLogicDevice);
			
			mSceneGraphicsPipeline->close(mLogicDevice);
			mAppUiGraphicsPipeline->close(mLogicDevice);
			mSwapChain->close(mLogicDevice);

			ShaderProgramVulkan& appUiShaderProgram = mAppUiGraphicsPipeline->getShaderProgram();
			appUiShaderProgram.closePipelineSpecificObjects(mLogicDevice);

			vkDestroyDescriptorPool(mLogicDevice, mDescriptorPool, nullptr);

			mSwapChainCreationInfo->setWidth(width);
			mSwapChainCreationInfo->setHeight(height);
			mSwapChain = ObjInit::swapChain(*mSwapChainCreationInfo);

			std::vector<VkDescriptorType> descTypes;
			for (auto& descType : sceneShaderProgram.getUniformLayout().asDescriptorTypes()) {
				descTypes.push_back(descType);
			}
			for (auto& descType : appUiShaderProgram.getUniformLayout().asDescriptorTypes()) {
				descTypes.push_back(descType);
			}
			createDescriptorPool(descTypes);

			prepareScenePipeline(sceneShaderProgram, true);
			prepareAppUiPipeline(true);
		}
	}

	//Draw then Present rendered frame
	void RenderingContextVulkan::drawFrame() {
		uint32_t imageIndex = mSwapChain->aquireNextImageIndex(
			mLogicDevice,
			mFramesInFlightFences[mCurrentFrame],
			mImageAvailableSemaphores[mCurrentFrame]
		);

		VkCommandBuffer cmd = mCommandBuffers[imageIndex];
		beginCommandBuffer(cmd);
		
		//Draw scene
		mSwapChain->beginRenderPass(SwapChainVulkan::RenderPassType::SCENE, imageIndex, cmd);
		mSceneGraphicsPipeline->recordDrawCommands(imageIndex, cmd);
		mSwapChain->endRenderPass(cmd);

		updateUniformBufferObject(imageIndex);

		//Draw Application UI
		mSwapChain->beginRenderPass(SwapChainVulkan::RenderPassType::APP_UI, imageIndex, cmd);
		mAppUiGraphicsPipeline->addVaoToDraw(*mAppUiVao);
		mAppUiGraphicsPipeline->recordDrawCommands(imageIndex, cmd);
		mSwapChain->endRenderPass(cmd);

		//Clear each frame
		mSceneGraphicsPipeline->clearVaoToDraw();
		mAppUiGraphicsPipeline->clearVaoToDraw();

		endCommandBuffer(cmd);

		present(imageIndex, cmd);

		//TODO::Make this so it is only removing resources from frames that are NO LONGER in use
		//Release GPU resources of no longer in use app data
		for (IGPUResourceVulkan& res : mToReleaseGpuResources) {
			res.close(mLogicDevice);
		}
		mToReleaseGpuResources.clear();
	}

	void RenderingContextVulkan::present(uint32_t imageIndex, VkCommandBuffer cmd) {
		if (mImageFencesInFlight[imageIndex] != VK_NULL_HANDLE) {
			vkWaitForFences(mLogicDevice, 1, &mImageFencesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
		}

		//Mark the image as now being in use by this frame
		mImageFencesInFlight[imageIndex] = mFramesInFlightFences[mCurrentFrame];

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkSemaphore waitSemaphores[] = { mImageAvailableSemaphores[mCurrentFrame] };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &cmd;

		VkSemaphore signalSemaphores[] = { mRenderFinishedSemaphores[mCurrentFrame] };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		vkResetFences(mLogicDevice, 1, &mFramesInFlightFences[mCurrentFrame]);

		VK_TRY(
			vkQueueSubmit(mGraphicsQueue, 1, &submitInfo, mFramesInFlightFences[mCurrentFrame]),
			"Failed to submit Draw Command Buffer."
		);

		VkPresentInfoKHR present = {};
		present.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		present.waitSemaphoreCount = 1;
		present.pWaitSemaphores = signalSemaphores;

		VkSwapchainKHR swapChains[] = { mSwapChain->get() };
		present.swapchainCount = 1;
		present.pSwapchains = swapChains;
		present.pImageIndices = &imageIndex;

		vkQueuePresentKHR(mPresentQueue, &present);

		//TODO:: Check result of vkQueuePresentKHR here

		mCurrentFrame = (mCurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

	}

	void RenderingContextVulkan::updateUniformBufferObject(uint32_t currentImage) {

		//NOTE:: Current the UBO only uses the camera's projection view matrix so updating here works,
		const uint32_t uboBinding = 0;
		mSceneGraphicsPipeline->getShaderDescriptor().getBuffersFromBinding(uboBinding)[currentImage]->
			setData(mLogicDevice, &mUbo.ProjectionViewMat, sizeof(mUbo.ProjectionViewMat));

		mAppUiShaderProgram->getShaderDescriptor().getBuffersFromBinding(0)[currentImage]
			->setData(mLogicDevice, &mAppUiProjection, sizeof(glm::mat4x4));
	}

	void RenderingContextVulkan::createQueues(QueueFamilyIndices& queueFamilyIndices) {
		vkGetDeviceQueue(mLogicDevice, queueFamilyIndices.graphicsFamily.value(), 0, &mGraphicsQueue);
		vkGetDeviceQueue(mLogicDevice, queueFamilyIndices.presentFamily.value(), 0, &mPresentQueue);
	}

	void RenderingContextVulkan::createCommandPool(QueueFamilyIndices& queueFamilyIndices) {
		VkCommandPoolCreateInfo cmdPoolCreateInfo = {};
		cmdPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		cmdPoolCreateInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
		cmdPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

		VK_TRY(
			vkCreateCommandPool(mLogicDevice, &cmdPoolCreateInfo, nullptr, &mCommandPool),
			"Failed to create Command Pool."
		);
	}

	void RenderingContextVulkan::createCommandBuffers() {
		mCommandBuffers.resize(mSwapChain->getFrameBufferCount());

		VkCommandBufferAllocateInfo cmdBuffAlloc = {};
		cmdBuffAlloc.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		cmdBuffAlloc.commandPool = mCommandPool;
		cmdBuffAlloc.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		cmdBuffAlloc.commandBufferCount = (uint32_t)mCommandBuffers.size();

		VK_TRY(
			vkAllocateCommandBuffers(mLogicDevice, &cmdBuffAlloc, mCommandBuffers.data()),
			"Failed to allocate Command Buffers."
		);
	}

	void RenderingContextVulkan::createDescriptorPool(std::vector<VkDescriptorType>& descTypes) {
		uint32_t imageCount = static_cast<uint32_t>(mSwapChain->getImageCount());

		std::vector<VkDescriptorPoolSize> poolSizes;
		for (VkDescriptorType descType : descTypes) {
			poolSizes.push_back({ descType, imageCount });
		}

		const uint32_t poolSizeCount = static_cast<uint32_t>(poolSizes.size());

		VkDescriptorPoolCreateInfo poolCreateInfo = {};
		poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolCreateInfo.poolSizeCount = poolSizeCount;
		poolCreateInfo.pPoolSizes = poolSizes.data();
		poolCreateInfo.maxSets = imageCount * poolSizeCount;

		VK_TRY(
			vkCreateDescriptorPool(mLogicDevice, &poolCreateInfo, nullptr, &mDescriptorPool),
			"Failed to create descriptor pool."
		);
	}

	void RenderingContextVulkan::createSyncObjects() {
		mImageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		mRenderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		mFramesInFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
		mImageFencesInFlight.resize(mSwapChain->getImageCount(), VK_NULL_HANDLE);

		VkSemaphoreCreateInfo semaphore = {};
		semaphore.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fence = {};
		fence.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fence.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			VK_TRY(
				vkCreateSemaphore(mLogicDevice, &semaphore, nullptr, &mImageAvailableSemaphores[i]),
				"Failed to create image available semaphores for a frame"
			);
			VK_TRY(
				vkCreateSemaphore(mLogicDevice, &semaphore, nullptr, &mRenderFinishedSemaphores[i]),
				"Failed to create render finished semaphores for a frame"
			);
			VK_TRY(
				vkCreateFence(mLogicDevice, &fence, nullptr, &mFramesInFlightFences[i]) != VK_SUCCESS,
				"Failed to create frame in flight fence"
			);
		}
	}

	void RenderingContextVulkan::closeSyncObjects() {
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			vkDestroySemaphore(mLogicDevice, mImageAvailableSemaphores[i], nullptr);
			vkDestroySemaphore(mLogicDevice, mRenderFinishedSemaphores[i], nullptr);
			vkDestroyFence(mLogicDevice, mFramesInFlightFences[i], nullptr);
		}
	}

	//TODO:: Make Vertex input more dynamic and allow application to set it
	void RenderingContextVulkan::prepareScenePipeline(ShaderProgramVulkan& shaderProgram, bool createUniformObjects) {
		std::vector<VkVertexInputAttributeDescription> attribDesc = Vertex::getAttributeDescriptions();
		mSceneGraphicsPipeline = ObjInit::graphicsPipeline(
			mSwapChain->getExtent(),
			mSwapChain->getRenderPass(SwapChainVulkan::RenderPassType::SCENE).get(),
			shaderProgram,
			Vertex::getBindingDescription(), //TODO:: shaderProgram.getBindingDescription();
			attribDesc //TODO:: shaderProgram.getAttributeDescription()
		);

		if (createUniformObjects) {
			createPipelineUniformObjects(*mSceneGraphicsPipeline);
		}
	}

	void RenderingContextVulkan::prepareAppUiPipeline(bool createUniformObjects) {
		std::vector<VkVertexInputAttributeDescription> attribDesc = std::move(VertexUi2D::getAttributeDescriptions());
		mAppUiGraphicsPipeline = ObjInit::graphicsPipeline(
			mSwapChain->getExtent(),
			mSwapChain->getRenderPass(SwapChainVulkan::RenderPassType::APP_UI).get(),
			*mAppUiShaderProgram,
			VertexUi2D::getBindingDescription(),
			attribDesc
		);

		if (createUniformObjects) {
			createPipelineUniformObjects(*mAppUiGraphicsPipeline);
		}
	}

	void RenderingContextVulkan::createPipelineUniformObjects(GraphicsPipelineVulkan& pipeline) {
		if (pipeline.getShaderProgram().getUniformLayout().hasUniforms()) {
			pipeline.setDescriptorPool(mDescriptorPool);
			pipeline.uploadShaderUniforms(
				mLogicDevice,
				mPhysicalDevice,
				static_cast<uint32_t>(mSwapChain->getImageCount())
			);
		} else {
			LOG_WARN("Tried to create uniform objects for pipeline without uniforms");
		}
	}

	void RenderingContextVulkan::addVaoToDraw(VertexArrayVulkan& vao) {
		mSceneGraphicsPipeline->addVaoToDraw(vao);
	}

	VkCommandBuffer RenderingContextVulkan::beginSingleTimeCommands() {
		VkCommandBufferAllocateInfo allocation{};
		allocation.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocation.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocation.commandPool = mCommandPool;
		allocation.commandBufferCount = 1;

		VkCommandBuffer cmdBuffer;
		VK_TRY(
			vkAllocateCommandBuffers(mLogicDevice, &allocation, &cmdBuffer),
			"Failed to allocate single time command buffer"
		);

		beginCommandBuffer(cmdBuffer, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

		return cmdBuffer;
	}

	void RenderingContextVulkan::endSingleTimeCommands(VkCommandBuffer cmdBuffer) {
		vkEndCommandBuffer(cmdBuffer);

		VkSubmitInfo submit{};
		submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit.commandBufferCount = 1;
		submit.pCommandBuffers = &cmdBuffer;

		VK_TRY(
			vkQueueSubmit(mGraphicsQueue, 1, &submit, VK_NULL_HANDLE),
			"Failed to submit single time command to graphics queue"
		);

		VK_TRY(
			vkQueueWaitIdle(mGraphicsQueue),
			"Failed to wait on graphics queue for single time command"
		);

		vkFreeCommandBuffers(mLogicDevice, mCommandPool, 1, &cmdBuffer);
	}

	void RenderingContextVulkan::beginCommandBuffer(VkCommandBuffer cmd, VkCommandBufferUsageFlags usage) {
		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = usage;

		VK_TRY(
			vkBeginCommandBuffer(cmd, &beginInfo),
			"Failed to begin recording Command Buffer."
		);
	}

	void RenderingContextVulkan::endCommandBuffer(VkCommandBuffer cmd) {
		VK_TRY(
			vkEndCommandBuffer(cmd),
			"Failed to end command buffer"
		);
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

	//TODO:: contains a single time command that executes on function end
	void RenderingContextVulkan::transitionImageLayout(
		VkDevice logicDevice,
		VkImage image,
		VkImageLayout oldLayout,
		VkImageLayout newLayout,
		VkImageAspectFlags aspectFlags
	) {
		VkCommandBuffer cmdBuffer = beginSingleTimeCommands();

		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = image;
		barrier.subresourceRange.aspectMask = aspectFlags;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		VkPipelineStageFlags srcStage;
		VkPipelineStageFlags dstStage;

		if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		} else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		} else {
			THROW("Layout transition not supported");
		}

		vkCmdPipelineBarrier(
			cmdBuffer,
			srcStage,
			dstStage,
			0,
			0,
			nullptr,
			0,
			nullptr,
			1,
			&barrier
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
		VK_TRY(
			vkCreateImageView(mLogicDevice, &view, nullptr, &imageView),
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
		VK_TRY(
			vkCreateSampler(mLogicDevice, &samplerCreate, nullptr, &sampler),
			"Failed to create sampler"
		);
		return sampler;
	}

	void RenderingContextVulkan::setPhysicalDevice(VkPhysicalDevice physicalDevice) {
		mPhysicalDevice = physicalDevice;
		vkGetPhysicalDeviceProperties(mPhysicalDevice, mPhysicalDeviceProperties.get());
	}

	std::shared_ptr<GraphicsPipelineVulkan> RenderingContextVulkan::createGraphicsPipeline(
		VkExtent2D extent,
		VkRenderPass renderPass,
		ShaderProgramVulkan& shaderProgram,
		VkVertexInputBindingDescription vertexInputBindingDesc,
		std::vector<VkVertexInputAttributeDescription>& vertexAttributes
	) {
		return std::make_shared<GraphicsPipelineVulkan>(
			mLogicDevice,
			mCommandPool,
			extent,
			renderPass,
			shaderProgram,
			vertexInputBindingDesc,
			vertexAttributes
		);
	}

	std::shared_ptr<SwapChainVulkan> RenderingContextVulkan::createSwapChain(SwapChainCreationInfo& swapChainCreate) {
		return std::make_shared<SwapChainVulkan>(mLogicDevice, swapChainCreate);
	}

	std::shared_ptr<RenderPassVulkan> RenderingContextVulkan::createRenderPass(
		VkFormat imageFormat,
		bool hasPassBefore,
		bool hasPassAfter,
		bool enableClearColour,
		VkClearValue clearColour = { 0.264f, 0.328f, 0.484f, 1.0f }
	) {
		return std::make_shared<RenderPassVulkan>(
			mLogicDevice,
			imageFormat,
			hasPassBefore,
			hasPassAfter,
			enableClearColour,
			clearColour
		);
	}

	std::shared_ptr<VertexArrayVulkan> RenderingContextVulkan::createVertexArray() {
		return std::make_shared<VertexArrayVulkan>();
	}

	std::shared_ptr<VertexBufferVulkan> RenderingContextVulkan::createVertexBuffer(
		const std::initializer_list<BufferElement>& elements,
		VkDeviceSize size,
		VkBufferUsageFlags usage,
		VkMemoryPropertyFlags props
	) {
		return std::make_shared<VertexBufferVulkan>(elements, mLogicDevice, mPhysicalDevice, size, usage, props);
	}

	std::shared_ptr<VertexBufferVulkan> RenderingContextVulkan::createStagedVertexBuffer(
		const std::initializer_list<BufferElement>& elements,
		void* data,
		VkDeviceSize size,
		VkBufferUsageFlags usage,
		VkMemoryPropertyFlags props
	) {
		return std::make_shared<VertexBufferVulkan>(elements, mLogicDevice, mPhysicalDevice, mCommandPool, mGraphicsQueue, (const void*) data, size, usage, props);
	}

	std::shared_ptr<VertexBufferVulkan> RenderingContextVulkan::createStagedVertexBuffer(
		const std::initializer_list<BufferElement>& elements,
		const void* data,
		VkDeviceSize size,
		VkBufferUsageFlags usage,
		VkMemoryPropertyFlags props
	) {
		return std::make_shared<VertexBufferVulkan>(elements, mLogicDevice, mPhysicalDevice, mCommandPool, mGraphicsQueue, data, size, usage, props);
	}

	std::shared_ptr<IndexBufferVulkan> RenderingContextVulkan::createIndexBuffer(VkDeviceSize size, uint32_t count) {
		return std::make_shared<IndexBufferVulkan>(mLogicDevice, mPhysicalDevice, size, count);
	}

	std::shared_ptr<IndexBufferVulkan> RenderingContextVulkan::createStagedIndexBuffer(void* data, VkDeviceSize size, uint32_t count) {
		return std::make_shared<IndexBufferVulkan>(mLogicDevice, mPhysicalDevice, mCommandPool, mGraphicsQueue, (const void*) data, size, count);
	}

	std::shared_ptr<IndexBufferVulkan> RenderingContextVulkan::createStagedIndexBuffer(const void* data, VkDeviceSize size, uint32_t count) {
		return std::make_shared<IndexBufferVulkan>(mLogicDevice, mPhysicalDevice, mCommandPool, mGraphicsQueue, data, size, count);
	}

	std::shared_ptr<BufferVulkan> RenderingContextVulkan::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags props) {
		return std::make_shared<BufferVulkan>(mLogicDevice, mPhysicalDevice, size, usage, props);
	}

	std::shared_ptr<BufferVulkan> RenderingContextVulkan::createStagedBuffer(void* data, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags props) {
		return std::make_shared<BufferVulkan>(mLogicDevice, mPhysicalDevice, mCommandPool, mGraphicsQueue, (const void*) data, size, usage, props);
	}

	std::shared_ptr<BufferVulkan> RenderingContextVulkan::createStagedBuffer(const void* data, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags props) {
		return std::make_shared<BufferVulkan>(mLogicDevice, mPhysicalDevice, mCommandPool, mGraphicsQueue, data, size, usage, props);
	}

	std::shared_ptr<ShaderProgramVulkan> RenderingContextVulkan::createShaderProgram(std::shared_ptr<ShaderVulkan> vertShader, std::shared_ptr<ShaderVulkan> fragShader) {
		return std::make_shared<ShaderProgramVulkan>(vertShader, fragShader);
	}

	std::shared_ptr<ShaderVulkan> RenderingContextVulkan::createShader(EShaderType type, std::string& filePath) {
		return std::make_shared<ShaderVulkan>(type, filePath);
	}

	std::shared_ptr<TextureVulkan> RenderingContextVulkan::createTexture(std::string& filePath) {
		return std::make_shared<TextureVulkan>(mLogicDevice, mPhysicalDevice, mCommandPool, mGraphicsQueue, filePath);
	}
}
