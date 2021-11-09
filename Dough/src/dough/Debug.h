#pragma once

#ifdef DEBUG_MEM_USE_CRT
	#define _CRTDBG_MAP_ALLOC
	#include <stdlib.h>
	#include <crtdbg.h>

	#undef DEBUG_MEM_DUMP
	#define DEBUG_MEM_DUMP _CrtDumpMemoryLeaks();
	#define CRT_CHECK_COMP(before, after) { _CrtMemState diff; if (_CrtMemDifference(&diff, &before, &after)) { _CrtMemDumpStatistics(&diff); } }
#elif defined DEBUG_MEM_USE_DOH
	#include <cstdint>
	#include <cstdlib>

	//static uint32_t totalAllocSize = 0;
	//static uint32_t totalFreeSize = 0;

	//Some memory is allocation before Application is initiated, this tracks the size and count of those.
	static uint32_t preAppInitAllocSize = 0;
	static uint32_t preAppInitAllocCount = 0;
	static uint32_t preAppInitFreeSize = 0;
	static uint32_t preAppInitFreeCount = 0;

	void* operator new(size_t size) {

		void* ptr = malloc(size);

		//totalAllocSize += static_cast<uint32_t>(size);
		if (DOH::Application::isInstantiated()) {
			DOH::Application::get().debug_addAlloc(size);
		} else {
			preAppInitAllocCount++;
			preAppInitAllocSize += static_cast<uint32_t>(size);
		}

		return ptr;
	}

	void operator delete(void* memory, size_t size) {
		free(memory);

		//totalFreeSize += static_cast<uint32_t>(size);
		if (DOH::Application::isInstantiated()) {
			DOH::Application::get().debug_addFreed(size);
		} else {
			preAppInitFreeCount++;
			preAppInitFreeSize += static_cast<uint32_t>(size);
		}
	}

	void dumpPreAppInitAllocInfo() {
		std::cout << "Pre App Alloc Count: " << preAppInitAllocCount << std::endl;
		std::cout << "Pre App Alloc Size (Bytes): " << preAppInitAllocSize << std::endl;
		std::cout << "Pre App Free Count: " << preAppInitFreeCount << std::endl;
		std::cout << "Pre App Free Size (Bytes): " << preAppInitFreeSize << std::endl;
	}

	void dumpMemAllocInfo() {
		//Print apart
		//	dumpPreAppInitAllocInfo();
		//Or add count to App counter
		if (DOH::Application::isInstantiated()) {
			DOH::Application::get().debug_addAllocCountAndSize(preAppInitAllocCount, preAppInitAllocSize);
			DOH::Application::get().debug_printAllocInfo();
		}
	}

	#undef DEBUG_MEM_DUMP
	#define DEBUG_MEM_DUMP dumpMemAllocInfo();
#endif
