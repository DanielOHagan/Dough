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
		mAppSceneUbo({ glm::mat4x4(1.0f) }),
		mAppUiProjection(1.0f),
		mDepthFormat(VK_FORMAT_UNDEFINED),
		mGpuResourcesFrameCloseCount({})
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
			std::to_string(mPhysicalDeviceProperties->driverVersion),
			Application::get().getRenderer().areValidationLayersEnabled()
		);

		createQueues(queueFamilyIndices);

		createCommandPool(queueFamilyIndices);

		mDepthFormat = RendererVulkan::findSupportedFormat(
			{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
			VK_IMAGE_TILING_OPTIMAL,
			VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
		);

		mSwapChainCreationInfo = std::make_unique<SwapChainCreationInfo>(
			scSupport,
			surface,
			queueFamilyIndices,
			VK_PRESENT_MODE_MAILBOX_KHR,
			true
		);
		mSwapChain = ObjInit::swapChain(*mSwapChainCreationInfo);

		createAppSceneDepthResources();
		createRenderPasses();
		createFrameBuffers();

		createCommandBuffers();
		createSyncObjects();

		mRenderer2d = std::make_unique<Renderer2dVulkan>(*this);
		mRenderer2d->init();

		mLineRenderer = std::make_unique<LineRenderer>();
		mLineRenderer->init(mLogicDevice, mSwapChain->getExtent(), sizeof(UniformBufferObject));

		mImGuiWrapper = std::make_unique<ImGuiWrapper>();
		ImGuiInitInfo imGuiInitInfo = {};
		imGuiInitInfo.ImageCount = mSwapChain->getImageCount();
		imGuiInitInfo.MinImageCount = 2;
		imGuiInitInfo.LogicDevice = mLogicDevice;
		imGuiInitInfo.PhysicalDevice = mPhysicalDevice;
		imGuiInitInfo.Queue = mGraphicsQueue;
		imGuiInitInfo.QueueFamily = queueFamilyIndices.GraphicsFamily.value();
		imGuiInitInfo.VulkanInstance = vulkanInstance;
		imGuiInitInfo.ImageFormat = mSwapChain->getImageFormat();

		mImGuiWrapper->init(window, imGuiInitInfo);
		mImGuiWrapper->uploadFonts(*this);
		mImGuiWrapper->createFrameBuffers(mLogicDevice, mSwapChain->getImageViews(), mSwapChain->getExtent());

		//TEMP:: In future multiple custom render states will be possible at once along with "built-in" render states (e.g. 2D batch renderer and Text)
		mCurrentRenderState = std::make_unique<CustomRenderState>("Application Render State");
	}

	void RenderingContextVulkan::close() {
		const size_t gpuResourceCount = mGpuResourcesToClose.size();
		for (size_t i = 0; i < gpuResourceCount; i++) {
			mGpuResourcesToClose.front()->close(mLogicDevice);
			mGpuResourcesToClose.pop();
		}

		vkFreeCommandBuffers(
			mLogicDevice,
			mCommandPool,
			static_cast<uint32_t>(mCommandBuffers.size()),
			mCommandBuffers.data()
		);

		if (mRenderer2d != nullptr) {
			mRenderer2d->close();
		}
		if (mImGuiWrapper != nullptr) {
			mImGuiWrapper->close(mLogicDevice);
		}
		if (mLineRenderer != nullptr) {
			mLineRenderer->close(mLogicDevice);
		}

		closeSyncObjects();
		closeRenderPasses();
		closeAppSceneDepthResources();
		closeFrameBuffers();

		if (mSwapChain != nullptr) {
			mSwapChain->close(mLogicDevice);
		}
		if (mCurrentRenderState != nullptr) {
			mCurrentRenderState->close(mLogicDevice);
		}

		if (mDescriptorPool != VK_NULL_HANDLE) {
			vkDestroyDescriptorPool(mLogicDevice, mDescriptorPool, nullptr);
		}

		vkDestroyCommandPool(mLogicDevice, mCommandPool, nullptr);
	}

	void RenderingContextVulkan::releaseFrameGpuResources(size_t releaseFrameIndex) {
		if (releaseFrameIndex < GPU_RESOURCE_CLOSE_FRAME_INDEX_COUNT) {
			size_t count = mGpuResourcesFrameCloseCount.at(releaseFrameIndex);

			if (count > 0) {
				for (size_t i = 0; i < count; i++) {
					mGpuResourcesToClose.front()->close(mLogicDevice);
					mGpuResourcesToClose.pop();
				}

				mGpuResourcesFrameCloseCount[releaseFrameIndex] = 0;
				LOG_INFO("Closed " << count << " GPU objects in Swap-Chain frame: " << releaseFrameIndex << ". Delayed by " << GPU_RESOURCE_CLOSE_DELAY_FRAMES << " frame(s)");
			}
		} else {
			LOG_ERR("Invalid releaseFrameIndex: " << releaseFrameIndex);
		}
	}

	void RenderingContextVulkan::resizeSwapChain(
		SwapChainSupportDetails& scSupport,
		VkSurfaceKHR surface,
		uint32_t width,
		uint32_t height
	) {
		if (
			mSwapChain->isResizable() &&
			(mSwapChain->getExtent().width != width || mSwapChain->getExtent().height != height)
		) {
			Application::get().getRenderer().deviceWaitIdle("Device waiting idle for swap chain recreation");

			std::vector<DescriptorTypeInfo> totalDescTypes;

			for (const auto& pipelineGroup : mCurrentRenderState->getRenderPassGraphicsPipelineMap()) {
				for (const auto& [name, pipeline] : pipelineGroup.second) {
					std::vector<DescriptorTypeInfo> descTypes = pipeline->getShaderProgram().getShaderDescriptorLayout().asDescriptorTypes();
					totalDescTypes.reserve(totalDescTypes.size() + descTypes.size());

					for (const DescriptorTypeInfo& descType : descTypes) {
						totalDescTypes.emplace_back(descType);
					}
				}
			}

			std::vector<DescriptorTypeInfo> lineDescTypes = mLineRenderer->getDescriptorTypeInfo();
			totalDescTypes.reserve(totalDescTypes.size() + lineDescTypes.size());
			for (const auto& descType : lineDescTypes) {
				totalDescTypes.emplace_back(descType);
			}

			if (totalDescTypes.size() > 0) {
				vkDestroyDescriptorPool(mLogicDevice, mDescriptorPool, nullptr);

				//TODO:: reset pool instead?
				//vkResetDescriptorPool(mLogicDevice, mDescriptorPool, 0);
			}

			closeRenderPasses();
			closeAppSceneDepthResources();
			closeFrameBuffers();
			mImGuiWrapper->closeRenderPass(mLogicDevice);
			mImGuiWrapper->closeFrameBuffers(mLogicDevice);

			mSwapChainCreationInfo->setWidth(width);
			mSwapChainCreationInfo->setHeight(height);

			if (mSwapChain != nullptr) {
				mSwapChain->resize(mLogicDevice, *mSwapChainCreationInfo);
			} else {
				mSwapChain = ObjInit::swapChain(*mSwapChainCreationInfo);
			}

			createRenderPasses();
			mImGuiWrapper->createRenderPass(mLogicDevice, mSwapChain->getImageFormat());
			createAppSceneDepthResources();
			createFrameBuffers();
			mImGuiWrapper->createFrameBuffers(mLogicDevice, mSwapChain->getImageViews(), mSwapChain->getExtent());

			if (totalDescTypes.size() > 0) {
				mDescriptorPool = createDescriptorPool(totalDescTypes);
			}

			const VkExtent2D extent = mSwapChain->getExtent();
			for (auto& pipelineGroup : mCurrentRenderState->getRenderPassGraphicsPipelineMap()) {
				const VkRenderPass rp = getRenderPass(pipelineGroup.first).get();
				for (auto& pipeline : pipelineGroup.second) {
					pipeline.second->recreate(
						mLogicDevice,
						extent,
						rp
					);
					createPipelineUniformObjects(*pipeline.second, mDescriptorPool);
				}
			}

			mLineRenderer->recreateGraphicsPipelines(
				mLogicDevice,
				extent,
				*mAppSceneRenderPass,
				*mAppUiRenderPass,
				mDescriptorPool
			);

			mRenderer2d->onSwapChainResize(*mSwapChain);
		}
	}

	//Draw then Present rendered frame
	void RenderingContextVulkan::drawFrame() {
		AppDebugInfo& debugInfo = Application::get().getDebugInfo();
		debugInfo.resetPerFrameData();

		uint32_t imageIndex = mSwapChain->aquireNextImageIndex(
			mLogicDevice,
			mFramesInFlightFences[mCurrentFrame],
			mImageAvailableSemaphores[mCurrentFrame]
		);

		mRenderer2d->resetLocalDebugInfo();
		TextRenderer::resetLocalDebugInfo();

		mRenderer2d->updateRenderer2dUniformData(imageIndex, mAppSceneUbo.ProjectionViewMat, mAppUiProjection);
		const uint32_t uboBinding = 0;
		TextRenderer::setUniformData(imageIndex, uboBinding, mAppSceneUbo.ProjectionViewMat, mAppUiProjection);

		VkCommandBuffer cmd = mCommandBuffers[imageIndex];
		beginCommandBuffer(cmd);

		CurrentBindingsState currentBindings = {};

		//Draw scene
		drawScene(imageIndex, cmd, currentBindings);

		//NOTE:: If using subpasses to separate rendering scene/UI, increment subpass index
		//vkCmdNextSubpass(cmd, VK_SUBPASS_CONTENTS_INLINE);

		//Draw Application UI
		drawUi(imageIndex, cmd, currentBindings);

		debugInfo.QuadBatchRendererDrawCalls += TextRenderer::getDrawnQuadCount();

		//Draw ImGui
		mImGuiWrapper->beginRenderPass(imageIndex, mSwapChain->getExtent(), cmd);
		mImGuiWrapper->render(cmd);
		RenderPassVulkan::endRenderPass(cmd);
		mImGuiWrapper->endFrame();

		endCommandBuffer(cmd);

		present(imageIndex, cmd);

		debugInfo.updatePerFrameData();

		releaseFrameGpuResources(mGpuResourceCloseFrame);

		mCurrentFrame = getNextFrameIndex(mCurrentFrame);
		mGpuResourceCloseFrame = getNextGpuResourceCloseFrameIndex(mGpuResourceCloseFrame);
	}

	void RenderingContextVulkan::drawScene(uint32_t imageIndex, VkCommandBuffer cmd, CurrentBindingsState& currentBindings) {
		AppDebugInfo& debugInfo = Application::get().getDebugInfo();

		mAppSceneRenderPass->begin(mAppSceneFrameBuffers[imageIndex], mSwapChain->getExtent(), cmd);
		auto scenePipelines = mCurrentRenderState->getRenderPassGraphicsPipelineGroup(ERenderPass::APP_SCENE);
		if (scenePipelines.has_value()) {
			for (auto& pipeline : scenePipelines->get()) {
				if (pipeline.second->isEnabled() && pipeline.second->getVaoDrawCount() > 0) {
					//TODO:: 
					// FIX:: This sets the data for all shader programs used by scene graphics pipelines in the frame.
					// Even if the data in the descriptors has already been set when a prior pipeline, which has
					// the same shader program, has been cycled through.

					//IMPORTANT:: Assumes all scene pipelines use this UBO and they're at binding 0
					const uint32_t binding = 0;
					pipeline.second->setFrameUniformData(
						mLogicDevice,
						imageIndex,
						binding,
						&mAppSceneUbo,
						sizeof(UniformBufferObject)
					);

					if (currentBindings.Pipeline != pipeline.second->get()) {
						pipeline.second->bind(cmd);
						debugInfo.PipelineBinds++;
						currentBindings.Pipeline = pipeline.second->get();
					}

					pipeline.second->recordDrawCommands(imageIndex, cmd, currentBindings);
					debugInfo.SceneDrawCalls += pipeline.second->getVaoDrawCount();
				}
			}
		}

		//Batch VAOs should have only one VAO and if the batch has at least one quad then there is a draw call
		mRenderer2d->flushScene(imageIndex, cmd, currentBindings);
		TextRenderer::drawScene(imageIndex, cmd, currentBindings, debugInfo);

		mLineRenderer->drawScene(
			mLogicDevice,
			imageIndex,
			&mAppSceneUbo,
			sizeof(UniformBufferObject),
			cmd,
			currentBindings,
			debugInfo
		);

		RenderPassVulkan::endRenderPass(cmd);
	}

	void RenderingContextVulkan::drawUi(uint32_t imageIndex, VkCommandBuffer cmd, CurrentBindingsState& currentBindings) {
		AppDebugInfo& debugInfo = Application::get().getDebugInfo();

		mAppUiRenderPass->begin(mAppUiFrameBuffers[imageIndex], mSwapChain->getExtent(), cmd);

		auto uiPipelines = mCurrentRenderState->getRenderPassGraphicsPipelineGroup(ERenderPass::APP_UI);
		if (uiPipelines.has_value()) {
			for (auto& pipeline : uiPipelines->get()) {
				if (pipeline.second->isEnabled() && pipeline.second->getVaoDrawCount() > 0) {

					//TODO:: 
					// FIX:: This sets the data for all shader programs used by scene graphics pipelines in the frame.
					// Even if the data in the descriptors has already been set when a prior pipeline, which has
					// the same shader program, has been cycled through.

					//IMPORTANT:: Assumes all scene pipelines use this UBO and they're at binding 0
					const uint32_t binding = 0;
					pipeline.second->setFrameUniformData(
						mLogicDevice,
						imageIndex,
						binding,
						&mAppUiProjection,
						sizeof(UniformBufferObject)
					);

					if (currentBindings.Pipeline != pipeline.second->get()) {
						pipeline.second->bind(cmd);
						debugInfo.PipelineBinds++;
					}

					pipeline.second->recordDrawCommands(imageIndex, cmd, currentBindings);
					debugInfo.UiDrawCalls += pipeline.second->getVaoDrawCount();
				}
			}
		}

		mRenderer2d->flushUi(imageIndex, cmd, currentBindings);
		TextRenderer::drawUi(imageIndex, cmd, currentBindings, debugInfo);

		mLineRenderer->drawUi(
			mLogicDevice,
			imageIndex,
			&mAppUiProjection,
			sizeof(UniformBufferObject),
			cmd,
			currentBindings,
			debugInfo
		);

		RenderPassVulkan::endRenderPass(cmd);
	}

	void RenderingContextVulkan::present(uint32_t imageIndex, VkCommandBuffer cmd) {
		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkSemaphore imageReadySemaphores[] = { mImageAvailableSemaphores[mCurrentFrame] };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		VkSemaphore commandsCompletedSemaphores[] = { mRenderFinishedSemaphores[mCurrentFrame] };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = imageReadySemaphores;
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &cmd;
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = commandsCompletedSemaphores;

		vkResetFences(mLogicDevice, 1, &mFramesInFlightFences[mCurrentFrame]);

		VK_TRY(
			vkQueueSubmit(mGraphicsQueue, 1, &submitInfo, mFramesInFlightFences[mCurrentFrame]),
			"Failed to submit Draw Command Buffer."
		);

		VkPresentInfoKHR present = {};
		present.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		present.waitSemaphoreCount = 1;
		present.pWaitSemaphores = commandsCompletedSemaphores;

		VkSwapchainKHR swapChains[] = { mSwapChain->get() };
		present.swapchainCount = 1;
		present.pSwapchains = swapChains;
		present.pImageIndices = &imageIndex;

		VK_TRY(
			vkQueuePresentKHR(mPresentQueue, &present),
			"Failed to present"
		);

		//IMPORTANT:: VK_TRY_KHR allows the TRY call to pass even though VK_SUBOPTIMAL_KHR is returned
		//VK_TRY_KHR(
		//	vkQueuePresentKHR(mPresentQueue, &present),
		//	"Failed to present"
		//);
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
		const uint32_t frameBufferCount = getAppFrameBufferCount();
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

	VkDescriptorPool RenderingContextVulkan::createDescriptorPool(const std::vector<DescriptorTypeInfo>& descTypes) {
		VkDescriptorPool descPool;

		const uint32_t imageCount = mSwapChain->getImageCount();

		std::vector<VkDescriptorPoolSize> poolSizes;
		poolSizes.reserve(descTypes.size());
		for (const DescriptorTypeInfo& descType : descTypes) {
			VkDescriptorPoolSize poolSize = { descType.first, descType.second * imageCount };
			poolSizes.emplace_back(poolSize);
		}

		VkDescriptorPoolCreateInfo poolCreateInfo = {};
		poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolCreateInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
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

	void RenderingContextVulkan::createAppSceneDepthResources() {
		const size_t imageCount = mSwapChain->getImageCount();

		mAppSceneDepthImages.reserve(imageCount);
		for (uint32_t i = 1; i < imageCount; i++) {
			VkImage depthImage = createImage(
				mSwapChain->getExtent().width,
				mSwapChain->getExtent().height,
				mDepthFormat,
				VK_IMAGE_TILING_OPTIMAL,
				VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
			);
			VkDeviceMemory depthImageMem = createImageMemory(
				depthImage,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
			);
			VkImageView depthImageView = createImageView(depthImage, mDepthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
			mAppSceneDepthImages.emplace_back(depthImage, depthImageMem, depthImageView);
		}
	}

	void RenderingContextVulkan::closeAppSceneDepthResources() {
		for (ImageVulkan& depthImage : mAppSceneDepthImages) {
			depthImage.close(mLogicDevice);
		}

		mAppSceneDepthImages.clear();
	}

	void RenderingContextVulkan::createRenderPasses() {
		const VkFormat imageFormat = mSwapChain->getImageFormat();

		{
			SubPassVulkan sceneSubPass = {
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				{//Colour attachments
					{
						ERenderPassAttachmentType::COLOUR,
						imageFormat,
						VK_ATTACHMENT_LOAD_OP_CLEAR,
						VK_ATTACHMENT_STORE_OP_STORE,
						VK_IMAGE_LAYOUT_UNDEFINED,
						VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
						{{ 0.264f, 0.328f, 0.484f, 1.0f }}
					}
				},
				{//optional depth stencil
					{
						ERenderPassAttachmentType::DEPTH,
						mDepthFormat,
						VK_ATTACHMENT_LOAD_OP_CLEAR,
						VK_ATTACHMENT_STORE_OP_DONT_CARE,
						VK_IMAGE_LAYOUT_UNDEFINED,
						VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
						{{ 1.0f, 0 }}
					}
				}
			};

			mAppSceneRenderPass = std::make_shared<RenderPassVulkan>(mLogicDevice, sceneSubPass);
		}

		{
			SubPassVulkan uiSubPass = {
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				{
					{
						ERenderPassAttachmentType::COLOUR,
						imageFormat,
						VK_ATTACHMENT_LOAD_OP_LOAD,
						VK_ATTACHMENT_STORE_OP_STORE,
						VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
						VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
						{{ 0.264f, 0.328f, 0.484f, 1.0f }}
					}
				}
			};

			mAppUiRenderPass = std::make_shared<RenderPassVulkan>(mLogicDevice, uiSubPass);
		}
	}

	void RenderingContextVulkan::closeRenderPasses() {
		mAppSceneRenderPass->close(mLogicDevice);
		mAppUiRenderPass->close(mLogicDevice);
	}

	void RenderingContextVulkan::createFrameBuffers() {
		const uint32_t imageCount = mSwapChain->getImageCount();
		const VkExtent2D extent = mSwapChain->getExtent();
		const std::vector<VkImageView>& imageViews = mSwapChain->getImageViews();

		mAppSceneFrameBuffers.resize(imageCount);
		mAppUiFrameBuffers.resize(imageCount);

		for (size_t i = 0; i < imageCount; i++) {
			std::array<VkImageView, 2> sceneAttachments = { imageViews[i], mAppSceneDepthImages[i % (imageCount - 1)].getImageView() };
			VkFramebufferCreateInfo appSceneFrameBufferInfo = {};
			appSceneFrameBufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			appSceneFrameBufferInfo.renderPass = mAppSceneRenderPass->get();
			appSceneFrameBufferInfo.attachmentCount = static_cast<uint32_t>(sceneAttachments.size());
			appSceneFrameBufferInfo.pAttachments = sceneAttachments.data();
			appSceneFrameBufferInfo.width = extent.width;
			appSceneFrameBufferInfo.height = extent.height;
			appSceneFrameBufferInfo.layers = 1;

			VK_TRY(
				vkCreateFramebuffer(mLogicDevice, &appSceneFrameBufferInfo, nullptr, &mAppSceneFrameBuffers[i]),
				"Failed to create Scene FrameBuffer."
			);

			VkImageView appUiAttachments[] = { imageViews[i] };
			VkFramebufferCreateInfo appUiFrameBufferInfo = {};
			appUiFrameBufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			appUiFrameBufferInfo.renderPass = mAppUiRenderPass->get();
			appUiFrameBufferInfo.attachmentCount = 1;
			appUiFrameBufferInfo.pAttachments = appUiAttachments;
			appUiFrameBufferInfo.width = extent.width;
			appUiFrameBufferInfo.height = extent.height;
			appUiFrameBufferInfo.layers = 1;

			VK_TRY(
				vkCreateFramebuffer(mLogicDevice, &appUiFrameBufferInfo, nullptr, &mAppUiFrameBuffers[i]),
				"Failed to create App Ui FrameBuffer."
			);
		}
	}

	void RenderingContextVulkan::closeFrameBuffers() {
		for (VkFramebuffer frameBuffer : mAppSceneFrameBuffers) {
			vkDestroyFramebuffer(mLogicDevice, frameBuffer, nullptr);
		}

		for (VkFramebuffer frameBuffer : mAppUiFrameBuffers) {
			vkDestroyFramebuffer(mLogicDevice, frameBuffer, nullptr);
		}
	}

	void RenderingContextVulkan::createPipelineUniformObjects(GraphicsPipelineVulkan& pipeline, VkDescriptorPool descPool) {
		if (pipeline.getShaderProgram().getUniformLayout().hasUniforms()) {
			pipeline.createShaderUniforms(
				mLogicDevice,
				mPhysicalDevice,
				mSwapChain->getImageCount(),
				descPool
			);
			pipeline.updateShaderUniforms(mLogicDevice, mSwapChain->getImageCount());
		} else {
			LOG_WARN("Tried to create uniform objects for pipeline without uniforms");
		}
	}

	PipelineRenderableConveyor RenderingContextVulkan::createPipeline(
		const std::string& name,
		GraphicsPipelineInstanceInfo& instanceInfo,
		const bool enabled
	) {
		std::optional<std::reference_wrapper<GraphicsPipelineMap>> map = mCurrentRenderState->getRenderPassGraphicsPipelineGroup(instanceInfo.getRenderPass());

		if (map.has_value()) {
			//Check if pipeline already exists
			const auto& itr = map->get().find(name);
			if (itr != map->get().end()) {
				LOG_ERR("Pipeline already exists: " << name.c_str());
				return {};
			}

			const auto pipeline = createGraphicsPipeline(instanceInfo, mSwapChain->getExtent());
			if (pipeline != nullptr) {
				mCurrentRenderState->addPipelineToRenderPass(instanceInfo.getRenderPass(), name, pipeline);
				return { *pipeline };
			} else {
				LOG_ERR("Failed to create pipeline: " << name.c_str());
				return {};
			}
		} else {
			LOG_ERR("Pipeline group not found: " << ERenderPassStrings[static_cast<uint32_t>(instanceInfo.getRenderPass())]);
			return {};
		}
	}

	PipelineRenderableConveyor RenderingContextVulkan::createPipelineConveyor(
		const ERenderPass renderPass,
		const std::string& name
	) {
		std::optional<std::reference_wrapper<GraphicsPipelineMap>> map = mCurrentRenderState->getRenderPassGraphicsPipelineGroup(renderPass);

		if (map.has_value()) {
			const auto& itr = map->get().find(name);
			if (itr != map->get().end()) {
				return { *itr->second };
			} else {
				LOG_ERR("Pipeline not found: " << name);
				return {};
			}
		} else {
			LOG_ERR("Pipeline group not found: " << ERenderPassStrings[(uint32_t) renderPass]);
			return {};
		}
	}

	void RenderingContextVulkan::enablePipeline(const ERenderPass renderPass, const std::string& name, bool enable) {
		std::optional<std::reference_wrapper<GraphicsPipelineMap>> map = mCurrentRenderState->getRenderPassGraphicsPipelineGroup(renderPass);

		if (map.has_value()) {
			const auto& itr = map->get().find(name);
			if (itr != map->get().end()) {
				itr->second->setEnabled(enable);
			} else {
				LOG_WARN("Unable to find pipeline: " << name);
			}
		}
	}

	void RenderingContextVulkan::closePipeline(const ERenderPass renderPass, const char* name) {
		mCurrentRenderState->closePipeline(mLogicDevice, renderPass, name);
	}

	void RenderingContextVulkan::createPipelineUniformObjects() {
		//TODO:: re-create this function as outlined above it's definition .h file


		std::vector<DescriptorTypeInfo> descTypes;
		uint32_t pipelineCount = 0;

		for (const auto& pipelineGroup : mCurrentRenderState->getRenderPassGraphicsPipelineMap()) {
			for (const auto& pipeline : pipelineGroup.second) {
				for (const auto& descType : pipeline.second->getShaderProgram().getShaderDescriptorLayout().asDescriptorTypes()) {
					descTypes.emplace_back(descType);
				}
				pipelineCount++;
			}
		}

		for (const auto& descType : mLineRenderer->getDescriptorTypeInfo()) {
			descTypes.emplace_back(descType);
			pipelineCount++;
		}

		if (pipelineCount > 0) {
			mDescriptorPool = createDescriptorPool(descTypes);

			const uint32_t imageCount = mSwapChain->getImageCount();

			for (auto& pipelineGroup : mCurrentRenderState->getRenderPassGraphicsPipelineMap()) {
				for (auto& pipeline : pipelineGroup.second) {
					pipeline.second->createShaderUniforms(
						mLogicDevice,
						mPhysicalDevice,
						imageCount,
						mDescriptorPool
					);
					pipeline.second->updateShaderUniforms(mLogicDevice, imageCount);
				}
			}

			mLineRenderer->createShaderUniforms(mLogicDevice, mPhysicalDevice, imageCount, mDescriptorPool);
			mLineRenderer->updateShaderUniforms(mLogicDevice, imageCount);
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

	void RenderingContextVulkan::transitionStagedImageLayout(
		VkImage image,
		VkImageLayout oldLayout,
		VkImageLayout newLayout,
		VkImageAspectFlags aspectFlags
	) {
		VkPipelineStageFlags srcStage;
		VkPipelineStageFlags dstStage;
		VkAccessFlags srcAccessMask;
		VkAccessFlags dstAccessMask;

		if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
			srcAccessMask = 0;
			dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		} else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
			srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		} else {
			THROW("Layout transition not supported");
		}

		transitionImageLayout(
			image,
			oldLayout,
			newLayout,
			aspectFlags,
			srcStage,
			dstStage,
			srcAccessMask,
			dstAccessMask
		);
	}

	void RenderingContextVulkan::transitionImageLayout(
		VkImage image,
		VkImageLayout oldLayout,
		VkImageLayout newLayout,
		VkImageAspectFlags aspectFlags,
		VkPipelineStageFlags srcStage,
		VkPipelineStageFlags dstStage,
		VkAccessFlags srcAccessMask,
		VkAccessFlags dstAccessMask
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
		barrier.srcAccessMask = srcAccessMask;
		barrier.dstAccessMask = dstAccessMask;

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

	VkImageView RenderingContextVulkan::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags) const {
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

	RenderPassVulkan& RenderingContextVulkan::getRenderPass(const ERenderPass renderPass) const {
		switch (renderPass) {
			case ERenderPass::APP_SCENE:
				return *mAppSceneRenderPass;
			case ERenderPass::APP_UI:
				return *mAppUiRenderPass;
		}

		THROW("Unknown render pass");
	}
}
