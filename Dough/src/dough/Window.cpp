#include "dough/Window.h"

#include "dough/Utils.h"

namespace DOH {

	Window::Window(uint32_t width, uint32_t height)
	:	mWidth(width),
		mHeight(height),
		mWindowPtr(nullptr),
		mRenderingContext() {
		TRY(width < 0 || height < 0, "Window width and height must be greater than 0.");
	}

	void Window::init() {
		TRY(!glfwInit(), "Failed to initialise GLFW.");

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

		mWindowPtr = glfwCreateWindow(mWidth, mHeight, "VulkanWindow", nullptr, nullptr);

		TRY(mWindowPtr == nullptr, "Failed to create GLFW window.");

		glfwSetWindowUserPointer(mWindowPtr, this);

		mRenderingContext.init(mWindowPtr, mWidth, mHeight);

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
				window->mRenderingContext.resizeSwapChain(width, height);
			}
			});
	}

	bool Window::shouldClose() const {
		return glfwWindowShouldClose(mWindowPtr);
	}

	void Window::pollEvents() {
		glfwPollEvents();
	}

	void Window::drawFrame() {
		mRenderingContext.drawFrame();
	}

	void Window::close() {
		if (!mRenderingContext.isClosed()) {
			mRenderingContext.close();
		}

		glfwDestroyWindow(mWindowPtr);
		glfwTerminate();
	}
}