#pragma once

#include <vulkan/vulkan_core.h>
#include <glm/glm.hpp>

#include <iostream>

#include "dough/rendering/Config.h"
#include "dough/rendering/RenderingContextVulkan.h"
#include "dough/Logging.h"

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
		//TODO:: Build config/engine init settings that have the highest C++ optimisation and Vulkan validation layers
		//const bool mValidationLayersEnabled = true;
#endif

	public:
		RendererVulkan();
		RendererVulkan(const RendererVulkan& copy) = delete;
		RendererVulkan operator=(const RendererVulkan& assignment) = delete;

		void init(Window& window);
		void close();
		inline void closeGpuResource(std::shared_ptr<IGPUResourceVulkan> res) { if (res != nullptr) mRenderingContext->addGpuResourceToClose(res); }
		inline void closeGpuResourceOwner(std::shared_ptr<IGPUResourceOwnerVulkan> resOwner) { if ( resOwner != nullptr) resOwner->addOwnedResourcesToClose(*mRenderingContext); }

		bool isReady() const;
		inline bool isClosed() const { return mInstance == VK_NULL_HANDLE; }

		void onResize(int width, int height);

		void drawFrame();
		void beginScene(ICamera& camera);
		void endScene();
		void beginUi(ICamera& camera);
		void endUi();
		inline void deviceWaitIdle(const char* msg) {
			LOG_INFO(msg);
			VK_TRY(vkDeviceWaitIdle(mLogicDevice), "Device failed to wait idle");
		}

		inline RenderingContextVulkan& getContext() const { return *mRenderingContext; }
		inline const bool areValidationLayersEnabled() const { return mValidationLayersEnabled; }
		inline const std::vector<const char*>& getValidationLayers() const { return mValidationLayers; };
		inline const std::vector<const char*>& getDeviceExtensions() const { return mDeviceExtensions; }

		static uint32_t findPhysicalDeviceMemoryType(
			VkPhysicalDevice physicalDevice,
			uint32_t typeFilter,
			VkMemoryPropertyFlags properties
		);
		static VkFormat findSupportedFormat(
			const std::vector<VkFormat>& candidates,
			VkImageTiling tiling,
			VkFormatFeatureFlags features
		);
		static bool hasStencilComponent(VkFormat format) {
			return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
		}

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

		VkFormat findSupportedFormatImpl(
			const std::vector<VkFormat>& candidates,
			VkImageTiling tiling,
			VkFormatFeatureFlags features
		);

		static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
			VkDebugUtilsMessageSeverityFlagBitsEXT msgSeverity,
			VkDebugUtilsMessageTypeFlagsEXT msgType,
			const VkDebugUtilsMessengerCallbackDataEXT* callbackDataPtr,
			void* userDataPtr
		);
	};
}
