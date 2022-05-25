#include "dough/Window.h"

#include "dough/Logging.h"
#include "dough/application/Application.h"

namespace DOH {

	Window::Window(uint32_t width, uint32_t height, EWindowDisplayMode displayMode)
	:	mWidth(width),
		mHeight(height),
		mWindowPtr(nullptr),
		mCurrentDisplayMode(displayMode),
		mSelectedMonitor(nullptr)
	{
		TRY(width < 0 || height < 0, "Window width and height must be greater than 0.");
	}

	void Window::init(const std::string& windowTitle) {
		TRY(!glfwInit(), "Failed to initialise GLFW.");

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

		int monitorCount = 0;
		GLFWmonitor** monitors = glfwGetMonitors(&monitorCount);

		if (monitors == NULL) {
			THROW("Unable to find any monitors");
		}

		//Store monitor names with index appended as names aren't guaranteed to be unique
		for (int i = 0; i < monitorCount; i++) {
			std::string monitorName = glfwGetMonitorName(*(monitors + i));
			monitorName.append(" (" + std::to_string(i) + ")");
			mAvailableMonitors.push_back({ *(monitors + i), monitorName });
		}

		mSelectedMonitor = glfwGetPrimaryMonitor();

		int vidModeCount = 0;
		const GLFWvidmode* vidMode = glfwGetVideoModes(mSelectedMonitor, &vidModeCount);
		std::vector<int> refreshRates;
		for (int i = 0; i < vidModeCount; i++) {
			refreshRates.push_back((vidMode + i)->refreshRate);
		}

		if (mCurrentDisplayMode == EWindowDisplayMode::BORDERLESS_FULLSCREEN) {
			mSelectedMonitor = glfwGetPrimaryMonitor();
			const GLFWvidmode* videoMode = glfwGetVideoMode(mSelectedMonitor);
			glfwWindowHint(GLFW_RED_BITS, videoMode->redBits);
			glfwWindowHint(GLFW_GREEN_BITS, videoMode->greenBits);
			glfwWindowHint(GLFW_BLUE_BITS, videoMode->blueBits);
			glfwWindowHint(GLFW_REFRESH_RATE, videoMode->refreshRate);
		}

		mWindowPtr = glfwCreateWindow(
			mWidth,
			mHeight,
			windowTitle.c_str(),
			mCurrentDisplayMode != EWindowDisplayMode::WINDOWED ? mSelectedMonitor : nullptr,
			nullptr
		);

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

	void Window::selectDisplayMode(const EWindowDisplayMode displayMode) {
		if (displayMode != mCurrentDisplayMode) {
			switch (displayMode) {
				case EWindowDisplayMode::WINDOWED:
				{
					//TODO:: keep a track of the windowed mode width/height separate from mWidth/mHeight as the latter
					// deal with viewport & swapchain resizing
					// 
					//TODO:: when switching from borderless fullscreen to windowed this fails to get window monitor

					mCurrentDisplayMode = EWindowDisplayMode::WINDOWED;
					if (mSelectedMonitor != nullptr) {
						changeToDisplayModeWindowed();
					} else {
						LOG_ERR("Failed to get selected monitor");
					}
					break;
				}
				case EWindowDisplayMode::BORDERLESS_FULLSCREEN:
				{
					mCurrentDisplayMode = EWindowDisplayMode::BORDERLESS_FULLSCREEN;
					GLFWmonitor* monitor = glfwGetPrimaryMonitor();
					if (mSelectedMonitor != nullptr) {
						changeToDisplayModeBorderlessFullscreen();
					} else {
						LOG_ERR("Failed to get selected monitor");
					}
					break;
				}
				case EWindowDisplayMode::FULLSCREEN:
				{
					mCurrentDisplayMode = EWindowDisplayMode::FULLSCREEN;
					if (mSelectedMonitor != nullptr) {
						changeToDisplayModeFullscreen();
					} else {
						LOG_ERR("Failed to get selected monitor");
					}
					break;
				}
			}
		} else {
			LOG_WARN("Already in selected display mode");
		}
	}

	void Window::selectMonitor(int selectedMonitorIndex) {
		if (selectedMonitorIndex > mAvailableMonitors.size() - 1) {
			LOG_ERR("Selected monitor index (" << selectedMonitorIndex << ") is not available");
		} else {
			mSelectedMonitor = mAvailableMonitors.at(selectedMonitorIndex).first;

			switch (mCurrentDisplayMode) {
				case EWindowDisplayMode::WINDOWED:
				{
					mCurrentDisplayMode = EWindowDisplayMode::WINDOWED;
					if (mSelectedMonitor != nullptr) {
						changeToDisplayModeWindowed();
					} else {
						LOG_ERR("Failed to get selected monitor");
					}
					break;
				}
				case EWindowDisplayMode::BORDERLESS_FULLSCREEN:
				{
					mCurrentDisplayMode = EWindowDisplayMode::BORDERLESS_FULLSCREEN;
					if (mSelectedMonitor != nullptr) {
						changeToDisplayModeBorderlessFullscreen();
					} else {
						LOG_ERR("Failed to get selected monitor");
					}
					break;
				}
				case EWindowDisplayMode::FULLSCREEN:
				{
					mCurrentDisplayMode = EWindowDisplayMode::FULLSCREEN;
					if (mSelectedMonitor != nullptr) {
						changeToDisplayModeFullscreen();
					} else {
						LOG_ERR("Failed to get selected monitor");
					}
					break;
				}
			}

			const GLFWvidmode* vidMode = glfwGetVideoMode(mSelectedMonitor);
			WindowMonitorChangeEvent monitorChange{
				*reinterpret_cast<Window*>(glfwGetWindowUserPointer(mWindowPtr)),
				(uint32_t) vidMode->width,
				(uint32_t) vidMode->height,
				(uint32_t) vidMode->refreshRate,
				mCurrentDisplayMode
			};
			Application::get().onWindowEvent(monitorChange);
		}
	}

	const std::string& Window::getSelectedMonitorName() const {
		for (const auto& monitor : mAvailableMonitors) {
			if (monitor.first == mSelectedMonitor) {
				return monitor.second;
			}
		}

		return "Unable to find monitor";
	}

	const std::vector<std::string> Window::getAllAvailableMonitorNames() const {
		std::vector<std::string> names;

		for (const auto& monitor : mAvailableMonitors) {
			names.push_back(monitor.second);
		}

		return names;
	}

	void Window::setResolution(uint32_t width, uint32_t height) {
		glfwSetWindowSize(mWindowPtr, width, height);
	}

	VkSurfaceKHR Window::createVulkanSurface(VkInstance vulkanInstance) {
		VkSurfaceKHR surface = VK_NULL_HANDLE;

		VK_TRY(
			glfwCreateWindowSurface(vulkanInstance, mWindowPtr, nullptr, &surface),
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

	void Window::iconify() {
		if (!Application::get().isIconified()) {
			glfwIconifyWindow(mWindowPtr);
		}
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
					return;
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
					LOG_WARN("Unknown action type: " << action);
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
					LOG_WARN("Unknown mouse button action: " << action);
					return;
				}
			}
		});

		glfwSetScrollCallback(mWindowPtr, [](GLFWwindow* windowPtr, double offsetX, double offsetY) {
			MouseScrollEvent scroll((float)offsetX, (float)offsetY);
			Application::get().onMouseEvent(scroll);
		});

		glfwSetWindowIconifyCallback(mWindowPtr, [](GLFWwindow* windowPtr, int iconified) {
			WindowIconifyChangeEvent iconify(
				*reinterpret_cast<Window*>(glfwGetWindowUserPointer(windowPtr)),
				iconified == GLFW_TRUE
			);
			Application::get().onWindowEvent(iconify);
		});
	}

	void Window::changeToDisplayModeWindowed() {
		const GLFWvidmode* videoMode = glfwGetVideoMode(mSelectedMonitor);
		glfwWindowHint(GLFW_RED_BITS, videoMode->redBits);
		glfwWindowHint(GLFW_GREEN_BITS, videoMode->greenBits);
		glfwWindowHint(GLFW_BLUE_BITS, videoMode->blueBits);

		glfwWindowHint(GLFW_REFRESH_RATE, videoMode->refreshRate);

		int x = 0;
		int y = 0;
		glfwGetMonitorPos(mSelectedMonitor, &x, &y);

		glfwSetWindowMonitor(
			mWindowPtr,
			nullptr,
			x + 50,
			y + 50,
			1920,
			1080,
			videoMode->refreshRate
		);
	}

	void Window::changeToDisplayModeBorderlessFullscreen() {
		const GLFWvidmode* videoMode = glfwGetVideoMode(mSelectedMonitor);
		glfwWindowHint(GLFW_RED_BITS, videoMode->redBits);
		glfwWindowHint(GLFW_GREEN_BITS, videoMode->greenBits);
		glfwWindowHint(GLFW_BLUE_BITS, videoMode->blueBits);
		glfwWindowHint(GLFW_REFRESH_RATE, videoMode->refreshRate);

		int x = 0;
		int y = 0;
		glfwGetMonitorPos(mSelectedMonitor, &x, &y);

		glfwSetWindowMonitor(
			mWindowPtr,
			nullptr,
			x,
			y,
			videoMode->width,
			videoMode->height,
			videoMode->refreshRate
		);
	}
	void Window::changeToDisplayModeFullscreen() {
		const GLFWvidmode* videoMode = glfwGetVideoMode(mSelectedMonitor);
		glfwWindowHint(GLFW_RED_BITS, videoMode->redBits);
		glfwWindowHint(GLFW_GREEN_BITS, videoMode->greenBits);
		glfwWindowHint(GLFW_BLUE_BITS, videoMode->blueBits);

		glfwWindowHint(GLFW_REFRESH_RATE, videoMode->refreshRate);

		int x = 0;
		int y = 0;
		glfwGetMonitorPos(mSelectedMonitor, &x, &y);

		glfwSetWindowMonitor(
			mWindowPtr,
			mSelectedMonitor,
			x,
			y,
			videoMode->width,
			videoMode->height,
			videoMode->refreshRate
		);
	}
}
