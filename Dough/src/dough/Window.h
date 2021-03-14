#pragma once

#include "dough/rendering/RenderingContextVulkan.h"

namespace DOH {

	class Window {

	private:

		uint32_t mWidth;
		uint32_t mHeight;
		GLFWwindow* mWindowPtr;

		RenderingContextVulkan mRenderingContext;

	public:
		Window(uint32_t width, uint32_t height);

		void init();
		void close();

		bool shouldClose() const;

		void pollEvents();

		void drawFrame();

		inline uint32_t getWidth() const { return mWidth; }
		inline uint32_t getHeight() const { return mHeight; }
	};
}