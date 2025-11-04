#pragma once

#include <GLFW/glfw3.h>

#include <vector>
#include <string>
#include <array>

namespace DOH {

	enum class EWindowDisplayMode {
		WINDOWED,
		BORDERLESS_FULLSCREEN,
		FULLSCREEN
	};

	static std::array<const char*, 3> EWindowDisplayModeStrings {
		"WINDOWED",
		"BORDERLESS_FULLSCREEN",
		"FULLSCREEN"
	};

	class Window {
	private:
		uint32_t mWidth;
		uint32_t mHeight;
		GLFWwindow* mWindowPtr;
		EWindowDisplayMode mCurrentDisplayMode;

		GLFWmonitor* mSelectedMonitor;
		std::vector<std::pair<GLFWmonitor*, std::string>> mAvailableMonitors;

	public:
		Window(uint32_t width, uint32_t height, EWindowDisplayMode displayMode);

		Window(const Window& copy) = delete;
		Window operator=(const Window& assignment) = delete;

		void init(const std::string& windowTitle);
		inline bool shouldClose() const { return glfwWindowShouldClose(mWindowPtr); };
		inline void pollEvents() { glfwPollEvents(); }
		void selectDisplayMode(const EWindowDisplayMode displayMode);
		void setResolution(uint32_t width, uint32_t height);
		VkSurfaceKHR createVulkanSurface(VkInstance vulkanInstance);
		std::vector<const char*> getRequiredExtensions(bool validationLayersEnabled);
		void close();
		void iconify();
		void selectMonitor(int selectedMonitorIndex);
		void setTitle(const std::string& title);
		const std::string& getSelectedMonitorName() const;
		const std::vector<std::pair<GLFWmonitor*, std::string>> getAllAvailableMonitors() const { return mAvailableMonitors; }

		inline uint32_t getWidth() const { return mWidth; }
		inline uint32_t getHeight() const { return mHeight; }
		inline GLFWwindow* getNativeWindow() const { return mWindowPtr; }
		inline EWindowDisplayMode getDisplayMode() const { return mCurrentDisplayMode; }

	private:
		void setUpCallbacks();
		void changeToDisplayModeWindowed();
		void changeToDisplayModeBorderlessFullscreen();
		void changeToDisplayModeFullscreen();
	};
}
