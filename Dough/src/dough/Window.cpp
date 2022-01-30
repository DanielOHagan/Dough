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

		setUpCallbacks();
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
		glfwDestroyWindow(mWindowPtr);
		glfwTerminate();
	}

	void Window::setUpCallbacks() {
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
				WindowResizeEvent resizeEvent{
					*reinterpret_cast<Window*>(glfwGetWindowUserPointer(windowPtr)),
					static_cast<uint32_t>(width),
					static_cast<uint32_t>(height)
				};
				Application::get().onWindowEvent(resizeEvent);
			}
		});

		glfwSetWindowFocusCallback(mWindowPtr, [](GLFWwindow* windowPtr, int focused) {
			WindowFocusChangeEvent focusChangeEvent{
				reinterpret_cast<Window&>(windowPtr),
				focused == GLFW_TRUE
			};
			Application::get().onWindowEvent(focusChangeEvent);
		});

		glfwSetWindowCloseCallback(mWindowPtr, [](GLFWwindow* windowPtr) {
			WindowCloseEvent closeEvent{ reinterpret_cast<Window&>(windowPtr) };
			Application::get().onWindowEvent(closeEvent);
		});

		glfwSetKeyCallback(mWindowPtr, [](GLFWwindow* windowPtr, int key, int scanCode, int action, int mods) {
			switch (action) {
				case GLFW_PRESS: {
					KeyDownEvent keyDown(key);
					Application::get().onKeyEvent(keyDown);
					break;
				}

				case GLFW_RELEASE: {
					KeyUpEvent keyUp(key);
					Application::get().onKeyEvent(keyUp);
					return;
				}

				case GLFW_REPEAT: {
					return;
				}

				default: {
					THROW("Unknown action type");
					return;
				}
			}
		});

		glfwSetCursorPosCallback(mWindowPtr, [](GLFWwindow* windowPtr, double posX, double posY) {
			MouseMoveEvent move((float)posX, (float)posY);
			Application::get().onMouseEvent(move);
		});

		glfwSetMouseButtonCallback(mWindowPtr, [](GLFWwindow* windowPtr, int button, int action, int mods) {
			switch (action) {
				case GLFW_PRESS: {
					MouseButtonDownEvent mouseBtnDown(button);
					Application::get().onMouseEvent(mouseBtnDown);
					return;
				}
				case GLFW_RELEASE: {
					MouseButtonUpEvent mouseBtnUp(button);
					Application::get().onMouseEvent(mouseBtnUp);
					return;
				}

				default: {
					THROW("Unknown mouse button action");
					return;
				}
			}
		});

		glfwSetScrollCallback(mWindowPtr, [](GLFWwindow* windowPtr, double offsetX, double offsetY) {
			MouseScrollEvent scroll((float)offsetX, (float)offsetY);
			Application::get().onMouseEvent(scroll);
		});
	}
}
