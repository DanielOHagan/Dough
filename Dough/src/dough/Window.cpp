#include "dough/Window.h"

#include "dough/Utils.h"
#include "dough/application/Application.h"

namespace DOH {

	Window::Window(uint32_t width, uint32_t height)
	:	mWidth(width),
		mHeight(height),
		mWindowPtr(nullptr)
	{
		TRY(width < 0 || height < 0, "Window width and height must be greater than 0.");
	}

	void Window::init() {
		TRY(!glfwInit(), "Failed to initialise GLFW.");

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

		mWindowPtr = glfwCreateWindow(mWidth, mHeight, "VulkanWindow", nullptr, nullptr);

		TRY(mWindowPtr == nullptr, "Failed to create GLFW window");

		glfwSetWindowUserPointer(mWindowPtr, this);

		//Setup callbacks
		glfwSetWindowSizeCallback(mWindowPtr, [](GLFWwindow* windowPtr, int width, int height) {
			if (width > 0 && height > 0) {
				auto window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(windowPtr));
				window->mWidth = width;
				window->mHeight = height;
			}
		});

		glfwSetFramebufferSizeCallback(mWindowPtr, [](GLFWwindow* windowPtr, int width, int height) {
			if (width > 0 && height > 0) {
				auto window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(windowPtr));
				Application::get().getRenderer().resizeSwapChain(width, height);
			}
		});

		glfwSetWindowCloseCallback(mWindowPtr, [](GLFWwindow* windowPtr) {
			Application::get().stop();
		});
	}

	bool Window::shouldClose() const {
		return glfwWindowShouldClose(mWindowPtr);
	}

	void Window::pollEvents() {
		glfwPollEvents();
	}

	VkSurfaceKHR Window::createVulkanSurface(VkInstance vulkanInstance) {
		VkSurfaceKHR surface = VK_NULL_HANDLE;

		TRY(
			glfwCreateWindowSurface(vulkanInstance, mWindowPtr, nullptr, &surface) != VK_SUCCESS,
			"Failed to create Surface."
		);

		return surface;
	}

	std::vector<const char*> Window::getRequiredExtensions(bool validationLayersEnabled) {
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensionNames;
		glfwExtensionNames = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		std::vector<const char*> extensionNames(glfwExtensionNames, glfwExtensionNames + glfwExtensionCount);

		if (validationLayersEnabled) {
			extensionNames.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}

		return extensionNames;
	}

	void Window::close() {
		//if (!mRenderer.isClosed()) {
		//	mRenderer.close();
		//}

		glfwDestroyWindow(mWindowPtr);
		glfwTerminate();
	}
}
