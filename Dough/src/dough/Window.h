#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>

namespace DOH {

	class Window {

	private:
		uint32_t mWidth;
		uint32_t mHeight;
		GLFWwindow* mWindowPtr;

	public:
		Window(uint32_t width, uint32_t height);

		Window(const Window& copy) = delete;
		Window operator=(const Window& assignment) = delete;

		void init();
		bool shouldClose() const;
		void pollEvents();
		VkSurfaceKHR createVulkanSurface(VkInstance vulkanInstance);
		std::vector<const char*> getRequiredExtensions(bool validationLayersEnabled);
		void close();

		inline uint32_t getWidth() const { return mWidth; }
		inline uint32_t getHeight() const { return mHeight; }
	};
}
