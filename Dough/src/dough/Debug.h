#pragma once

#ifdef DEBUG_MEM_USE_CRT
	#define _CRTDBG_MAP_ALLOC
	#include <stdlib.h>
	#include <crtdbg.h>

	#undef DEBUG_MEM_DUMP
	#define DEBUG_MEM_DUMP _CrtDumpMemoryLeaks();
	#define CRT_CHECK_COMP(before, after) { _CrtMemState diff; if (_CrtMemDifference(&diff, &before, &after)) { _CrtMemDumpStatistics(&diff); } }

	//#define DEBUG_HEADER_DEFINED
#elif defined DEBUG_MEM_USE_DOH
	#include <cstdint>
	#include <cstdlib>

	//Some memory is allocated before Application is initiated, this tracks the size and count of those.
	static uint32_t preAppInitAllocSize = 0;
	static uint32_t preAppInitAllocCount = 0;
	static uint32_t preAppInitFreeSize = 0;
	static uint32_t preAppInitFreeCount = 0;
	static uint32_t appAllocSize = 0;
	static uint32_t appAllocCount = 0;
	static uint32_t appFreeSize = 0;
	static uint32_t appFreeCount = 0;

	static bool trackingMemAlloc = false;
	static uint32_t trackedMemAllocSize = 0;
	static uint32_t trackedMemAllocCount = 0;
	static uint32_t trackedMemFreedSize = 0;
	static uint32_t trackedMemFreedCount = 0;


	void* operator new(size_t size) {

		void* ptr = malloc(size);

		if (DOH::Application::isInstantiated()) {
			appAllocSize += static_cast<uint32_t>(size);
			appAllocCount++;
		} else {
			preAppInitAllocCount++;
			preAppInitAllocSize += static_cast<uint32_t>(size);
		}

		if (trackingMemAlloc) {
			trackedMemAllocSize += static_cast<uint32_t>(size);
			trackedMemAllocCount++;
		}

		return ptr;
	}

	void operator delete(void* memory, size_t size) {
		free(memory);

		if (DOH::Application::isInstantiated()) {
			appFreeSize += static_cast<uint32_t>(size);
			appFreeCount++;
		} else {
			preAppInitFreeCount++;
			preAppInitFreeSize += static_cast<uint32_t>(size);
		}

		if (trackingMemAlloc) {
			trackedMemFreedSize += static_cast<uint32_t>(size);
			trackedMemFreedCount++;
		}
	}

	void resetTrackedAllocInfo() {
		trackingMemAlloc = false;
		trackedMemAllocSize = 0;
		trackedMemAllocCount = 0;
	}

	void dumpTrackedAllocInfo() {
		std::cout << "Tracked Alloc Count: " << trackedMemAllocCount << std::endl;
		std::cout << "Tracked Alloc Size (Bytes): " << trackedMemAllocSize << std::endl;
		std::cout << "Tracked Freed Count: " << trackedMemFreedCount << std::endl;
		std::cout << "Tracked Freed Size (Bytes): " << trackedMemFreedSize << std::endl;
	}

	void dumpPreAppInitAllocInfo() {
		std::cout << "Pre App Alloc Count: " << preAppInitAllocCount << std::endl;
		std::cout << "Pre App Alloc Size (Bytes): " << preAppInitAllocSize << std::endl;
		std::cout << "Pre App Free Count: " << preAppInitFreeCount << std::endl;
		std::cout << "Pre App Free Size (Bytes): " << preAppInitFreeSize << std::endl;
	}

	void dumpCombinedAllocInfo() {
		std::cout << "Total Alloc Count: " << preAppInitAllocCount + appAllocCount << std::endl;
		std::cout << "Total Alloc Size (Bytes): " << preAppInitAllocSize + appAllocSize << std::endl;
		std::cout << "Total Free Count: " << preAppInitFreeCount + appFreeCount << std::endl;
		std::cout << "Total Free Size (Bytes): " << preAppInitFreeSize + appFreeSize << std::endl;
		std::cout << "Alloc-Free Difference Count: " << (preAppInitAllocCount + appAllocCount) - (preAppInitFreeCount + appFreeCount) << std::endl;
		std::cout << "Alloc-Free Difference Size (Bytes): " << (preAppInitAllocSize + appAllocSize) - (preAppInitFreeSize + appFreeSize) << std::endl;
	}

	void dumpMemAllocInfo() {
		//Print apart
		//	dumpPreAppInitAllocInfo();
		//Or add count to App counter
			dumpCombinedAllocInfo();
	}

	#undef DEBUG_MEM_DUMP
	#define DEBUG_MEM_DUMP dumpMemAllocInfo();
#endif
