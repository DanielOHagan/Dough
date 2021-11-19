#include "dough/rendering/RendererVulkan.h"

#include "dough/application/Application.h"
#include "dough/Utils.h"
#include "dough/rendering/shader/ShaderVulkan.h"

#include <set>
#include <string>
#include <fstream>

namespace DOH {

	VkResult createDebugUtilsMessengerEXT(
		VkInstance vkInstance,
		const VkDebugUtilsMessengerCreateInfoEXT* createInfoPtr,
		const VkAllocationCallbacks* allocatorPtr,
		VkDebugUtilsMessengerEXT* debugMessengerPtr
	) {
		auto function = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(vkInstance, "vkCreateDebugUtilsMessengerEXT");

		if (function != nullptr) {
			return function(vkInstance, createInfoPtr, allocatorPtr, debugMessengerPtr);
		} else {
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}

	void DestroyDebugUtilsMessengerEXT(
		VkInstance vkInstance,
		VkDebugUtilsMessengerEXT debugMesenger,
		const VkAllocationCallbacks* allocatorPtr
	) {
		auto function = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(vkInstance, "vkDestroyDebugUtilsMessengerEXT");
		if (function != nullptr) {
			function(vkInstance, debugMesenger, allocatorPtr);
		}
	}

	RendererVulkan::RendererVulkan()
	:	mInstance(VK_NULL_HANDLE),
		mPhysicalDevice(VK_NULL_HANDLE),
		mLogicDevice(VK_NULL_HANDLE),
		mSurface(VK_NULL_HANDLE),
		mDebugMessenger(nullptr)
	{}

	void RendererVulkan::init(int width, int height) {
		createVulkanInstance();

		setupDebugMessenger();

		mSurface = Application::get().getWindow().createVulkanSurface(mInstance);

		pickPhysicalDevice();
		createLogicalDevice();

		mQueueFamilyIndices = queryQueueFamilies(mPhysicalDevice);
		SwapChainSupportDetails scSupport = SwapChainVulkan::querySwapChainSupport(mPhysicalDevice, mSurface);

		mRenderingContext = std::make_unique<RenderingContextVulkan>(mLogicDevice, mPhysicalDevice);
		mRenderingContext->init(scSupport, mSurface, mQueueFamilyIndices, width, height);
	}

	void RendererVulkan::drawFrame() {
		mRenderingContext->drawFrame();
	}

	void RendererVulkan::close() {
		vkDeviceWaitIdle(mLogicDevice);

		mRenderingContext->close();

		vkDestroyDevice(mLogicDevice, nullptr);

		if (mValidationLayersEnabled) {
			DestroyDebugUtilsMessengerEXT(mInstance, mDebugMessenger, nullptr);
		}

		vkDestroySurfaceKHR(mInstance, mSurface, nullptr);
		vkDestroyInstance(mInstance, nullptr);
	}

	bool RendererVulkan::isClosed() {
		return mInstance == VK_NULL_HANDLE;
	}

	void RendererVulkan::resizeSwapChain(int width, int height) {
		SwapChainSupportDetails scSupport = SwapChainVulkan::querySwapChainSupport(mPhysicalDevice, mSurface);
		mRenderingContext->resizeSwapChain(scSupport, mSurface, mQueueFamilyIndices, width, height);
	}

	void RendererVulkan::createVulkanInstance() {
		TRY(
			mValidationLayersEnabled && !checkValidationLayerSupport(),
			"Not all required validation layers are available."
		);

		VkApplicationInfo appInfo = {};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Hello Triangle";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "Dough";
		appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 1);
		appInfo.apiVersion = VK_API_VERSION_1_0;

		VkInstanceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;

		auto extensions = Application::get().getWindow().getRequiredExtensions(mValidationLayersEnabled);
		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();

		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
		if (mValidationLayersEnabled) {
			createInfo.enabledLayerCount = static_cast<uint32_t>(mValidationLayers.size());
			createInfo.ppEnabledLayerNames = mValidationLayers.data();

			populateDebugMessengerCreateInfo(debugCreateInfo);
			createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
		} else {
			createInfo.enabledLayerCount = 0;
			createInfo.pNext = nullptr;
		}

		TRY(
			vkCreateInstance(&createInfo, nullptr, &mInstance) != VK_SUCCESS,
			"Failed to create Vulkan Instance."
		);
	}

	bool RendererVulkan::checkValidationLayerSupport() {
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		for (const char* layerName : mValidationLayers) {
			bool layerFound = false;

			for (const auto& layerProperties : availableLayers) {
				if (strcmp(layerName, layerProperties.layerName) == 0) {
					layerFound = true;
					break;
				}
			}

			if (!layerFound) {
				return false;
			}
		}

		return true;
	}

	void RendererVulkan::setupDebugMessenger() {
		if (!mValidationLayersEnabled) {
			return;
		}

		VkDebugUtilsMessengerCreateInfoEXT createInfo;
		populateDebugMessengerCreateInfo(createInfo);

		TRY(
			createDebugUtilsMessengerEXT(mInstance, &createInfo, nullptr, &mDebugMessenger) != VK_SUCCESS,
			"Failed to set up Debug Messenger."
		);
	}

	void RendererVulkan::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
		createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity =
			//Only showing Warnings and Errors right now
			//VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
			//VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType =
			VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = debugCallback;
	}

	void RendererVulkan::pickPhysicalDevice() {
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(mInstance, &deviceCount, nullptr);

		TRY(deviceCount == 0, "Failed to find GPUs with Vulkan support.")

			std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(mInstance, &deviceCount, devices.data());

		for (const auto& device : devices) {
			if (isDeviceSuitable(device)) {
				mPhysicalDevice = device;
				break;
			}
		}

		TRY(mPhysicalDevice == VK_NULL_HANDLE, "Failed to find GPUs with Vulkan support.");
	}

	bool RendererVulkan::isDeviceSuitable(VkPhysicalDevice device) {
		QueueFamilyIndices indices = queryQueueFamilies(device);

		bool requiredExtensionsSupported = checkDeviceExtensionSupport(device);

		bool swapChainAdequate = false;
		if (requiredExtensionsSupported) {
			SwapChainSupportDetails swapChainSupport = SwapChainVulkan::querySwapChainSupport(device, mSurface);
			swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
		}

		VkPhysicalDeviceFeatures supportedDeviceFeatures;
		vkGetPhysicalDeviceFeatures(device, &supportedDeviceFeatures);

		return 
			indices.isComplete() &&
			requiredExtensionsSupported &&
			swapChainAdequate &&
			supportedDeviceFeatures.samplerAnisotropy;
	}

	QueueFamilyIndices RendererVulkan::queryQueueFamilies(VkPhysicalDevice device) {
		QueueFamilyIndices indices;

		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

		int i = 0;
		for (const auto& queueFamily : queueFamilies) {
			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
				indices.graphicsFamily = i;
			}

			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, mSurface, &presentSupport);

			if (presentSupport) {
				indices.presentFamily = i;
			}

			if (indices.isComplete()) {
				break;
			}

			i++;
		}

		return indices;
	}

	bool RendererVulkan::checkDeviceExtensionSupport(VkPhysicalDevice device) {
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

		std::set<std::string> requiredExtensions(mDeviceExtensions.begin(), mDeviceExtensions.end());

		for (const auto& extension : availableExtensions) {
			requiredExtensions.erase(extension.extensionName);
		}

		return requiredExtensions.empty();
	}

	void RendererVulkan::createLogicalDevice() {
		QueueFamilyIndices indices = queryQueueFamilies(mPhysicalDevice);

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};

		float queuePriority = 1.0f;
		for (uint32_t queueFamily : uniqueQueueFamilies) {
			VkDeviceQueueCreateInfo queueCreateInfo = {};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;

			queueCreateInfos.push_back(queueCreateInfo);
		}

		VkPhysicalDeviceFeatures deviceFeatures = {};
		deviceFeatures.samplerAnisotropy = VK_TRUE;

		VkDeviceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.pQueueCreateInfos = queueCreateInfos.data();
		createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		createInfo.pEnabledFeatures = &deviceFeatures;
		createInfo.enabledExtensionCount = static_cast<uint32_t>(mDeviceExtensions.size());
		createInfo.ppEnabledExtensionNames = mDeviceExtensions.data();

		if (mValidationLayersEnabled) {
			createInfo.enabledLayerCount = static_cast<uint32_t>(mValidationLayers.size());
			createInfo.ppEnabledLayerNames = mValidationLayers.data();
		} else {
			createInfo.enabledLayerCount = 0;
		}

		TRY(
			vkCreateDevice(mPhysicalDevice, &createInfo, nullptr, &mLogicDevice),
			"Failed to create logical device."
		);
	}

	uint32_t RendererVulkan::findPhysicalDeviceMemoryType(
		VkPhysicalDevice physicalDevice,
		uint32_t typeFilter,
		VkMemoryPropertyFlags properties
	) {
		VkPhysicalDeviceMemoryProperties memProps = {};
		vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProps);

		for (uint32_t i = 0; i < memProps.memoryTypeCount; i++) {
			if ((typeFilter & (1 << i)) && (memProps.memoryTypes[i].propertyFlags & properties) == properties) {
				return i;
			}
		}

		THROW("Failed to find suitable memory type.");

		return 0;
	}
}
