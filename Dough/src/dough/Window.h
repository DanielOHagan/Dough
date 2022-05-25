#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include <string>

namespace DOH {

	//TODO:: monitor connect/disconnect callback handling

	enum class EWindowDisplayMode {
		WINDOWED,
		BORDERLESS_FULLSCREEN,
		FULLSCREEN
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
		bool shouldClose() const;
		void pollEvents();
		void selectDisplayMode(const EWindowDisplayMode displayMode);
		void setResolution(uint32_t width, uint32_t height);
		VkSurfaceKHR createVulkanSurface(VkInstance vulkanInstance);
		std::vector<const char*> getRequiredExtensions(bool validationLayersEnabled);
		void close();
		void iconify();
		void selectMonitor(int selectedMonitorIndex);
		const std::string& getSelectedMonitorName() const;
		const std::vector<std::string> getAllAvailableMonitorNames() const;

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
