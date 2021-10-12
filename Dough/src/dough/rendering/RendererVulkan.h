#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan_core.h>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

#include <iostream>

#include "dough/rendering/Config.h"
#include "dough/rendering/RenderingContextVulkan.h"

namespace DOH {

	class RendererVulkan {

	private:

		const std::vector<const char*> mValidationLayers = {
			"VK_LAYER_KHRONOS_validation"
		};

		const std::vector<const char*> mDeviceExtensions = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};

		QueueFamilyIndices mQueueFamilyIndices;

		VkInstance mInstance;
		VkPhysicalDevice mPhysicalDevice;
		VkDevice mLogicDevice;
		VkDebugUtilsMessengerEXT mDebugMessenger;

		//Move to context?
		VkSurfaceKHR mSurface;

		std::unique_ptr<RenderingContextVulkan> mRenderingContext;

#if defined (_DEBUG)
		const bool mValidationLayersEnabled = true;
#else
		const bool mValidationLayersEnabled = false;
#endif

	public:

		RendererVulkan();

		void init(int width, int height);
		void close();

		bool isClosed();

		//RenderingPipelineVulkan createRenderingPipeline(args);
		void resizeSwapChain(int width, int height);

		void drawFrame();

		inline RenderingContextVulkan& getContext() const { return *mRenderingContext; }

		static uint32_t findPhysicalDeviceMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);

	private:

		//-----Initialisation-----
		void createVulkanInstance();
		bool checkValidationLayerSupport();

		//-----Debug Initialisation-----
		void setupDebugMessenger();
		void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

		//-----Phyiscal and Logic Devices-----
		void pickPhysicalDevice();
		bool isDeviceSuitable(VkPhysicalDevice device);
		QueueFamilyIndices queryQueueFamilies(VkPhysicalDevice device);
		bool checkDeviceExtensionSupport(VkPhysicalDevice device);
		void createLogicalDevice();

		static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
			VkDebugUtilsMessageSeverityFlagBitsEXT msgSeverity,
			VkDebugUtilsMessageTypeFlagsEXT msgType,
			const VkDebugUtilsMessengerCallbackDataEXT* callbackDataPtr,
			void* userDataPtr
		) {
			std::cerr << "Validation layer: " << callbackDataPtr->pMessage << std::endl;

			return VK_FALSE;
		}
	};
}
