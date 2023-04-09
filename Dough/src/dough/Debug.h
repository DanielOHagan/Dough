#pragma once

#include "dough/Logging.h"

//NOTE:: static members that alloc heap memory do not free it before DEBUG_MEM_DUMP is called in Main.cpp
#if not defined DOH_DEBUG_DEFINES_EMPTY
	#ifdef DOH_DEBUG_MEM_USE_CRT
		#define _CRTDBG_MAP_ALLOC
		#include <stdlib.h>
		#include <crtdbg.h>

		#undef DOH_DEBUG_MEM_TRACK_START
		#define DOH_DEBUG_MEM_TRACK_START _CrtMemState start; _CrtMemCheckpoint(&start);
		#undef DOH_DEBUG_MEM_TRACK_END
		#define DOH_DEBUG_MEM_TRACK_END _CrtMemState end; _CrtMemCheckpoint(&end);

		//Dump tracked memory alloc/free info
		//MUST have two _CrtMemState objects named start and end. Generally this should be done by using DEBUG_MEM_TRACK_START and DEBUG_MEM_TRACK_END
		#undef DOH_DEBUG_MEM_DUMP_TRACK
		#define DOH_DEBUG_MEM_DUMP_TRACK _CrtMemState diff; if (_CrtMemDifference(&diff, &start, &end) == 1) _CrtMemDumpStatistics(&diff);

		//Dump tracked memory leak info
		#undef DOH_DEBUG_MEM_DUMP_LEAKS
		#define DOH_DEBUG_MEM_DUMP_LEAKS\
			if (_CrtDumpMemoryLeaks() == 1)\
				LOG_ERR("Possible memory leak! Check Output-Debug for further details")\
			else LOG_INFO("No memory leak");

	//NOTE:: Since not all static members aren't guaranteed to have been allocated before new/delete overloading then they may not be tracked.
	// This may result in tracking the freeing of an object but not the allocation & vice versa.
	// To prevent this make sure all heap allocated static members allocated after the memory tracker starts are deleted before it ends.
	// For example Application::INSTANCE is allocated in its init function, which is guaranteed to call the getMemoryTracker method.
	// Whereas, static std::string members are released after the executable stops, at which point the tracker has already dumped its info.
	#elif defined DOH_DEBUG_MEM_USE_DOH
		#include <cstdint>
		#include <cstdlib>
		#include <iostream>
		#include <new>

		namespace DOH {
			struct MemoryTracker {
				size_t TotalAllocSize = 0;
				size_t TotalFreeSize = 0;

				uint32_t TotalAllocCount = 0;
				uint32_t TotalFreeCount = 0;
			};

			//Return static local variable to gurantee that there is only one and it is created on the first time this method is called.
			DOH::MemoryTracker& getMemoryTracker() {
				static DOH::MemoryTracker memTracker;
				return memTracker;
			}
		}

		void* operator new(size_t size) {

			//Avoid malloc(0)
			if (size == 0) {
				++size;
			}

			void* ptr = ptr = malloc(size);

			if (ptr == nullptr) {
				throw std::bad_alloc{};
			}

			DOH::MemoryTracker& memTracker = DOH::getMemoryTracker();

			memTracker.TotalAllocSize += size;
			memTracker.TotalAllocCount++;

			return ptr;
		}

		void* operator new[](size_t size) {

			//Avoid malloc(0)
			if (size == 0) {
				++size;
			}

			void* ptr = malloc(size);

			if (ptr == nullptr) {
				throw std::bad_alloc{};
			}

			DOH::MemoryTracker& memTracker = DOH::getMemoryTracker();

			memTracker.TotalAllocSize += size;
			memTracker.TotalAllocCount++;

			return ptr;
		}

		void operator delete(void* memory, size_t size) {
			free(memory);

			DOH::MemoryTracker& memTracker = DOH::getMemoryTracker();

			memTracker.TotalFreeSize += size;
			memTracker.TotalFreeCount++;
		}

		void operator delete[](void* memory, size_t size) {
			free(memory);

			DOH::MemoryTracker& memTracker = DOH::getMemoryTracker();

			memTracker.TotalFreeSize += size;
			memTracker.TotalFreeCount++;
		}

		inline uint32_t getCurrentTotalAllocCountDifference() {
			const DOH::MemoryTracker& memTracker = DOH::getMemoryTracker();
			return memTracker.TotalAllocCount - memTracker.TotalFreeCount;
		}

		inline size_t getCurrentTotalAllocSizeDifference() {
			const DOH::MemoryTracker& memTracker = DOH::getMemoryTracker();
			return memTracker.TotalAllocSize - memTracker.TotalFreeSize;
		}

		void dumpMemAllocInfo() {
			DOH::MemoryTracker& memTracker = DOH::getMemoryTracker();

			LOGLN_UNDERLINED("Memory Tracker info dump:");
			LOGLN_WHITE("Total Alloc Count: " << memTracker.TotalAllocCount);
			LOGLN_WHITE("Total Alloc Size (Bytes): " << memTracker.TotalAllocSize);
			LOGLN_WHITE("Total Free Count: " << memTracker.TotalFreeCount);
			LOGLN_WHITE("Total Free Size (Bytes): " << memTracker.TotalFreeSize);
		}

		void dumpMemLeakInfo() {
			DOH::MemoryTracker& memTracker = DOH::getMemoryTracker();

			if (memTracker.TotalAllocCount != memTracker.TotalFreeCount || memTracker.TotalAllocSize != memTracker.TotalFreeSize) {
				LOG_WARN("Possible mem leak detected!");
				LOG_WARN(
					"Alloc-Free count difference: " << getCurrentTotalAllocCountDifference() <<
					" (" << getCurrentTotalAllocSizeDifference() << " bytes)"
				);
				LOG_WARN("Statically stored objects are destroyed after this print so they may be identified here as a mem leak.");
			} else {
				LOGLN_GREEN("No mem leak detected");
			}
		}

		//Tracking starts when getMemoryTracker() is first called, calling it again does NOT reset the tracker
		#undef DOH_DEBUG_MEM_TRACK_START
		#define DOH_DEBUG_MEM_TRACK_START getMemoryTracker();

		//TODO:: currently nothing really stopping mem tracking, since currently only one tracker can exist it might be best to keep it alive constantly
		#undef DOH_DEBUG_MEM_TRACK_END
		#define DOH_DEBUG_MEM_TRACK_END

		#undef DOH_DEBUG_MEM_DUMP_TRACK
		#define DOH_DEBUG_MEM_DUMP_TRACK dumpMemAllocInfo();

		#undef DOH_DEBUG_MEM_DUMP_LEAKS
		#define DOH_DEBUG_MEM_DUMP_LEAKS dumpMemLeakInfo();
	#endif
#endif
