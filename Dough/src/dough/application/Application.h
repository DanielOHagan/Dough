#pragma once

#define DEBUG_MEM_USE_DOH

#include "dough/Utils.h"
#include "dough/Window.h"
#include "dough/rendering/RendererVulkan.h"

namespace DOH {

	class Application {

	private:

		static Application* INSTANCE;

		std::unique_ptr<Window> mWindow;
		std::unique_ptr<RendererVulkan> mRenderer;
		bool mRunning;

	public:
		Application(const Application& copy) = delete;
		void operator=(const Application& assignment) = delete;

		void run();
		void stop() { mRunning = false; }

		inline RendererVulkan& getRenderer() const { return *mRenderer; }
		inline Window& getWindow() const { return *mWindow; }

		static Application& get() { return *INSTANCE; }
		static int start();
		static bool isInstantiated() { return INSTANCE != nullptr; };

#ifdef DEBUG_MEM_USE_DOH
	public:
		uint32_t totalAllocCount = 0;
		uint32_t totalAllocSize = 0;
		uint32_t totalFreeCount = 0;
		uint32_t totalFreeSize = 0;
		void debug_addAllocCountAndSize(uint32_t count, uint32_t size) {
			totalAllocCount += count;
			totalAllocSize += size;
		}
		void debug_addAlloc(size_t size) {
			totalAllocCount++;
			totalAllocSize += static_cast<uint32_t>(size);
		}
		void debug_addFreed(size_t size) {
			totalFreeCount++;
			totalFreeSize += static_cast<uint32_t>(size);
		}
		uint32_t debug_getCurrentMemUsage() {
			return totalAllocSize - totalFreeSize;
		}
		void debug_printSize(const char* label , uint32_t size) {
			std::cout << label << ": " << size << std::endl;
		}
		void debug_printAllocInfo() {
			std::cout << "Total Alloc Count: " << totalAllocCount << std::endl;
			std::cout << "Total Alloc Size (Bytes): " << totalAllocSize << std::endl;
			std::cout << "Total Freed Count: " << totalFreeCount << std::endl;
			std::cout << "Total Freed Size (Bytes): " << totalFreeSize << std::endl;
			std::cout << "Total Difference Count: " << totalAllocCount - totalFreeCount << std::endl;
			std::cout << "Total Difference Size (Bytes): " << totalAllocSize - totalFreeSize << std::endl;
		}
#endif

	private:
		Application();

		void init();
		void mainLoop();
		void update();
		void render();
		void close();
	};
}
