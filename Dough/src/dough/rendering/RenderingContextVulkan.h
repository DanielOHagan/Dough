#pragma once

#define GLFW_INCLUDE_VULKAN
#define GLM_FORCE_RADIANS
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <iostream>

#include "dough/rendering/Config.h"
#include "dough/rendering/RenderingPipelineVulkan.h"

namespace DOH {

	class RenderingContextVulkan {

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
		VkSurfaceKHR mSurface;
		RenderingPipelineVulkan mRenderingPipeline;
		VkDebugUtilsMessengerEXT mDebugMessenger;

#if defined (_DEBUG)
		const bool mValidationLayersEnabled = true;
#else
		const bool mValidationLayersEnabled = false;
#endif

	public:

		RenderingContextVulkan();

		void init(GLFWwindow* windowPtr, int width, int height);
		void close();

		bool isClosed();

		void resizeSwapChain(int width, int height);

		void drawFrame();

		static uint32_t findPhysicalDeviceMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);

	private:

		//-----Initialisation-----
		void createVulkanInstance();
		bool checkValidationLayerSupport();
		std::vector<const char*> getRequiredExtensions();

		//-----Debug Initialisation-----
		void setupDebugMessenger();
		void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

		//-----Window Specifics----- NOTE:: Need a better name/description for this section
		void createSurface(GLFWwindow* windowPtr);

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