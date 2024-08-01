#include "dough/rendering/RenderingContextVulkan.h"

#include "dough/rendering/RendererVulkan.h"
#include "dough/Logging.h"
#include "dough/time/Time.h"
#include "dough/application/Application.h"
#include "dough/rendering/ShapeRenderer.h"
#include "dough/rendering/text/TextRenderer.h"
#include "dough/rendering/pipeline/DescriptorApiVulkan.h"

#include <chrono>
#include <glm/gtc/matrix_transform.hpp>
#include <tracy/public/tracy/Tracy.hpp>

//TODO:: Is this really necessary?
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
		mEngineDescriptorPool(VK_NULL_HANDLE),
		mCustomDescriptorPool(VK_NULL_HANDLE),
		mCommandPool(VK_NULL_HANDLE),
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
		ZoneScoped;

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
		mSwapChain = createSwapChain(*mSwapChainCreationInfo);

		createAppSceneDepthResources();
		createRenderPasses();
		createFrameBuffers();

		createCommandBuffers();
		createSyncObjects();

		mResourceDefaults.WhiteTexture = createTexture(
			255.0f,
			255.0f,
			255.0f,
			255.0f,
			false,
			"White Texture"
		);
		mResourceDefaults.EmptyTextureArray = std::make_unique<TextureArray>(8, *mResourceDefaults.WhiteTexture);

		mSceneCameraGpu = std::make_unique<CameraGpuData>();
		mSceneCameraGpu->CpuData.ProjView = glm::mat4x4(1.0f);
		mUiCameraGpu = std::make_unique<CameraGpuData>();
		mUiCameraGpu->CpuData.ProjView = glm::mat4x4(1.0f);

		const uint32_t imageCount = mSwapChain->getImageCount();
		mSceneCameraGpu->ValueBuffers.resize(imageCount);
		mUiCameraGpu->ValueBuffers.resize(imageCount);
		for (uint32_t i = 0; i < imageCount; i++) {
			mSceneCameraGpu->ValueBuffers[i] = createBuffer(
				sizeof(UniformBufferObject),
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
			);
			mSceneCameraGpu->ValueBuffers[i]->map(mLogicDevice, sizeof(UniformBufferObject));

			mUiCameraGpu->ValueBuffers[i] = createBuffer(
				sizeof(UniformBufferObject),
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
			);
			mUiCameraGpu->ValueBuffers[i]->map(mLogicDevice, sizeof(UniformBufferObject));
		}

		createEngineDescriptorLayouts();
		createEngineDescriptorPool();
		createEngineDescriptorSets();

		ShapeRenderer::init(*this);
		TextRenderer::init(*this);
		LineRenderer::init(*this);

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

		//Create custom pool with "max" limits so the editor can create as many as it needs.
		//Currently set to an arbitrary number that should be enough for my uses.
		//Some types are commented out since they are not currently used.
		// 
		//TODO:: Needs something to track descriptor counts to prevent trying to alloc after reaching limit.
		std::vector<DescriptorTypeInfo> customPoolDescTypeInfos = {
			//{ VK_DESCRIPTOR_TYPE_SAMPLER,					8192u },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,	8192u },
			//{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,				8192u },
			//{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,				8192u },
			//{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,		8192u },
			//{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,		8192u },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,			8192u },
			//{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,			8192u },
			//{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,	8192u },
			//{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC,	8192u }
			//{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,			8192u }
		};
		mCustomDescriptorPool = createDescriptorPool(customPoolDescTypeInfos);
	}

	void RenderingContextVulkan::close() {
		ZoneScoped;

		vkFreeCommandBuffers(
			mLogicDevice,
			mCommandPool,
			static_cast<uint32_t>(mCommandBuffers.size()),
			mCommandBuffers.data()
		);

		for (auto& buffer : mSceneCameraGpu->ValueBuffers) {
			buffer->close(mLogicDevice);
		}
		for (auto& buffer : mUiCameraGpu->ValueBuffers) {
			buffer->close(mLogicDevice);
		}

		TextRenderer::close();
		ShapeRenderer::close();
		LineRenderer::close();
		if (mImGuiWrapper != nullptr) {
			mImGuiWrapper->close(mLogicDevice);
		}

		//Resource Defaults
		mResourceDefaults.WhiteTexture->close(mLogicDevice);

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

		//Close deletion queue at end of function so other IGPU resources can be added before.
		//NOTE:: This is fine since closing is single threaded.
		const size_t gpuResourceCount = mGpuResourcesToClose.size();
		for (size_t i = 0; i < gpuResourceCount; i++) {
			mGpuResourcesToClose.front()->close(mLogicDevice);
			mGpuResourcesToClose.pop();
		}

		for (auto& setLayout : mAllDescriptorSetLayouts) {
			setLayout.second->close(mLogicDevice);
		}

		if (mEngineDescriptorPool != VK_NULL_HANDLE) {
			vkDestroyDescriptorPool(mLogicDevice, mEngineDescriptorPool, nullptr);
		}
		if (mCustomDescriptorPool != VK_NULL_HANDLE) {
			vkDestroyDescriptorPool(mLogicDevice, mCustomDescriptorPool, nullptr);
		}

		vkDestroyCommandPool(mLogicDevice, mCommandPool, nullptr);
	}

	void RenderingContextVulkan::releaseFrameGpuResources(size_t releaseFrameIndex) {
		ZoneScoped;

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
		ZoneScoped;

		if (
			mSwapChain->isResizable() &&
			(mSwapChain->getExtent().width != width || mSwapChain->getExtent().height != height)
		) {
			Application::get().getRenderer().deviceWaitIdle("Device waiting idle for swap chain recreation");

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
				mSwapChain = createSwapChain(*mSwapChainCreationInfo);
			}

			createRenderPasses();
			mImGuiWrapper->createRenderPass(mLogicDevice, mSwapChain->getImageFormat());
			createAppSceneDepthResources();
			createFrameBuffers();
			mImGuiWrapper->createFrameBuffers(mLogicDevice, mSwapChain->getImageViews(), mSwapChain->getExtent());

			ShapeRenderer::onSwapChainResize(*mSwapChain);
			TextRenderer::onSwapChainResize(*mSwapChain);
			LineRenderer::onSwapChainResize(*mSwapChain);

			const VkExtent2D extent = mSwapChain->getExtent();
			for (auto& pipelineGroup : mCurrentRenderState->getRenderPassGraphicsPipelineMap()) {
				const VkRenderPass rp = getRenderPass(pipelineGroup.first).get();
				for (auto& pipeline : pipelineGroup.second) {
					pipeline.second->resize(
						mLogicDevice,
						extent,
						rp
					);
				}
			}
		}
	}

	//Draw then Present rendered frame
	void RenderingContextVulkan::drawFrame() {
		ZoneScoped;

		AppDebugInfo& debugInfo = Application::get().getDebugInfo();
		debugInfo.resetPerFrameData();

		uint32_t imageIndex = mSwapChain->aquireNextImageIndex(
			mLogicDevice,
			mFramesInFlightFences[mCurrentFrame],
			mImageAvailableSemaphores[mCurrentFrame]
		);

		ShapeRenderer::resetLocalDebugInfo();
		TextRenderer::resetLocalDebugInfo();

		//Set camera data
		mSceneCameraGpu->ValueBuffers[imageIndex]->setDataMapped(mLogicDevice, &mSceneCameraGpu->CpuData.ProjView, sizeof(UniformBufferObject));
		mUiCameraGpu->ValueBuffers[imageIndex]->setDataMapped(mLogicDevice, &mUiCameraGpu->CpuData.ProjView, sizeof(UniformBufferObject));

		VkCommandBuffer cmd = mCommandBuffers[imageIndex];
		beginCommandBuffer(cmd);

		//TODO:: CurrentBindings should last after a frame is done, recreating here is in-efficient
		CurrentBindingsState currentBindings = {};

		drawScene(imageIndex, cmd, currentBindings);
		drawUi(imageIndex, cmd, currentBindings);

		debugInfo.QuadBatchRendererDrawCalls += ShapeRenderer::getDrawnQuadCount();
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
		ZoneScoped;

		AppDebugInfo& debugInfo = Application::get().getDebugInfo();

		mAppSceneRenderPass->begin(mAppSceneFrameBuffers[imageIndex], mSwapChain->getExtent(), cmd);
		auto scenePipelines = mCurrentRenderState->getRenderPassGraphicsPipelineGroup(ERenderPass::APP_SCENE);
		if (scenePipelines.has_value()) {
			for (auto& pipeline : scenePipelines->get()) {
				if (pipeline.second->getVaoDrawCount() > 0) {
					if (currentBindings.Pipeline != pipeline.second->get()) {
						pipeline.second->bind(cmd);
						debugInfo.PipelineBinds++;
						currentBindings.Pipeline = pipeline.second->get();
					}
					bindSceneUboToPipeline(cmd, *pipeline.second, imageIndex, currentBindings, debugInfo);
					pipeline.second->recordDrawCommands(cmd, currentBindings, 1);
					debugInfo.SceneDrawCalls += pipeline.second->getVaoDrawCount();
				}
			}
		}

		ShapeRenderer::drawScene(imageIndex, cmd, currentBindings);
		TextRenderer::drawScene(imageIndex, cmd, currentBindings);
		LineRenderer::drawScene(imageIndex, cmd, currentBindings);

		RenderPassVulkan::endRenderPass(cmd);
	}

	void RenderingContextVulkan::drawUi(uint32_t imageIndex, VkCommandBuffer cmd, CurrentBindingsState& currentBindings) {
		ZoneScoped;

		AppDebugInfo& debugInfo = Application::get().getDebugInfo();

		mAppUiRenderPass->begin(mAppUiFrameBuffers[imageIndex], mSwapChain->getExtent(), cmd);

		auto uiPipelines = mCurrentRenderState->getRenderPassGraphicsPipelineGroup(ERenderPass::APP_UI);
		if (uiPipelines.has_value()) {
			for (auto& pipeline : uiPipelines->get()) {
				if (pipeline.second->getVaoDrawCount() > 0) {
					if (currentBindings.Pipeline != pipeline.second->get()) {
						pipeline.second->bind(cmd);
						debugInfo.PipelineBinds++;
						currentBindings.Pipeline = pipeline.second->get();
					}

					bindUiUboToPipeline(
						cmd,
						*pipeline.second,
						imageIndex,
						currentBindings,
						debugInfo
					);

					pipeline.second->recordDrawCommands(cmd, currentBindings, 1);
					debugInfo.UiDrawCalls += pipeline.second->getVaoDrawCount();
				}
			}
		}

		ShapeRenderer::drawUi(imageIndex, cmd, currentBindings);
		TextRenderer::drawUi(imageIndex, cmd, currentBindings);
		LineRenderer::drawUi(imageIndex,cmd, currentBindings);

		RenderPassVulkan::endRenderPass(cmd);
	}

	void RenderingContextVulkan::present(uint32_t imageIndex, VkCommandBuffer cmd) {
		ZoneScoped;

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

	void RenderingContextVulkan::bindSceneUboToPipeline(
		VkCommandBuffer cmd,
		GraphicsPipelineVulkan& pipeline,
		uint32_t imageIndex,
		CurrentBindingsState& currentBindings,
		AppDebugInfo& debugInfo
	) {
		//Bind scene ubo since GraphicsPipeline class doesn't have access to desc set object
		const uint32_t uboSlot = 0;
		VkDescriptorSet currentUboDescSet = mSceneCameraGpu->DescriptorSets[imageIndex];
		vkCmdBindDescriptorSets(
			cmd,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			pipeline.getGraphicsPipelineLayout(),
			uboSlot,
			1,
			&currentUboDescSet,
			0,
			nullptr
		);
		currentBindings.DescriptorSets[uboSlot] = currentUboDescSet;
		debugInfo.DescriptorSetBinds++;
	}

	void RenderingContextVulkan::bindUiUboToPipeline(
		VkCommandBuffer cmd,
		GraphicsPipelineVulkan& pipeline,
		uint32_t imageIndex,
		CurrentBindingsState& currentBindings,
		AppDebugInfo& debugInfo
	) {
		//Bind scene ubo since GraphicsPipeline class doesn't have access to desc set object
		const uint32_t uboSlot = 0;
		VkDescriptorSet currentUboDescSet = mUiCameraGpu->DescriptorSets[imageIndex];
		vkCmdBindDescriptorSets(
			cmd,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			pipeline.getGraphicsPipelineLayout(),
			uboSlot,
			1,
			&currentUboDescSet,
			0,
			nullptr
		);
		currentBindings.DescriptorSets[uboSlot] = currentUboDescSet;
		debugInfo.DescriptorSetBinds++;
	}

	void RenderingContextVulkan::createQueues(QueueFamilyIndices& queueFamilyIndices) {
		ZoneScoped;

		vkGetDeviceQueue(mLogicDevice, queueFamilyIndices.GraphicsFamily.value(), 0, &mGraphicsQueue);
		vkGetDeviceQueue(mLogicDevice, queueFamilyIndices.PresentFamily.value(), 0, &mPresentQueue);
	}

	void RenderingContextVulkan::createCommandPool(QueueFamilyIndices& queueFamilyIndices) {
		ZoneScoped;

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
		ZoneScoped;

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
		ZoneScoped;

		VkDescriptorPool descPool;

		const uint32_t imageCount = mSwapChain->getImageCount();
		//NOTE:: Only supporting the first 11 types of VkDescriptorType. Most will likely not even be needed.
		constexpr size_t MAX_DESC_TYPE_COUNT = 11;

		std::vector<VkDescriptorPoolSize> poolSizes;
		std::array<uint32_t, MAX_DESC_TYPE_COUNT> poolsSizeIndexes = {};
		poolsSizeIndexes.fill(UINT_MAX);
		poolSizes.reserve(MAX_DESC_TYPE_COUNT);
		uint32_t descCount = 0;
		uint32_t nextAvailablePoolIndex = 0;
		for (const DescriptorTypeInfo& descType : descTypes) {

			//IMPORTANT:: This cast works because poolSizeIndexes uses the same index order as the VkDescriptorType enum definitions
			uint32_t typeIndex = static_cast<uint32_t>(descType.first);
			if (poolsSizeIndexes[typeIndex] == UINT32_MAX) {
				VkDescriptorPoolSize poolSize = { descType.first, 0 };
				poolSizes.emplace_back(poolSize);

				poolsSizeIndexes[typeIndex] = nextAvailablePoolIndex;
				nextAvailablePoolIndex++;
			}
			poolSizes[poolsSizeIndexes[typeIndex]].descriptorCount += descType.second;

			//If the cast option is not working then create a switch statement with a case for each descriptor type.
			//switch (descType.first) {
			//	case {DescriptorType}:
			//	{
			//		if (poolsSizeIndexes[{DescriptorTypeIndex}] == UINT32_MAX) {
			//			VkDescriptorPoolSize poolSize = { descType.first, 0 };
			//			poolSizes.emplace_back(poolSize);
			//
			//			poolsSizeIndexes[{DescriptorTypeIndex}] = nextAvailablePoolIndex;
			//			nextAvailablePoolIndex++;
			//		}
			//		poolSizes[poolsSizeIndexes[{DescriptorTypeIndex}]].descriptorCount += descType.second;
			//		break;
			//	}
			//}

			descCount += descType.second;
		}

		VkDescriptorPoolCreateInfo poolCreateInfo = {};
		poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolCreateInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		poolCreateInfo.pPoolSizes = poolSizes.data();
		poolCreateInfo.maxSets = descCount;

		VK_TRY(
			vkCreateDescriptorPool(mLogicDevice, &poolCreateInfo, nullptr, &descPool),
			"Failed to create descriptor pool."
		);

		return descPool;
	}

	void RenderingContextVulkan::createSyncObjects() {
		ZoneScoped;

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
		ZoneScoped;

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			vkDestroySemaphore(mLogicDevice, mImageAvailableSemaphores[i], nullptr);
			vkDestroySemaphore(mLogicDevice, mRenderFinishedSemaphores[i], nullptr);
			vkDestroyFence(mLogicDevice, mFramesInFlightFences[i], nullptr);
		}
	}

	void RenderingContextVulkan::createAppSceneDepthResources() {
		ZoneScoped;

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
		ZoneScoped;

		for (ImageVulkan& depthImage : mAppSceneDepthImages) {
			depthImage.close(mLogicDevice);
		}

		mAppSceneDepthImages.clear();
	}

	void RenderingContextVulkan::createRenderPasses() {
		ZoneScoped;

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
		ZoneScoped;

		mAppSceneRenderPass->close(mLogicDevice);
		mAppUiRenderPass->close(mLogicDevice);
	}

	void RenderingContextVulkan::createFrameBuffers() {
		ZoneScoped;

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
		ZoneScoped;

		for (VkFramebuffer frameBuffer : mAppSceneFrameBuffers) {
			vkDestroyFramebuffer(mLogicDevice, frameBuffer, nullptr);
		}

		for (VkFramebuffer frameBuffer : mAppUiFrameBuffers) {
			vkDestroyFramebuffer(mLogicDevice, frameBuffer, nullptr);
		}
	}

	PipelineRenderableConveyor RenderingContextVulkan::createPipelineInCurrentRenderState(
		const std::string& name,
		GraphicsPipelineInstanceInfo& instanceInfo
	) {
		ZoneScoped;

		std::optional<std::reference_wrapper<GraphicsPipelineMap>> map = mCurrentRenderState->getRenderPassGraphicsPipelineGroup(instanceInfo.getRenderPass());

		if (map.has_value()) {
			//Check if pipeline already exists
			const auto& itr = map->get().find(name);
			if (itr != map->get().end()) {
				LOG_ERR("Pipeline already exists: " << name.c_str());
				return {};
			}

			const auto pipeline = createGraphicsPipeline(instanceInfo);
			pipeline->init(mLogicDevice, mSwapChain->getExtent(), getRenderPass(instanceInfo.getRenderPass()).get());
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
		ZoneScoped;

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
			LOG_ERR("Pipeline group not found: " << ERenderPassStrings[static_cast<uint32_t>(renderPass)]);
			return {};
		}
	}

	void RenderingContextVulkan::closePipeline(const ERenderPass renderPass, const char* name) {
		ZoneScoped;

		mCurrentRenderState->closePipeline(mLogicDevice, renderPass, name);
	}

	void RenderingContextVulkan::createEngineDescriptorLayouts() {
		std::vector<AShaderDescriptor> uboDesc = {
			ValueDescriptor(0, sizeof(UniformBufferObject))
		};
		std::vector<AShaderDescriptor> textureDesc = {
			TextureDescriptor(0, *mResourceDefaults.WhiteTexture)
		};
		std::vector<AShaderDescriptor> textureArrayDesc = {
			TextureArrayDescriptor(0, *mResourceDefaults.EmptyTextureArray)
		};

		std::shared_ptr<DescriptorSetLayoutVulkan> uboSet = createDescriptorSetLayout(uboDesc, true, "ubo");
		std::shared_ptr<DescriptorSetLayoutVulkan> textureSet = createDescriptorSetLayout(textureDesc, true, "singleTexture");
		std::shared_ptr<DescriptorSetLayoutVulkan> textureArraySet = createDescriptorSetLayout(textureArrayDesc, true, "singleTextureArray8");

		//Allow for easier access outisde of the layout map
		mCommonDescriptorSetLayouts = std::make_unique<CommonDescriptorSetLayouts>(*uboSet, *textureSet, *textureArraySet);
	}

	void RenderingContextVulkan::createEngineDescriptorPool() {
		//IMPORTANT NOTE:: The desc count of a texture array is added to total desc type counts because allocation requires a set layout.
		//This allows for the use of textures being used as a single sampler and also in an array in a different shader.

		std::vector<DescriptorTypeInfo> descTypeInfos;
		
		const uint32_t imageCount = mSwapChain->getImageCount();

		//Init to 3 for required descriptor sets:
		//Scene & UI UBO's and mResourceDefaults members
		uint32_t descTypeInfoCount = 3;

		std::vector<DescriptorTypeInfo> shapeDescTypeInfos = ShapeRenderer::getEngineDescriptorTypeInfos();
		descTypeInfoCount += static_cast<uint32_t>(shapeDescTypeInfos.size());
		std::vector<DescriptorTypeInfo> textDescTypeInfos = TextRenderer::getEngineDescriptorTypeInfos();
		descTypeInfoCount += static_cast<uint32_t>(textDescTypeInfos.size());
		//std::vector<DescriptorTypeInfo> lineDescTypeInfos = LineRenderer::getEngineDescriptorTypeInfos();
		//descTypeInfoCount += lineDescTypeInfos.size();

		descTypeInfos.reserve(descTypeInfoCount);

		descTypeInfos.push_back({ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, imageCount }); //Scene UBO
		descTypeInfos.push_back({ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, imageCount }); //UI UBO
		descTypeInfos.push_back({ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1u }); //White Texture
		descTypeInfos.push_back({ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 8u }); //Empty Texture Array

		for (const auto& descTypeInfo : shapeDescTypeInfos) {
			descTypeInfos.emplace_back(descTypeInfo);
		}
		for (const auto& descTypeInfo : textDescTypeInfos) {
			descTypeInfos.emplace_back(descTypeInfo);
		}

		mEngineDescriptorPool = createDescriptorPool(descTypeInfos);
	}

	void RenderingContextVulkan::createEngineDescriptorSets() {
		//Allocate descriptor sets
		const uint32_t imageCount = mSwapChain->getImageCount();
		DescriptorSetLayoutVulkan& uboSetLayout = mCommonDescriptorSetLayouts->Ubo;
		DescriptorSetLayoutVulkan& textureSetLayout = mCommonDescriptorSetLayouts->SingleTexture;
		DescriptorSetLayoutVulkan& texArrSetLayout = mCommonDescriptorSetLayouts->SingleTextureArray8;

		//Uniform Buffers
		mSceneCameraGpu->DescriptorSets = DescriptorApiVulkan::allocateDescriptorSetsFromLayout(
			mLogicDevice,
			mEngineDescriptorPool,
			uboSetLayout,
			imageCount
		);
		mUiCameraGpu->DescriptorSets = DescriptorApiVulkan::allocateDescriptorSetsFromLayout(
			mLogicDevice,
			mEngineDescriptorPool,
			uboSetLayout,
			imageCount
		);

		//Textures
		mResourceDefaults.WhiteTextureDescriptorSet = DescriptorApiVulkan::allocateDescriptorSetFromLayout(
			mLogicDevice,
			mEngineDescriptorPool,
			textureSetLayout
		);

		//Texture Array
		mResourceDefaults.EmptyTextureArrayDescriptorSet = DescriptorApiVulkan::allocateDescriptorSetFromLayout(
			mLogicDevice,
			mEngineDescriptorPool,
			texArrSetLayout
		);

		//Update descriptor sets
		//Cameras
		const uint32_t projViewBinding = 0;
		for (uint32_t i = 0; i < imageCount; i++) {
			//Scene camera
			DescriptorSetUpdate sceneCameraUpdate = {
				{{ uboSetLayout.getDescriptors()[projViewBinding], *mSceneCameraGpu->ValueBuffers[i] }},
				mSceneCameraGpu->DescriptorSets[i]
			};
			DescriptorApiVulkan::updateDescriptorSet(mLogicDevice, sceneCameraUpdate);

			//UI camera
			DescriptorSetUpdate uiCameraUpdate = {
				{{ uboSetLayout.getDescriptors()[projViewBinding], *mUiCameraGpu->ValueBuffers[i] }},
				mUiCameraGpu->DescriptorSets[i]
			};
			DescriptorApiVulkan::updateDescriptorSet(mLogicDevice, uiCameraUpdate);
		}

		//Textures
		const uint32_t textureSamplerBinding = 0;
		DescriptorSetUpdate whiteTextureUpdate = {
			{{ textureSetLayout.getDescriptors()[textureSamplerBinding], *mResourceDefaults.WhiteTexture }},
			mResourceDefaults.WhiteTextureDescriptorSet
		};
		DescriptorApiVulkan::updateDescriptorSet(mLogicDevice, whiteTextureUpdate);

		//Texture Arrays
		const uint32_t texArrBinding = 0;
		DescriptorSetUpdate texArrUpdate = {
			{{ texArrSetLayout.getDescriptors()[texArrBinding], *mResourceDefaults.EmptyTextureArray }},
			mResourceDefaults.EmptyTextureArrayDescriptorSet
		};
	}

	VkCommandBuffer RenderingContextVulkan::beginSingleTimeCommands() {
		ZoneScoped;

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
		ZoneScoped;

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
		ZoneScoped;

		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = usage;

		VK_TRY(
			vkBeginCommandBuffer(cmd, &beginInfo),
			"Failed to begin recording Command Buffer."
		);
	}

	void RenderingContextVulkan::endCommandBuffer(VkCommandBuffer cmd) {
		ZoneScoped;

		VK_TRY(
			vkEndCommandBuffer(cmd),
			"Failed to end command buffer"
		);
	}

	void RenderingContextVulkan::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
		ZoneScoped;

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
		ZoneScoped;

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
		ZoneScoped;

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
		ZoneScoped;

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
		ZoneScoped;

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
		ZoneScoped;

		mPhysicalDevice = physicalDevice;
		vkGetPhysicalDeviceProperties(mPhysicalDevice, mPhysicalDeviceProperties.get());
	}

	void RenderingContextVulkan::addDescriptorSetLayout(const char* name, std::shared_ptr<DescriptorSetLayoutVulkan> setLayout) {
		auto result = mAllDescriptorSetLayouts.emplace(name, setLayout);
		if (!result.second) {
			LOG_ERR("Failed to add set layout to mAllDescriptorSets. Name: " << name);
		}
	}

	std::shared_ptr<DescriptorSetLayoutVulkan> RenderingContextVulkan::getDescriptorSetLayout(const char* name) {
		ZoneScoped;

		auto result = mAllDescriptorSetLayouts.find(name);
		return result != mAllDescriptorSetLayouts.end() ? result->second : nullptr;
	}

	void RenderingContextVulkan::removeDescriptorSetLayout(const char* name, bool closeLayout) {
		ZoneScoped;

		auto set = mAllDescriptorSetLayouts.extract(name);

		if (closeLayout && !set.empty()) {
			addGpuResourceToClose(set.mapped());
		}
	}

	std::shared_ptr<DescriptorSetLayoutVulkan> RenderingContextVulkan::createDescriptorSetLayout(
		const std::vector<AShaderDescriptor>& uniforms,
		bool addToLayoutCache,
		const char* name
	) {
		ZoneScoped;

		std::shared_ptr<DescriptorSetLayoutVulkan> set = std::make_shared<DescriptorSetLayoutVulkan>(uniforms);
		set->init(mLogicDevice);
		if (addToLayoutCache) {
			addDescriptorSetLayout(name, set);
		}
		return set;
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
		ZoneScoped;

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
		ZoneScoped;

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

	VkPushConstantRange RenderingContextVulkan::pushConstantInfo(VkShaderStageFlagBits stage, uint32_t size, uint32_t offset) {
		VkPushConstantRange pushConstant = {};
		pushConstant.stageFlags = stage;
		pushConstant.size = size;
		pushConstant.offset = offset;
		return pushConstant;
	}
}
