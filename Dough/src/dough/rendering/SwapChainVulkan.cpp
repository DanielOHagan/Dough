#include "dough/application/Application.h"
#include "dough/rendering/ObjInit.h"
#include "dough/Logging.h"

namespace DOH {

	SwapChainVulkan::SwapChainVulkan(
		VkDevice logicDevice,
		SwapChainSupportDetails scsd,
		VkSurfaceKHR surface,
		QueueFamilyIndices& indices,
		uint32_t width,
		uint32_t height
	) : mSwapChainSupportDetails(scsd),
		mSwapChain(VK_NULL_HANDLE),
		mImageFormat(VK_FORMAT_UNDEFINED),
		mExtent({}),
		mResizable(true)
	{
		init(logicDevice, scsd, surface, indices, width, height);
	}

	SwapChainVulkan::SwapChainVulkan(
		VkDevice logicDevice,
		SwapChainCreationInfo& creationInfo
	) : mSwapChainSupportDetails(creationInfo.SupportDetails),
		mSwapChain(VK_NULL_HANDLE),
		mImageFormat(VK_FORMAT_UNDEFINED),
		mExtent({}),
		mResizable(true)
	{
		init(
			logicDevice,
			mSwapChainSupportDetails,
			creationInfo.Surface,
			creationInfo.Indices,
			creationInfo.getWidth(),
			creationInfo.getHeight()
		);
	}

	void SwapChainVulkan::init(
		VkDevice logicDevice,
		SwapChainSupportDetails& swapChainSupportDetails,
		VkSurfaceKHR surface,
		QueueFamilyIndices& indices,
		uint32_t width,
		uint32_t height
	) {
		TRY(width < 1 || height < 1, "Swap Chain Width and Height must be larger than 0.");

		mSwapChainSupportDetails = swapChainSupportDetails;

		VkSurfaceFormatKHR surfaceFormat = SwapChainVulkan::chooseSwapSurfaceFormat(mSwapChainSupportDetails.formats);
		VkPresentModeKHR presentMode = SwapChainVulkan::chooseSwapPresentMode(mSwapChainSupportDetails.presentModes);
		VkExtent2D extent = SwapChainVulkan::chooseSwapExtent(mSwapChainSupportDetails.capabilities, width, height);

		uint32_t imageCount = mSwapChainSupportDetails.capabilities.minImageCount + 1;
		if (mSwapChainSupportDetails.capabilities.maxImageCount > 0 && imageCount > mSwapChainSupportDetails.capabilities.maxImageCount) {
			imageCount = mSwapChainSupportDetails.capabilities.maxImageCount;
		}

		VkSwapchainCreateInfoKHR createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = surface;
		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

		if (indices.graphicsFamily != indices.presentFamily) {
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		} else {
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			createInfo.queueFamilyIndexCount = 0; //Optional
			createInfo.pQueueFamilyIndices = nullptr; //Optional
		}

		createInfo.preTransform = mSwapChainSupportDetails.capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE;
		createInfo.oldSwapchain = VK_NULL_HANDLE;

		VK_TRY(
			vkCreateSwapchainKHR(logicDevice, &createInfo, nullptr, &mSwapChain),
			"Failed to create swap chain."
		);

		//Query image count in swap chain (we have defined a min image count but not a max and Vulkan might not always use the same amount),
		// then resize images vector and store inside.
		vkGetSwapchainImagesKHR(logicDevice, mSwapChain, &imageCount, nullptr);
		mImages.resize(imageCount);
		vkGetSwapchainImagesKHR(logicDevice, mSwapChain, &imageCount, mImages.data());

		//Store swap chain info for later.
		mImageFormat = surfaceFormat.format;
		mExtent = extent;

		createImageViews(logicDevice);

		mSceneRenderPass = ObjInit::renderPass(mImageFormat, false, true, true);
		mAppUiRenderPass = ObjInit::renderPass(mImageFormat, true, false , false, { 1, 0, 0 });

		createFrameBuffers(logicDevice);
	}

	void SwapChainVulkan::close(VkDevice logicDevice) {
		mSceneRenderPass->close(logicDevice);
		mAppUiRenderPass->close(logicDevice);

		destroyFrameBuffers(logicDevice);

		for (auto imageView : mImageViews) {
			vkDestroyImageView(logicDevice, imageView, nullptr);
		}
		vkDestroySwapchainKHR(logicDevice, mSwapChain, nullptr);
	}

	void SwapChainVulkan::createImageViews(VkDevice logicDevice) {
		mImageViews.resize(mImages.size());

		for (size_t i = 0; i < mImages.size(); i++) {
			mImageViews[i] = Application::get().getRenderer().getContext().createImageView(mImages[i], mImageFormat);
		}
	}

	void SwapChainVulkan::createFrameBuffers(VkDevice logicDevice) {
		const size_t imageCount = getImageCount();
		mSceneFrameBuffers.resize(imageCount);
		mAppUiFrameBuffers.resize(imageCount);

		for (size_t i = 0; i < imageCount; i++) {
			VkImageView sceneAttachments[] = { mImageViews[i] };
			VkFramebufferCreateInfo sceneFrameBufferInfo = {};
			sceneFrameBufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			sceneFrameBufferInfo.renderPass = mSceneRenderPass->get();
			sceneFrameBufferInfo.attachmentCount = 1;
			sceneFrameBufferInfo.pAttachments = sceneAttachments;
			sceneFrameBufferInfo.width = mExtent.width;
			sceneFrameBufferInfo.height = mExtent.height;
			sceneFrameBufferInfo.layers = 1;
			
			VK_TRY(
				vkCreateFramebuffer(logicDevice, &sceneFrameBufferInfo, nullptr, &mSceneFrameBuffers[i]),
				"Failed to create Scene FrameBuffer."
			);

			VkImageView appUiAttachments[] = { mImageViews[i] };
			VkFramebufferCreateInfo appUiFrameBufferInfo = {};
			appUiFrameBufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			appUiFrameBufferInfo.renderPass = mAppUiRenderPass->get();
			appUiFrameBufferInfo.attachmentCount = 1;
			appUiFrameBufferInfo.pAttachments = appUiAttachments;
			appUiFrameBufferInfo.width = mExtent.width;
			appUiFrameBufferInfo.height = mExtent.height;
			appUiFrameBufferInfo.layers = 1;
			
			VK_TRY(
				vkCreateFramebuffer(logicDevice, &appUiFrameBufferInfo, nullptr, &mAppUiFrameBuffers[i]),
				"Failed to create App Ui FrameBuffer."
			);
		}
	}

	void SwapChainVulkan::destroyFrameBuffers(VkDevice logicDevice) {
		for (VkFramebuffer frameBuffer : mSceneFrameBuffers) {
			vkDestroyFramebuffer(logicDevice, frameBuffer, nullptr);
		}

		for (VkFramebuffer frameBuffer : mAppUiFrameBuffers) {
			vkDestroyFramebuffer(logicDevice, frameBuffer, nullptr);
		}
	}
	
	void SwapChainVulkan::beginRenderPass(RenderPassType type, size_t frameBufferIndex, VkCommandBuffer cmd) {
		switch (type) {
			case RenderPassType::SCENE:
				mSceneRenderPass->begin(mSceneFrameBuffers[frameBufferIndex], mExtent, cmd);
				break;
			case RenderPassType::APP_UI:
				mAppUiRenderPass->begin(mAppUiFrameBuffers[frameBufferIndex], mExtent, cmd);
				break;
			default:
				LOG_ERR("Unknown render pass type");
				break;
		}
	}

	void SwapChainVulkan::endRenderPass(VkCommandBuffer cmd) {
		vkCmdEndRenderPass(cmd);
	}

	uint32_t SwapChainVulkan::aquireNextImageIndex(
		VkDevice logicDevice,
		VkFence frameInFlightFence,
		VkSemaphore imageAvailableSemaphore
	) {
		vkWaitForFences(logicDevice, 1, &frameInFlightFence, VK_TRUE, UINT64_MAX);

		uint32_t imageIndex;
		vkAcquireNextImageKHR(
			logicDevice,
			mSwapChain,
			UINT64_MAX,
			imageAvailableSemaphore,
			VK_NULL_HANDLE,
			&imageIndex
		);

		return imageIndex;
	}

	RenderPassVulkan& SwapChainVulkan::getRenderPass(RenderPassType type) const {
		switch (type) {
			case RenderPassType::SCENE:
				if (mSceneRenderPass == nullptr) {
					THROW("Scene render pass is null");
				}
				return *mSceneRenderPass;
			case RenderPassType::APP_UI:
				if (mAppUiRenderPass == nullptr) {
					THROW("App UI render pass is null");
				}
				return *mAppUiRenderPass;
		}

		THROW("Unknown render pass type");
	}

	SwapChainSupportDetails SwapChainVulkan::querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface) {
		SwapChainSupportDetails details;

		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

		if (formatCount != 0) {
			details.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
		}

		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

		if (presentModeCount != 0) {
			details.presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
		}

		return details;
	}

	VkSurfaceFormatKHR SwapChainVulkan::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
		for (const auto& availableFormat : availableFormats) {
			if (
				availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
				availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
			) {
				return availableFormat;
			}
		}

		return availableFormats[0];
	}

	VkPresentModeKHR SwapChainVulkan::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
		for (const auto& availablePresentMode : availablePresentModes) {
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
				return availablePresentMode;
			}
		}

		return VK_PRESENT_MODE_FIFO_KHR;
	}

	VkExtent2D SwapChainVulkan::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, uint32_t width, uint32_t height) {
		if (capabilities.currentExtent.width != UINT32_MAX) {
			return capabilities.currentExtent;
		} else {
			VkExtent2D actualExtent = {width, height};

			actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
			actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

			return actualExtent;
		}
	}
}
