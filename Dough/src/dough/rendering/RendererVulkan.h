#pragma once

#define GLFW_INCLUDE_VULKAN
#define GLM_FORCE_RADIANS
#include <GLFW/glfw3.h>
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

		/* TODO::
		Put RendererVulkan in to Application and have it's "output" be RenderingContext.
		Including devices into the RendererVulkan class may help differentiate the intent
			of each class, the renderer has top level control and should, in theory, work 
			without a display (the surface).
		Have the RendererVulkan class decide when to call drawFrame and have it tell the context
			class do the actual drawing (or at least the abstracted function call)
		*/
		RenderingContextVulkan mRenderingContext;

#if defined (_DEBUG)
		const bool mValidationLayersEnabled = true;
#else
		const bool mValidationLayersEnabled = false;
#endif

	public:

		RendererVulkan();

		void init(GLFWwindow* windowPtr, int width, int height);
		void close();

		bool isClosed();

		//RenderingPipelineVulkan createRenderingPipeline(args);
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