#include "dough/rendering/RenderingContextVulkan.h"
#include "dough/rendering/RendererVulkan.h"
#include "dough/rendering/renderer2d/Renderer2dVulkan.h"
#include "dough/rendering/ObjInit.h"
#include "dough/Logging.h"

#include <chrono>
#include <glm/gtc/matrix_transform.hpp>

#define CLOSE_NOT_NULL(objPtr) if (objPtr != nullptr) {objPtr->close(mLogicDevice);}
#define NOT_NULL_CHECK(objPtr) if (objPtr == nullptr) {\
	LOG_ERR(VAR_NAME(objPtr) << " null on ready check");\
	return false;\
}

namespace DOH {

	RenderingContextVulkan::RenderingContextVulkan(VkDevice logicDevice, VkPhysicalDevice physicalDevice)
	:	mLogicDevice(logicDevice),
		mPhysicalDevice(physicalDevice),
		mGraphicsQueue(VK_NULL_HANDLE),
		mPresentQueue(VK_NULL_HANDLE),
		mDescriptorPool(VK_NULL_HANDLE),
		mCommandPool(VK_NULL_HANDLE),
		mSceneUbo({ glm::mat4x4(1.0f) }),
		mAppUiProjection(1.0f)
	{}

	bool RenderingContextVulkan::isReady() const {
		NOT_NULL_CHECK(mSceneGraphicsPipeline);
		NOT_NULL_CHECK(mAppUiGraphicsPipeline);
		NOT_NULL_CHECK(mPhysicalDeviceProperties);
		NOT_NULL_CHECK(mSwapChainCreationInfo);
		NOT_NULL_CHECK(mSwapChain);

		return true;
	}

	void RenderingContextVulkan::init(
		SwapChainSupportDetails& scSupport,
		VkSurfaceKHR surface,
		QueueFamilyIndices& queueFamilyIndices,
		Window& window,
		VkInstance vulkanInstance
	) {
		mPhysicalDeviceProperties = std::make_unique<VkPhysicalDeviceProperties>();
		vkGetPhysicalDeviceProperties(mPhysicalDevice, mPhysicalDeviceProperties.get());

		createQueues(queueFamilyIndices);

		createCommandPool(queueFamilyIndices);

		mSwapChainCreationInfo = std::make_unique<SwapChainCreationInfo>(scSupport, surface, queueFamilyIndices);
		mSwapChain = ObjInit::swapChain(*mSwapChainCreationInfo);

		createCommandBuffers();
		createSyncObjects();

		mRenderer2d = std::make_unique<Renderer2dVulkan>(*this);
		mRenderer2d->init(mLogicDevice);

		mImGuiWrapper = std::make_unique<ImGuiWrapper>();
		ImGuiInitInfo imGuiInitInfo = {};
		imGuiInitInfo.ImageCount = static_cast<uint32_t>(mSwapChain->getImageCount());
		imGuiInitInfo.MinImageCount = 2;
		imGuiInitInfo.LogicDevice = mLogicDevice;
		imGuiInitInfo.PhysicalDevice = mPhysicalDevice;
		imGuiInitInfo.Queue = mGraphicsQueue;
		imGuiInitInfo.QueueFamily = VK_QUEUE_GRAPHICS_BIT;
		imGuiInitInfo.UiRenderPass = mSwapChain->getRenderPass(SwapChainVulkan::ERenderPassType::APP_UI).get();
		imGuiInitInfo.VulkanInstance = vulkanInstance;

		mImGuiWrapper->init(window, imGuiInitInfo);
		mImGuiWrapper->uploadFonts(*this);
	}

	void RenderingContextVulkan::close() {
		releaseGpuResources();

		vkFreeCommandBuffers(
			mLogicDevice,
			mCommandPool,
			static_cast<uint32_t>(mCommandBuffers.size()),
			mCommandBuffers.data()
		);

		CLOSE_NOT_NULL(mRenderer2d);
		CLOSE_NOT_NULL(mImGuiWrapper);

		closeSyncObjects();

		CLOSE_NOT_NULL(mSwapChain);
		CLOSE_NOT_NULL(mSceneGraphicsPipeline);
		CLOSE_NOT_NULL(mAppUiGraphicsPipeline);

		vkDestroyCommandPool(mLogicDevice, mCommandPool, nullptr);
		vkDestroyDescriptorPool(mLogicDevice, mDescriptorPool, nullptr);
	}

	void RenderingContextVulkan::releaseGpuResources() {
		for (std::shared_ptr<IGPUResourceVulkan> res : mToReleaseGpuResources) {
			res->close(mLogicDevice);
		}
		mToReleaseGpuResources.clear();
	}

	/**
	* Prepare renderer for work after App has defined a scene and UI pipeline
	*/
	void RenderingContextVulkan::setupPostAppLogicInit() {
		std::vector<DescriptorTypeInfo> descTypes;
		for (DescriptorTypeInfo& descType : mSceneGraphicsPipeline->getShaderProgram().getUniformLayout().asDescriptorTypes()) {
			descTypes.push_back(descType);
		}
		for (DescriptorTypeInfo& descType : mAppUiGraphicsPipeline->getShaderProgram().getUniformLayout().asDescriptorTypes()) {
			descTypes.push_back(descType);
		}
		mDescriptorPool = createDescriptorPool(descTypes, 2);
		createPipelineUniformObjects(*mSceneGraphicsPipeline, mDescriptorPool);
		createPipelineUniformObjects(*mAppUiGraphicsPipeline, mDescriptorPool);
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
			);

			//Close unusable objects
			ShaderProgramVulkan& sceneShaderProgram = mSceneGraphicsPipeline->getShaderProgram();
			sceneShaderProgram.closePipelineSpecificObjects(mLogicDevice);
			ShaderProgramVulkan& appUiShaderProgram = mAppUiGraphicsPipeline->getShaderProgram();
			appUiShaderProgram.closePipelineSpecificObjects(mLogicDevice);
			
			CLOSE_NOT_NULL(mSceneGraphicsPipeline)
			CLOSE_NOT_NULL(mAppUiGraphicsPipeline);
			CLOSE_NOT_NULL(mSwapChain);

			vkDestroyDescriptorPool(mLogicDevice, mDescriptorPool, nullptr);

			mRenderer2d->closeSwapChainSpecificObjects(mLogicDevice);

			//Recreate objects
			mSwapChainCreationInfo->setWidth(width);
			mSwapChainCreationInfo->setHeight(height);
			mSwapChain = ObjInit::swapChain(*mSwapChainCreationInfo);

			std::vector<DescriptorTypeInfo> descTypes;
			for (DescriptorTypeInfo& descType : sceneShaderProgram.getUniformLayout().asDescriptorTypes()) {
				descTypes.push_back(descType);
			}
			for (DescriptorTypeInfo& descType : appUiShaderProgram.getUniformLayout().asDescriptorTypes()) {
				descTypes.push_back(descType);
			}
			mDescriptorPool = createDescriptorPool(descTypes, 2);

			prepareScenePipeline(sceneShaderProgram, true);
			prepareAppUiPipeline(appUiShaderProgram, true);

			mRenderer2d->recreateSwapChainSpecificObjects(*mSwapChain);
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
		
		updateSceneUbo(imageIndex);
		mRenderer2d->updateSceneUniformData(mLogicDevice, imageIndex, mSceneUbo.ProjectionViewMat);
		updateUiProjectionMatrix(imageIndex);

		//TODO:: mRenderer2d->uploadSceneData();
		//TODO:: mRenderer2d->uploadUiData();

		//TODO:: draw___(mSwapChain->getRenderPass(SwapChainVulkan::ERenderPassType::SCENE, imageIndex, cmd);
		//TODO:: draw___(mSwapChain->getRenderPass(SwapChainVulkan::ERenderPassType::UI, imageIndex, cmd);
		//TODO:: draw___(mSwapChain->getRenderPass(SwapChainVulkan::ERenderPassType::IMGUI / DEBUG, imageIndex, cmd);

		//Draw scene
		drawScene(imageIndex, cmd);

		//Draw Application UI
		drawUi(imageIndex, cmd);


		//Clear each frame
		mSceneGraphicsPipeline->clearVaoToDraw();
		mAppUiGraphicsPipeline->clearVaoToDraw();

		endCommandBuffer(cmd);

		present(imageIndex, cmd);

		releaseGpuResources();
	}

	void RenderingContextVulkan::drawScene(uint32_t imageIndex, VkCommandBuffer cmd) {
		mSwapChain->beginRenderPass(SwapChainVulkan::ERenderPassType::SCENE, imageIndex, cmd);
		mSceneGraphicsPipeline->bind(cmd);
		mSceneGraphicsPipeline->recordDrawCommands(imageIndex, cmd);
		if (mRenderSceneBatch) {
			mRenderer2d->flush/*Scene*/(mLogicDevice, imageIndex, cmd);
		}
		mSwapChain->endRenderPass(cmd);
	}

	void RenderingContextVulkan::drawUi(uint32_t imageIndex, VkCommandBuffer cmd) {
		mSwapChain->beginRenderPass(SwapChainVulkan::ERenderPassType::APP_UI, imageIndex, cmd);
		mAppUiGraphicsPipeline->bind(cmd);
		mAppUiGraphicsPipeline->recordDrawCommands(imageIndex, cmd);
		if (mRenderUiBatch) {
			//Renderer2d::get().flushUi();
		}
		mImGuiWrapper->render(cmd);
		mSwapChain->endRenderPass(cmd);
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

		VK_TRY(
			vkQueuePresentKHR(mPresentQueue, &present),
			"Failed to present"
		);

		mCurrentFrame = (mCurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
	}

	void RenderingContextVulkan::updateSceneUbo(uint32_t currentImage) {
		//NOTE:: Current the UBO only uses the camera's projection view matrix so updating here works,
		const uint32_t uboBinding = 0;
		mSceneGraphicsPipeline->getShaderDescriptor().getBuffersFromBinding(uboBinding)[currentImage]->
			setData(mLogicDevice, &mSceneUbo.ProjectionViewMat, sizeof(mSceneUbo.ProjectionViewMat));
	}

	void RenderingContextVulkan::updateUiProjectionMatrix(uint32_t currentImage) {
		const uint32_t projBinding = 0;
		mAppUiGraphicsPipeline->getShaderDescriptor().getBuffersFromBinding(projBinding)[currentImage]
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

	VkDescriptorPool RenderingContextVulkan::createDescriptorPool(std::vector<DescriptorTypeInfo>& descTypes, uint32_t pipelineCount) {
		VkDescriptorPool descPool;

		uint32_t imageCount = static_cast<uint32_t>(mSwapChain->getImageCount());

		std::vector<VkDescriptorPoolSize> poolSizes;
		uint32_t poolSizeCount = 0;
		for (DescriptorTypeInfo& descType : descTypes) {
			poolSizes.push_back({ descType.first, descType.second * imageCount });
			poolSizeCount++;
		}

		VkDescriptorPoolCreateInfo poolCreateInfo = {};
		poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolCreateInfo.poolSizeCount = poolSizeCount;
		poolCreateInfo.pPoolSizes = poolSizes.data();
		poolCreateInfo.maxSets = imageCount * pipelineCount;

		VK_TRY(
			vkCreateDescriptorPool(mLogicDevice, &poolCreateInfo, nullptr, &descPool),
			"Failed to create descriptor pool."
		);

		return descPool;
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

	void RenderingContextVulkan::prepareScenePipeline(ShaderProgramVulkan& shaderProgram, bool createUniformObjects) {
		const uint32_t binding = 0;
		std::vector<VkVertexInputAttributeDescription> attribDesc = std::move(Vertex3dTextured::asAttributeDescriptions(binding));
		mSceneGraphicsPipeline = ObjInit::graphicsPipeline(
			mSwapChain->getExtent(),
			mSwapChain->getRenderPass(SwapChainVulkan::ERenderPassType::SCENE).get(),
			shaderProgram,
			createBindingDescription(binding, sizeof(Vertex3dTextured), VK_VERTEX_INPUT_RATE_VERTEX),
			attribDesc
		);

		if (createUniformObjects) {
			createPipelineUniformObjects(*mSceneGraphicsPipeline, mDescriptorPool);
		}
	}

	void RenderingContextVulkan::prepareAppUiPipeline(ShaderProgramVulkan& shaderProgram, bool createUniformObjects) {
		const uint32_t binding = 0;
		std::vector<VkVertexInputAttributeDescription> attribDesc = std::move(Vertex2d::asAttributeDescriptions(binding));
		mAppUiGraphicsPipeline = ObjInit::graphicsPipeline(
			mSwapChain->getExtent(),
			mSwapChain->getRenderPass(SwapChainVulkan::ERenderPassType::APP_UI).get(),
			shaderProgram,
			createBindingDescription(binding, sizeof(Vertex2d), VK_VERTEX_INPUT_RATE_VERTEX),
			attribDesc
		);

		if (createUniformObjects) {
			createPipelineUniformObjects(*mAppUiGraphicsPipeline, mDescriptorPool);
		}
	}

	void RenderingContextVulkan::createPipelineUniformObjects(GraphicsPipelineVulkan& pipeline, VkDescriptorPool descPool) {
		if (pipeline.getShaderProgram().getUniformLayout().hasUniforms()) {
			pipeline.setDescriptorPool(descPool);
			pipeline.uploadShaderUniforms(
				mLogicDevice,
				mPhysicalDevice,
				static_cast<uint32_t>(mSwapChain->getImageCount())
			);
		} else {
			LOG_WARN("Tried to create uniform objects for pipeline without uniforms");
		}
	}

	void RenderingContextVulkan::preparePipeline(
		std::shared_ptr<GraphicsPipelineVulkan> graphicsPipeline,
		ShaderProgramVulkan& shaderProgram,
		VkRenderPass renderPass,
		std::vector<VkVertexInputAttributeDescription>& attribDesc,
		uint32_t vertexStride
	) {
		const uint32_t binding = 0;
		graphicsPipeline = ObjInit::graphicsPipeline(
			mSwapChain->getExtent(),
			renderPass,
			shaderProgram,
			createBindingDescription(binding, vertexStride, VK_VERTEX_INPUT_RATE_VERTEX),
			attribDesc
		);
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

	VkImage RenderingContextVulkan::createImage(
		VkDevice logicDevice,
		VkPhysicalDevice physicalDevice,
		uint32_t width,
		uint32_t height,
		VkFormat format,
		VkImageTiling tiling,
		VkImageUsageFlags usage
	) {
		VkImageCreateInfo imageCreateInfo = {};
		imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
		imageCreateInfo.extent.width = width;
		imageCreateInfo.extent.height = height;
		imageCreateInfo.extent.depth = 1;
		imageCreateInfo.mipLevels = 1;
		imageCreateInfo.arrayLayers = 1;
		imageCreateInfo.format = format;
		imageCreateInfo.tiling = tiling;
		imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageCreateInfo.usage = usage;
		imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageCreateInfo.flags = 0; //Optional

		VkImage image;
		VK_TRY(
			vkCreateImage(logicDevice, &imageCreateInfo, nullptr, &image),
			"Failed to create image"
		);
		return image;
	}

	VkDeviceMemory RenderingContextVulkan::createImageMemory(
		VkDevice logicDevice,
		VkPhysicalDevice physicalDevice,
		VkImage image,
		VkMemoryPropertyFlags props
	) {
		VkDeviceMemory imageMemory;
		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(logicDevice, image, &memRequirements);

		VkMemoryAllocateInfo allocation{};
		allocation.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocation.allocationSize = memRequirements.size;
		allocation.memoryTypeIndex = RendererVulkan::findPhysicalDeviceMemoryType(
			physicalDevice,
			memRequirements.memoryTypeBits,
			props
		);

		VK_TRY(
			vkAllocateMemory(logicDevice, &allocation, nullptr, &imageMemory),
			"Failed to allocate image memory"
		);

		VK_TRY(
			vkBindImageMemory(logicDevice, image, imageMemory, 0),
			"Failed to bind image memory"
		);

		return imageMemory;
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
		return std::make_shared<VertexBufferVulkan>(elements, mLogicDevice, mPhysicalDevice, mCommandPool, mGraphicsQueue, (const void*)data, size, usage, props);
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

	std::shared_ptr<ShaderVulkan> RenderingContextVulkan::createShader(EShaderType type, const std::string& filePath) {
		return std::make_shared<ShaderVulkan>(type, filePath);
	}

	std::shared_ptr<TextureVulkan> RenderingContextVulkan::createTexture(const std::string& filePath) {
		return std::make_shared<TextureVulkan>(mLogicDevice, mPhysicalDevice, filePath);
	}

	std::shared_ptr<TextureVulkan> RenderingContextVulkan::createTexture(float r, float g, float b, float a, bool colourRgbaNormalised) {
		return std::make_shared<TextureVulkan>(mLogicDevice, mPhysicalDevice, r, g, b, a, colourRgbaNormalised);
	}
}
