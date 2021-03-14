#include "dough/rendering/RenderingContextVulkan.h"

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

	RenderingContextVulkan::RenderingContextVulkan()
	:	mInstance(VK_NULL_HANDLE),
		mPhysicalDevice(VK_NULL_HANDLE),
		mLogicDevice(VK_NULL_HANDLE),
		mSurface(VK_NULL_HANDLE),
		mDebugMessenger(nullptr) {
	}

	void RenderingContextVulkan::init(GLFWwindow* windowPtr, int width, int height) {
		createVulkanInstance();

		setupDebugMessenger();

		createSurface(windowPtr);

		pickPhysicalDevice();
		createLogicalDevice();

		mQueueFamilyIndices = queryQueueFamilies(mPhysicalDevice);

		SwapChainSupportDetails scSupport = SwapChainVulkan::querySwapChainSupport(mPhysicalDevice, mSurface);
		mRenderingPipeline.init(mLogicDevice, mPhysicalDevice, scSupport, mSurface, mQueueFamilyIndices, width, height);
	}

	void RenderingContextVulkan::drawFrame() {
		mRenderingPipeline.drawFrame();
	}

	void RenderingContextVulkan::close() {
		vkDeviceWaitIdle(mLogicDevice);

		mRenderingPipeline.close();

		vkDestroyDevice(mLogicDevice, nullptr);

		if (mValidationLayersEnabled) {
			DestroyDebugUtilsMessengerEXT(mInstance, mDebugMessenger, nullptr);
		}

		vkDestroySurfaceKHR(mInstance, mSurface, nullptr);
		vkDestroyInstance(mInstance, nullptr);
	}

	bool RenderingContextVulkan::isClosed() {
		return mInstance == VK_NULL_HANDLE;
	}

	void RenderingContextVulkan::resizeSwapChain(int width, int height) {
		SwapChainSupportDetails scSupport = SwapChainVulkan::querySwapChainSupport(mPhysicalDevice, mSurface);
		mRenderingPipeline.resizeSwapChain(scSupport, mSurface, mQueueFamilyIndices, width, height);
	}

	void RenderingContextVulkan::createVulkanInstance() {
		TRY(
			mValidationLayersEnabled && !checkValidationLayerSupport(),
			"Not all required validation layers are available."
		);

		VkApplicationInfo appInfo = {};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Hello Triangle";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "No Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0;

		VkInstanceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;

		auto extensions = getRequiredExtensions();
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

	bool RenderingContextVulkan::checkValidationLayerSupport() {
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

	std::vector<const char*> RenderingContextVulkan::getRequiredExtensions() {
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensionNames;
		glfwExtensionNames = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		std::vector<const char*> extensionNames(glfwExtensionNames, glfwExtensionNames + glfwExtensionCount);

		if (mValidationLayersEnabled) {
			extensionNames.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}

		return extensionNames;
	}

	void RenderingContextVulkan::setupDebugMessenger() {
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

	void RenderingContextVulkan::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
		createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity =
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType =
			VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = debugCallback;
	}

	void RenderingContextVulkan::createSurface(GLFWwindow* windowPtr) {
		TRY(
			glfwCreateWindowSurface(mInstance, windowPtr, nullptr, &mSurface) != VK_SUCCESS,
			"Failed to create Surface."
		);
	}

	void RenderingContextVulkan::pickPhysicalDevice() {
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

	bool RenderingContextVulkan::isDeviceSuitable(VkPhysicalDevice device) {
		QueueFamilyIndices indices = queryQueueFamilies(device);

		bool requiredExtensionsSupported = checkDeviceExtensionSupport(device);

		bool swapChainAdequate = false;
		if (requiredExtensionsSupported) {
			SwapChainSupportDetails swapChainSupport = SwapChainVulkan::querySwapChainSupport(device, mSurface);
			swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
		}

		return indices.isComplete() && requiredExtensionsSupported && swapChainAdequate;
	}

	QueueFamilyIndices RenderingContextVulkan::queryQueueFamilies(VkPhysicalDevice device) {
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

	bool RenderingContextVulkan::checkDeviceExtensionSupport(VkPhysicalDevice device) {
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

	void RenderingContextVulkan::createLogicalDevice() {
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

	uint32_t RenderingContextVulkan::findPhysicalDeviceMemoryType(
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

		TRY(true, "Failed to find suitable memory type.");

		return 0;
	}
}