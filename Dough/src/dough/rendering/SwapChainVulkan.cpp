#include "dough/rendering/SwapChainVulkan.h"

namespace DOH {

	SwapChainVulkan::SwapChainVulkan()
	:	mSwapChainSupportDetails(),
		mVkSwapChain(VK_NULL_HANDLE),
		mVkSwapChainImageFormat(VK_FORMAT_UNDEFINED),
		mVkSwapChainExtent({}),
		mResizable(true)
	{
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

		TRY(
			vkCreateSwapchainKHR(logicDevice, &createInfo, nullptr, &mVkSwapChain) != VK_SUCCESS,
			"Failed to create swap chain."
		);

		//Query image count in swap chain (we have defined a min image count but not a max and Vulkan might not always use the same amount),
		// then resize images vector and store inside.
		vkGetSwapchainImagesKHR(logicDevice, mVkSwapChain, &imageCount, nullptr);
		mVkSwapChainImages.resize(imageCount);
		vkGetSwapchainImagesKHR(logicDevice, mVkSwapChain, &imageCount, mVkSwapChainImages.data());

		//Store swap chain info for later.
		mVkSwapChainImageFormat = surfaceFormat.format;
		mVkSwapChainExtent = extent;

		createImageViews(logicDevice);
	}

	void SwapChainVulkan::close(VkDevice logicDevice) {
		destroyFramebuffers(logicDevice);

		for (auto imageView : mVkSwapChainImageViews) {
			vkDestroyImageView(logicDevice, imageView, nullptr);
		}
		vkDestroySwapchainKHR(logicDevice, mVkSwapChain, nullptr);
	}

	void SwapChainVulkan::createImageViews(VkDevice logicDevice) {
		mVkSwapChainImageViews.resize(mVkSwapChainImages.size());

		for (size_t i = 0; i < mVkSwapChainImages.size(); i++) {
			VkImageViewCreateInfo createInfo = {};
			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createInfo.image = mVkSwapChainImages[i];
			createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			createInfo.format = mVkSwapChainImageFormat;
			createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			createInfo.subresourceRange.baseMipLevel = 0;
			createInfo.subresourceRange.levelCount = 1;
			createInfo.subresourceRange.baseArrayLayer = 0;
			createInfo.subresourceRange.layerCount = 1;

			TRY(
				vkCreateImageView(logicDevice, &createInfo, nullptr, &mVkSwapChainImageViews[i]) != VK_SUCCESS,
				"Failed to create image view."
			);
		}
	}

	void SwapChainVulkan::createFramebuffers(VkDevice logicDevice, VkRenderPass renderPass) {
		mVkSwapChainFramebuffers.resize(mVkSwapChainImageViews.size());

		for (size_t i = 0; i < mVkSwapChainImageViews.size(); i++) {
			VkImageView attachments[] = {
				mVkSwapChainImageViews[i]
			};

			VkFramebufferCreateInfo framebufferInfo = {};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = renderPass;
			framebufferInfo.attachmentCount = 1;
			framebufferInfo.pAttachments = attachments;
			framebufferInfo.width = mVkSwapChainExtent.width;
			framebufferInfo.height = mVkSwapChainExtent.height;
			framebufferInfo.layers = 1;

			TRY(
				vkCreateFramebuffer(logicDevice, &framebufferInfo, nullptr, &mVkSwapChainFramebuffers[i]) != VK_SUCCESS,
				"Failed to create Framebuffer."
			);
		}
	}

	void SwapChainVulkan::destroyFramebuffers(VkDevice logicDevice) {
		for (auto framebuffer : mVkSwapChainFramebuffers) {
			vkDestroyFramebuffer(logicDevice, framebuffer, nullptr);
		}
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

	SwapChainVulkan SwapChainVulkan::create(
		VkDevice logicDevice,
		SwapChainSupportDetails scsd,
		VkSurfaceKHR surface,
		QueueFamilyIndices& indices,
		uint32_t width,
		uint32_t height
	) {
		SwapChainVulkan sc = SwapChainVulkan();
		sc.init(logicDevice, scsd, surface, indices, width, height);
		return sc;
	}
	
	SwapChainVulkan SwapChainVulkan::createNonInit() {
		return SwapChainVulkan();
	}
}