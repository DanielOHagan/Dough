#include "dough/rendering/SwapChainVulkan.h"

#include "dough/application/Application.h"
#include "dough/Logging.h"
#include "dough/time/Time.h"

#include <tracy/public/tracy/Tracy.hpp>

namespace DOH {

	SwapChainVulkan::SwapChainVulkan(
		VkDevice logicDevice,
		SwapChainCreationInfo& scCreate
	) : mSwapChain(VK_NULL_HANDLE),
		mImageFormat(VK_FORMAT_UNDEFINED),
		mExtent({}),
		mResizable(true)
	{
		init(logicDevice, scCreate);
	}

	SwapChainVulkan::~SwapChainVulkan() {
		ZoneScoped;

		if (isUsingGpuResource()) {
			LOG_ERR(
				"SwapChain GPU resource NOT released before destructor was called." <<
				"Handle: " << mSwapChain << " Format: " << mImageFormat
			);

			//NOTE:: This is to stop the IGPUResource::~IGPUReource from logging a misleading error message.
			mUsingGpuResource = false;
		}
	}

	void SwapChainVulkan::init(VkDevice logicDevice, const SwapChainCreationInfo& scCreate) {
		ZoneScoped;

		createSwapChainKHR(logicDevice, scCreate);
	}

	void SwapChainVulkan::close(VkDevice logicDevice) {
		ZoneScoped;

		for (auto imageView : mImageViews) {
			vkDestroyImageView(logicDevice, imageView, nullptr);
		}

		//NOTE:: destroying presentable images is handled based on device implementation,
		// vkDestroySwapchainKHR allows device implementation to handle the destruction of
		// images associated with the given swapchain (see Vulkan Spec for info)

		vkDestroySwapchainKHR(logicDevice, mSwapChain, nullptr);

		mUsingGpuResource = false;
	}

	void SwapChainVulkan::resize(VkDevice logicDevice, const SwapChainCreationInfo& scCreate) {
		ZoneScoped;

		for (auto imageView : mImageViews) {
			vkDestroyImageView(logicDevice, imageView, nullptr);
		}

		createSwapChainKHR(logicDevice, scCreate);
	}

	uint32_t SwapChainVulkan::aquireNextImageIndex(
		VkDevice logicDevice,
		VkFence frameInFlightFence,
		VkSemaphore imageAvailableSemaphore
	) {
		ZoneScoped;

		vkWaitForFences(logicDevice, 1, &frameInFlightFence, VK_TRUE, Time::ONE_SECOND_MILLIS);

		uint32_t imageIndex;
		vkAcquireNextImageKHR(
			logicDevice,
			mSwapChain,
			Time::ONE_SECOND_MILLIS,
			imageAvailableSemaphore,
			VK_NULL_HANDLE,
			&imageIndex
		);

		return imageIndex;
	}

	SwapChainSupportDetails SwapChainVulkan::querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface) {
		ZoneScoped;

		SwapChainSupportDetails details = {};

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
		ZoneScoped;

		for (const auto& availableFormat : availableFormats) {
			if (
				availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
				availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
			) {
				return availableFormat;
			}
		}

		LOG_WARN("Desired surface format (format: VK_FORMAT_B8G8R8A8_SRGB, colorSpace: VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) not found, falling back on first available format.");

		return availableFormats[0];
	}

	VkPresentModeKHR SwapChainVulkan::chooseSwapPresentMode(
		const std::vector<VkPresentModeKHR>& availablePresentModes,
		VkPresentModeKHR desiredPresentMode,
		bool fallbackToImmediatePresentMode
	) {
		ZoneScoped;

		for (const auto& availablePresentMode : availablePresentModes) {
			if (availablePresentMode == desiredPresentMode) {
				//LOG_INFO("Desired present mode found:" << SwapChainVulkan::getPresentModeAsString(desiredPresentMode));
				return availablePresentMode;
			}
		}

		if (fallbackToImmediatePresentMode) {
			LOG_WARN("Desired present mode " << SwapChainVulkan::getPresentModeAsString(desiredPresentMode) << " not found, falling back on enabled optional default VK_PRESENT_MODE_IMMEDIATE_KHR");
			return VK_PRESENT_MODE_IMMEDIATE_KHR;
		}

		LOG_WARN("Desired present mode " << SwapChainVulkan::getPresentModeAsString(desiredPresentMode) << " not found, falling back on default VK_PRESENT_MODE_FIFO_KHR");
		return VK_PRESENT_MODE_FIFO_KHR;
	}

	VkExtent2D SwapChainVulkan::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, uint32_t width, uint32_t height) {
		ZoneScoped;

		if (capabilities.currentExtent.width != UINT32_MAX) {
			return capabilities.currentExtent;
		} else {
			VkExtent2D actualExtent = { width, height };

			actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
			actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

			return actualExtent;
		}
	}

	void SwapChainVulkan::createSwapChainKHR(VkDevice logicDevice, const SwapChainCreationInfo& scCreate) {
		ZoneScoped;

		const uint32_t width = scCreate.getWidth();
		const uint32_t height = scCreate.getHeight();

		TRY(width < 1 || height < 1, "Swap Chain Width and Height must be larger than 0.");


		//TODO:: These device specifications are called each swapchain creation & recreation. 
		//	Should this be limited to the first time the swap chain is created and if/when the device changes.
		VkSurfaceFormatKHR surfaceFormat = SwapChainVulkan::chooseSwapSurfaceFormat(scCreate.SupportDetails.formats);
		VkPresentModeKHR presentMode = SwapChainVulkan::chooseSwapPresentMode(
			scCreate.SupportDetails.presentModes,
			scCreate.DesiredPresentMode,
			scCreate.FallbackToImmediatePresentMode
		);
		VkExtent2D extent = SwapChainVulkan::chooseSwapExtent(scCreate.SupportDetails.capabilities, width, height);

		uint32_t imageCount = std::min(2u, scCreate.SupportDetails.capabilities.minImageCount + 1); //Set default image count to 2
		if (scCreate.SupportDetails.capabilities.maxImageCount > 0 && imageCount > scCreate.SupportDetails.capabilities.maxImageCount) {
			imageCount = scCreate.SupportDetails.capabilities.maxImageCount;
		}

		VkSwapchainCreateInfoKHR createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = scCreate.Surface;
		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		uint32_t queueFamilyIndices[] = { scCreate.Indices.GraphicsFamily.value(), scCreate.Indices.PresentFamily.value() };

		if (scCreate.Indices.GraphicsFamily != scCreate.Indices.PresentFamily) {
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		} else {
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			createInfo.queueFamilyIndexCount = 0; //Optional
			createInfo.pQueueFamilyIndices = nullptr; //Optional
		}

		createInfo.preTransform = scCreate.SupportDetails.capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE;
		createInfo.oldSwapchain = mSwapChain;

		VkSwapchainKHR swapChain = VK_NULL_HANDLE;

		VK_TRY(
			vkCreateSwapchainKHR(logicDevice, &createInfo, nullptr, &swapChain),
			"Failed to create swap chain."
		);

		if (mSwapChain != VK_NULL_HANDLE) {
			vkDestroySwapchainKHR(logicDevice, mSwapChain, nullptr);
		}

		mSwapChain = swapChain;

		vkGetSwapchainImagesKHR(logicDevice, mSwapChain, &imageCount, nullptr);
		mImages.resize(imageCount);
		vkGetSwapchainImagesKHR(logicDevice, mSwapChain, &imageCount, mImages.data());

		//Store swap chain info for later.
		mImageFormat = surfaceFormat.format;
		mExtent = extent;

		mImageViews.resize(imageCount);
		const auto& context = Application::get().getRenderer().getContext();

		for (size_t i = 0; i < imageCount; i++) {
			mImageViews[i] = context.createImageView(mImages[i], mImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
		}

		mUsingGpuResource = true;
	}

	const char* SwapChainVulkan::getPresentModeAsString(VkPresentModeKHR presentMode) {
		//Taken from doc: https://registry.khronos.org/vulkan/specs/latest/man/html/VkPresentModeKHR.html
		switch (presentMode) {
			case VK_PRESENT_MODE_IMMEDIATE_KHR: return "VK_PRESENT_MODE_IMMEDIATE_KHR (0)";
			case VK_PRESENT_MODE_MAILBOX_KHR: return "VK_PRESENT_MODE_MAILBOX_KHR (1)";
			case VK_PRESENT_MODE_FIFO_KHR: return "VK_PRESENT_MODE_FIFO_KHR (2)";
			case VK_PRESENT_MODE_FIFO_RELAXED_KHR: return "VK_PRESENT_MODE_FIFO_RELAXED_KHR (3)"; 
			case VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR: return "VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR (1000111000)"; // Provided by VK_KHR_shared_presentable_image
			case VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR: return "VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR (1000111001)"; // Provided by VK_KHR_shared_presentable_image
			//case VK_PRESENT_MODE_FIFO_LATEST_READY_KHR: return "VK_PRESENT_MODE_FIFO_LATEST_READY_KHR (1000361000)"; // Provided by VK_KHR_present_mode_fifo_latest_ready
			//case VK_PRESENT_MODE_FIFO_LATEST_READY_EXT: return "VK_PRESENT_MODE_FIFO_LATEST_READY_EXT (VK_PRESENT_MODE_FIFO_LATEST_READY_KHR)"; // Provided by VK_EXT_present_mode_fifo_latest_ready
			default: return "Unknown present mode"; //This should NOT happen!
		}
	}
}
