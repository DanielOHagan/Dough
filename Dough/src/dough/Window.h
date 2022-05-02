#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include <string>

namespace DOH {

	enum class WindowDisplayMode {
		WINDOWED,
		BORDERLESS_FULLSCREEN,
		FULLSCREEN
	};

	class Window {

	private:
		uint32_t mWidth;
		uint32_t mHeight;
		GLFWwindow* mWindowPtr;
		WindowDisplayMode mCurrentDisplayMode;

	public:
		Window(uint32_t width, uint32_t height, WindowDisplayMode displayMode);

		Window(const Window& copy) = delete;
		Window operator=(const Window& assignment) = delete;

		void init(const std::string& windowTitle);
		bool shouldClose() const;
		void pollEvents();
		void selectDisplayMode(const WindowDisplayMode displayMode);
		void setResolution(uint32_t width, uint32_t height);
		VkSurfaceKHR createVulkanSurface(VkInstance vulkanInstance);
		std::vector<const char*> getRequiredExtensions(bool validationLayersEnabled);
		void close();
		void iconify();

		inline uint32_t getWidth() const { return mWidth; }
		inline uint32_t getHeight() const { return mHeight; }
		inline GLFWwindow* getNativeWindow() const { return mWindowPtr; }
		inline WindowDisplayMode getDisplayMode() const { return mCurrentDisplayMode; }

	private:
		void setUpCallbacks();
	};
}
