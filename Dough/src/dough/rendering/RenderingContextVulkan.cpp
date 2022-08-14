#include "dough/rendering/RenderingContextVulkan.h"
#include "dough/rendering/RendererVulkan.h"
#include "dough/rendering/ObjInit.h"
#include "dough/Logging.h"
#include "dough/time/Time.h"
#include "dough/application/Application.h"

#include <chrono>
#include <glm/gtc/matrix_transform.hpp>

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

		//Physical device info
		mRenderingDeviceInfo = std::make_unique<RenderingDeviceInfo>(
			(
				std::to_string(VK_VERSION_MAJOR(mPhysicalDeviceProperties->apiVersion)) + "." +
				std::to_string(VK_VERSION_MINOR(mPhysicalDeviceProperties->apiVersion)) + "." +
				std::to_string(VK_VERSION_PATCH(mPhysicalDeviceProperties->apiVersion))
			),
			mPhysicalDeviceProperties->deviceName,
			std::to_string(mPhysicalDeviceProperties->driverVersion)
		);

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
		imGuiInitInfo.ImageCount = mSwapChain->getImageCount();
		imGuiInitInfo.MinImageCount = 2;
		imGuiInitInfo.LogicDevice = mLogicDevice;
		imGuiInitInfo.PhysicalDevice = mPhysicalDevice;
		imGuiInitInfo.Queue = mGraphicsQueue;
		imGuiInitInfo.QueueFamily = queueFamilyIndices.GraphicsFamily.value();
		imGuiInitInfo.RenderPass = mSwapChain->getRenderPass(SwapChainVulkan::ERenderPassType::IMGUI).get();
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

		if (mRenderer2d != nullptr) {
			mRenderer2d->close(mLogicDevice);
		}
		if (mImGuiWrapper != nullptr) {
			mImGuiWrapper->close(mLogicDevice);
		}

		closeSyncObjects();

		if (mSwapChain != nullptr) {
			mSwapChain->close(mLogicDevice);
		}
		closeCustomPipelines();

		vkDestroyCommandPool(mLogicDevice, mCommandPool, nullptr);
	}
	
	void RenderingContextVulkan::closeCustomPipelines() {
		closeScenePipelines();
		closeUiPipelines();

		if (mDescriptorPool != VK_NULL_HANDLE) {
			vkDestroyDescriptorPool(mLogicDevice, mDescriptorPool, nullptr);
		}
	}

	void RenderingContextVulkan::closeScenePipelines() {
		for (const auto& pipeline : mSceneGraphicsPipelines) {
			pipeline.second->close(mLogicDevice);
		}
		mSceneGraphicsPipelines.clear();
	}

	void RenderingContextVulkan::closeUiPipelines() {
		for (const auto& pipeline : mUiGraphicsPipelines) {
			pipeline.second->close(mLogicDevice);
		}
		mUiGraphicsPipelines.clear();
	}

	void RenderingContextVulkan::closeGpuResourceImmediately(std::shared_ptr<IGPUResourceVulkan> res) {
		res->close(mLogicDevice);
	}

	void RenderingContextVulkan::closeGpuResourceImmediately(IGPUResourceVulkan& res) {
		res.close(mLogicDevice);
	}

	void RenderingContextVulkan::releaseGpuResources() {
		for (std::shared_ptr<IGPUResourceVulkan> res : mToReleaseGpuResources) {
			res->close(mLogicDevice);
		}
		mToReleaseGpuResources.clear();
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

			std::vector<DescriptorTypeInfo> descTypes;
			bool usingDescPool = false;
			for (const auto& pipeline : mSceneGraphicsPipelines) {
				usingDescPool = true;
				
				for (const auto& descType : pipeline.second->getShaderProgram().getUniformLayout().asDescriptorTypes()) {
					descTypes.push_back(descType);
				}
			}

			for (const auto& pipeline : mUiGraphicsPipelines) {
				usingDescPool = true;

				for (const auto& descType : pipeline.second->getShaderProgram().getUniformLayout().asDescriptorTypes()) {
					descTypes.push_back(descType);
				}
			}

			if (usingDescPool) {
				vkDestroyDescriptorPool(mLogicDevice, mDescriptorPool, nullptr);
			}

			if (mSwapChain != nullptr) {
				mSwapChain->close(mLogicDevice);
			}

			//Recreate objects
			mSwapChainCreationInfo->setWidth(width);
			mSwapChainCreationInfo->setHeight(height);
			mSwapChain = ObjInit::swapChain(*mSwapChainCreationInfo);

			if (usingDescPool) {
				mDescriptorPool = createDescriptorPool(descTypes);
			}

			for (const auto& pipeline : mSceneGraphicsPipelines) {
				pipeline.second->recreate(
					mLogicDevice,
					mSwapChain->getExtent(),
					mSwapChain->getRenderPass(SwapChainVulkan::ERenderPassType::SCENE).get()
				);
				createPipelineUniformObjects(*pipeline.second, mDescriptorPool);
			}

			for (const auto& pipeline : mUiGraphicsPipelines) {
				pipeline.second->recreate(
					mLogicDevice,
					mSwapChain->getExtent(),
					mSwapChain->getRenderPass(SwapChainVulkan::ERenderPassType::APP_UI).get()
				);
				createPipelineUniformObjects(*pipeline.second, mDescriptorPool);
			}

			mRenderer2d->onSwapChainResize(mLogicDevice, *mSwapChain);
		}
	}

	//Draw then Present rendered frame
	void RenderingContextVulkan::drawFrame() {
		Application::get().getDebugInfo().resetDrawCalls();

		uint32_t imageIndex = mSwapChain->aquireNextImageIndex(
			mLogicDevice,
			mFramesInFlightFences[mCurrentFrame],
			mImageAvailableSemaphores[mCurrentFrame]
		);

		mRenderer2d->resetDrawCount();

		mRenderer2d->updateRenderer2dUniformData(mLogicDevice, imageIndex, mSceneUbo.ProjectionViewMat);

		VkCommandBuffer cmd = mCommandBuffers[imageIndex];
		beginCommandBuffer(cmd);

		//TODO::
		//	Possible idea for drawing API, not guaranteed:
		//	drawRenderPass(mSwapChain->getRenderPass(SwapChainVulkan::ERenderPassType::/*Whichever render pass*/, imageIndex, cmd);

		//Draw scene
		drawScene(imageIndex, cmd);

		//Draw Application UI
		drawUi(imageIndex, cmd);

		//Draw ImGui
		mSwapChain->beginRenderPass(SwapChainVulkan::ERenderPassType::IMGUI, imageIndex, cmd);
		mImGuiWrapper->render(cmd);
		mSwapChain->endRenderPass(cmd);

		mImGuiWrapper->endFrame();

		//mRenderingDebugInfo.updateTotalDrawCallCount();

		//Clear to-draw lists whether or not pipeline is enabled
		for (const auto& pipeline : mSceneGraphicsPipelines) {
			pipeline.second->clearRenderableToDraw();
		}
		for (const auto& pipeline : mUiGraphicsPipelines) {
			pipeline.second->clearRenderableToDraw();
		}

		endCommandBuffer(cmd);

		present(imageIndex, cmd);

		//TODO:: does this still need to be here?
		//releaseGpuResources();
	}

	void RenderingContextVulkan::drawScene(uint32_t imageIndex, VkCommandBuffer cmd) {
		AppDebugInfo& debugInfo = Application::get().getDebugInfo();

		mSwapChain->beginRenderPass(SwapChainVulkan::ERenderPassType::SCENE, imageIndex, cmd);
		for (const auto& pipeline : mSceneGraphicsPipelines) {
			if (pipeline.second->isEnabled()) {

				//Assumes all scene pipelines use this UBO and they're at binding 0
				const uint32_t binding = 0;
				pipeline.second->getShaderDescriptor().getBuffersFromBinding(binding)[imageIndex]
					->setData(mLogicDevice, &mSceneUbo, sizeof(UniformBufferObject));

				pipeline.second->bind(cmd);
				pipeline.second->recordDrawCommands(imageIndex, cmd);
				debugInfo.SceneDrawCalls += pipeline.second->getVaoDrawCount();
			}
		}
		
		//Batch VAOs should have only one VAO and if the batch has at least one quad then there is a draw call
		mRenderer2d->flushScene(mLogicDevice, imageIndex, cmd);
		debugInfo.BatchRendererDrawCalls = mRenderer2d->getDrawCount();
		mSwapChain->endRenderPass(cmd);
	}

	void RenderingContextVulkan::drawUi(uint32_t imageIndex, VkCommandBuffer cmd) {
		AppDebugInfo& debugInfo = Application::get().getDebugInfo();

		mSwapChain->beginRenderPass(SwapChainVulkan::ERenderPassType::APP_UI, imageIndex, cmd);

		for (const auto& pipeline : mUiGraphicsPipelines) {
			if (pipeline.second->isEnabled()) {

				//Assumes all scene pipelines use this UBO and they're at binding 0
				const uint32_t binding = 0;
				pipeline.second->getShaderDescriptor().getBuffersFromBinding(binding)[imageIndex]
					->setData(mLogicDevice, &mAppUiProjection, sizeof(glm::mat4x4));

				pipeline.second->bind(cmd);
				pipeline.second->recordDrawCommands(imageIndex, cmd);
				debugInfo.UiDrawCalls += pipeline.second->getVaoDrawCount();
			}
		}

		//Renderer2d::get().flushUi();
		mSwapChain->endRenderPass(cmd);
	}

	void RenderingContextVulkan::present(uint32_t imageIndex, VkCommandBuffer cmd) {
		if (mImageFencesInFlight[imageIndex] != VK_NULL_HANDLE) {
			vkWaitForFences(mLogicDevice, 1, &mImageFencesInFlight[imageIndex], VK_TRUE, 100);
		}

		//Mark the image as now being in use by this frame
		mImageFencesInFlight[imageIndex] = mFramesInFlightFences[mCurrentFrame];

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkSemaphore waitSemaphores[] = { mImageAvailableSemaphores[mCurrentFrame] };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		VkSemaphore signalSemaphores[] = { mRenderFinishedSemaphores[mCurrentFrame] };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &cmd;
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

		VK_TRY_KHR(
			vkQueuePresentKHR(mPresentQueue, &present),
			"Failed to present"
		);

		mCurrentFrame = (mCurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
	}

	void RenderingContextVulkan::createQueues(QueueFamilyIndices& queueFamilyIndices) {
		vkGetDeviceQueue(mLogicDevice, queueFamilyIndices.GraphicsFamily.value(), 0, &mGraphicsQueue);
		vkGetDeviceQueue(mLogicDevice, queueFamilyIndices.PresentFamily.value(), 0, &mPresentQueue);
	}

	void RenderingContextVulkan::createCommandPool(QueueFamilyIndices& queueFamilyIndices) {
		VkCommandPoolCreateInfo cmdPoolCreateInfo = {};
		cmdPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		cmdPoolCreateInfo.queueFamilyIndex = queueFamilyIndices.GraphicsFamily.value();
		cmdPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

		VK_TRY(
			vkCreateCommandPool(mLogicDevice, &cmdPoolCreateInfo, nullptr, &mCommandPool),
			"Failed to create Command Pool."
		);
	}

	void RenderingContextVulkan::createCommandBuffers() {
		const uint32_t frameBufferCount = mSwapChain->getFrameBufferCount();
		mCommandBuffers.resize(frameBufferCount);

		VkCommandBufferAllocateInfo cmdBuffAlloc = {};
		cmdBuffAlloc.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		cmdBuffAlloc.commandPool = mCommandPool;
		cmdBuffAlloc.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		cmdBuffAlloc.commandBufferCount = frameBufferCount;

		VK_TRY(
			vkAllocateCommandBuffers(mLogicDevice, &cmdBuffAlloc, mCommandBuffers.data()),
			"Failed to allocate Command Buffers."
		);
	}

	VkDescriptorPool RenderingContextVulkan::createDescriptorPool(std::vector<DescriptorTypeInfo>& descTypes) {
		VkDescriptorPool descPool;

		const uint32_t imageCount = mSwapChain->getImageCount();

		std::vector<VkDescriptorPoolSize> poolSizes;
		uint32_t poolSizeCount = 0;
		for (const DescriptorTypeInfo& descType : descTypes) {
			poolSizes.push_back({ descType.first, descType.second * imageCount });
			poolSizeCount++;
		}

		VkDescriptorPoolCreateInfo poolCreateInfo = {};
		poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolCreateInfo.poolSizeCount = poolSizeCount;
		poolCreateInfo.pPoolSizes = poolSizes.data();
		poolCreateInfo.maxSets = static_cast<uint32_t>(descTypes.size()) * imageCount;

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

	void RenderingContextVulkan::createPipelineUniformObjects(GraphicsPipelineVulkan& pipeline, VkDescriptorPool descPool) {
		if (pipeline.getShaderProgram().getUniformLayout().hasUniforms()) {
			pipeline.uploadShaderUniforms(
				mLogicDevice,
				mPhysicalDevice,
				mSwapChain->getImageCount(),
				descPool
			);
		} else {
			LOG_WARN("Tried to create uniform objects for pipeline without uniforms");
		}
	}

	void RenderingContextVulkan::createPipeline(
		const std::string& name,
		GraphicsPipelineInstanceInfo& instanceInfo,
		const bool enabled
	) {
		if (instanceInfo.RenderPass == SwapChainVulkan::ERenderPassType::IMGUI) {
			LOG_ERR("Unable to create ImGui pipeline");
			return;
		}

		auto& map = instanceInfo.RenderPass ==
			SwapChainVulkan::ERenderPassType::SCENE ? mSceneGraphicsPipelines : mUiGraphicsPipelines;
		const auto& itr = map.find(name);
		if (itr != map.end()) {
			LOG_ERR("Pipeline already exists: " << name);
			return;
		}

		const auto pipeline = createGraphicsPipeline(instanceInfo, mSwapChain->getExtent());
		pipeline->setEnabled(enabled);

		map.emplace(name, pipeline);
	}

	PipelineRenderableConveyer RenderingContextVulkan::createPipelineConveyer(
		const SwapChainVulkan::ERenderPassType renderPass,
		const std::string& name
	) {
		if (renderPass == SwapChainVulkan::ERenderPassType::IMGUI) {
			LOG_ERR("Unable to create ImGui pipeline conveyer");
			return {};
		}

		auto& map = renderPass ==
			SwapChainVulkan::ERenderPassType::SCENE ? mSceneGraphicsPipelines : mUiGraphicsPipelines;
		const auto& itr = map.find(name);
		if (itr != map.end()) {
			return { *itr->second };
		} else {
			LOG_ERR("Pipeline (" << name << ") not found");
			return {};
		}
	}

	void RenderingContextVulkan::enablePipeline(const SwapChainVulkan::ERenderPassType renderPass, const std::string& name, bool enable) {
		if (renderPass == SwapChainVulkan::ERenderPassType::IMGUI) {
			LOG_ERR("Unable to enable/disable ImGui pipeline");
			return;
		}

		auto& map = renderPass ==
			SwapChainVulkan::ERenderPassType::SCENE ? mSceneGraphicsPipelines : mUiGraphicsPipelines;
		const auto& itr = map.find(name);
		if (itr != map.end()) {
			itr->second->setEnabled(enable);
		} else {
			LOG_WARN("Unable to find pipeline: " << name);
		}
	}

	void RenderingContextVulkan::closePipeline(const SwapChainVulkan::ERenderPassType renderPass, const std::string& name) {
		if (renderPass == SwapChainVulkan::ERenderPassType::IMGUI) {
			LOG_ERR("Unable to close ImGui pipeline");
			return;
		}

		auto& map = renderPass == SwapChainVulkan::ERenderPassType::SCENE ? mSceneGraphicsPipelines : mUiGraphicsPipelines;
		const auto& itr = map.find(name);
		if (itr != map.end()) {
			itr->second->close(mLogicDevice);
			map.erase(itr);
		} else {
			LOG_WARN("Unable to find pipeline: " << name);
		}
	}

	void RenderingContextVulkan::createPipelineUniformObjects() {
		std::vector<DescriptorTypeInfo> descTypes;
		uint32_t pipelineCount = 0;
		for (const auto& pipeline : mSceneGraphicsPipelines) {
			for (const auto& descType : pipeline.second->getShaderProgram().getUniformLayout().asDescriptorTypes()) {
				descTypes.push_back(descType);
			}
			pipelineCount++;
		}

		for (const auto& pipeline : mUiGraphicsPipelines) {
			for (const auto& descType : pipeline.second->getShaderProgram().getUniformLayout().asDescriptorTypes()) {
				descTypes.push_back(descType);
			}
			pipelineCount++;
		}

		if (pipelineCount > 0) {
			mDescriptorPool = createDescriptorPool(descTypes);

			for (const auto& pipeline : mSceneGraphicsPipelines) {
				pipeline.second->uploadShaderUniforms(
					mLogicDevice,
					mPhysicalDevice,
					mSwapChain->getImageCount(),
					mDescriptorPool
				);
			}

			for (const auto& pipeline : mUiGraphicsPipelines) {
				pipeline.second->uploadShaderUniforms(
					mLogicDevice,
					mPhysicalDevice,
					mSwapChain->getImageCount(),
					mDescriptorPool
				);
			}
		} else {
			LOG_WARN("Attempted to create uniform objects when not using any custom pipelines");
		}
	}

	VkCommandBuffer RenderingContextVulkan::beginSingleTimeCommands() {
		VkCommandBufferAllocateInfo allocation{};
		allocation.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocation.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocation.commandPool = mCommandPool;
		allocation.commandBufferCount = 1;

		VkCommandBuffer cmd;
		VK_TRY(
			vkAllocateCommandBuffers(mLogicDevice, &allocation, &cmd),
			"Failed to allocate single time command buffer"
		);
		beginCommandBuffer(cmd, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
		return cmd;
	}

	void RenderingContextVulkan::endSingleTimeCommands(VkCommandBuffer cmd) {
		endCommandBuffer(cmd);

		VkSubmitInfo submit{};
		submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit.commandBufferCount = 1;
		submit.pCommandBuffers = &cmd;

		VK_TRY(
			vkQueueSubmit(mGraphicsQueue, 1, &submit, VK_NULL_HANDLE),
			"Failed to submit single time command to graphics queue"
		);

		VK_TRY(
			vkQueueWaitIdle(mGraphicsQueue),
			"Failed to wait on graphics queue for single time command"
		);

		vkFreeCommandBuffers(mLogicDevice, mCommandPool, 1, &cmd);
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

	VkImageView RenderingContextVulkan::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags) {
		VkImageViewCreateInfo view{};
		view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		view.image = image;
		view.viewType = VK_IMAGE_VIEW_TYPE_2D;
		view.format = format;
		view.subresourceRange.aspectMask = aspectFlags;
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
		GraphicsPipelineInstanceInfo& instanceInfo,
		VkExtent2D extent
	) {
		return std::make_unique<GraphicsPipelineVulkan>(
			mLogicDevice,
			instanceInfo,
			mSwapChain->getRenderPass(instanceInfo.RenderPass).get(),
			extent
		);
	}

	std::shared_ptr<SwapChainVulkan> RenderingContextVulkan::createSwapChain(SwapChainCreationInfo& swapChainCreate) {
		return std::make_shared<SwapChainVulkan>(mLogicDevice, swapChainCreate);
	}

	std::shared_ptr<RenderPassVulkan> RenderingContextVulkan::createRenderPass(
		VkFormat imageFormat,
		VkImageLayout initialLayout,
		VkImageLayout finalLayout,
		VkAttachmentLoadOp loadOp,
		bool enableClearColour,
		VkClearValue clearColour,
		VkFormat depthFormat
	) {
		return std::make_shared<RenderPassVulkan>(
			mLogicDevice,
			imageFormat,
			initialLayout,
			finalLayout,
			loadOp,
			enableClearColour,
			clearColour,
			depthFormat
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

	std::shared_ptr<VertexBufferVulkan> RenderingContextVulkan::createVertexBuffer(
		const std::vector<BufferElement>& elements,
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

	std::shared_ptr<VertexBufferVulkan> RenderingContextVulkan::createStagedVertexBuffer(
		const std::vector<BufferElement>& elements,
		const void* data,
		VkDeviceSize size,
		VkBufferUsageFlags usage,
		VkMemoryPropertyFlags props
	) {
		return std::make_shared<VertexBufferVulkan>(elements, mLogicDevice, mPhysicalDevice, mCommandPool, mGraphicsQueue, data, size, usage, props);
	}

	std::shared_ptr<VertexBufferVulkan> RenderingContextVulkan::createStagedVertexBuffer(
		const EVertexType vertexType,
		const void* data,
		VkDeviceSize size,
		VkBufferUsageFlags usage,
		VkMemoryPropertyFlags props
	) {
		return std::make_shared<VertexBufferVulkan>(vertexType, mLogicDevice, mPhysicalDevice, mCommandPool, mGraphicsQueue, data, size, usage, props);
	}

	std::shared_ptr<IndexBufferVulkan> RenderingContextVulkan::createIndexBuffer(VkDeviceSize size) {
		return std::make_shared<IndexBufferVulkan>(mLogicDevice, mPhysicalDevice, size);
	}

	std::shared_ptr<IndexBufferVulkan> RenderingContextVulkan::createStagedIndexBuffer(void* data, VkDeviceSize size) {
		return std::make_shared<IndexBufferVulkan>(mLogicDevice, mPhysicalDevice, mCommandPool, mGraphicsQueue, (const void*) data, size);
	}

	std::shared_ptr<IndexBufferVulkan> RenderingContextVulkan::createStagedIndexBuffer(const void* data, VkDeviceSize size) {
		return std::make_shared<IndexBufferVulkan>(mLogicDevice, mPhysicalDevice, mCommandPool, mGraphicsQueue, data, size);
	}

	std::unique_ptr<IndexBufferVulkan> RenderingContextVulkan::createSharedStagedIndexBuffer(const void* data, VkDeviceSize size) {
		return std::make_unique<IndexBufferVulkan>(mLogicDevice, mPhysicalDevice, mCommandPool, mGraphicsQueue, data, size);
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

	std::shared_ptr<MonoSpaceTextureAtlasVulkan> RenderingContextVulkan::createMonoSpaceTextureAtlas(
		const std::string& filePath,
		const uint32_t rowCount,
		const uint32_t colCount
	) {
		return std::make_shared<MonoSpaceTextureAtlasVulkan>(mLogicDevice, mPhysicalDevice, filePath, rowCount, colCount);
	}
}
