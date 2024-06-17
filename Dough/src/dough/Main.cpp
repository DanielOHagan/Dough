#include "dough/application/Application.h"
#include "editor/EditorAppLogic.h"
#include "editor/DemoLiciousAppLogic.h"

#include "dough/EmptyDefines.h"
#include "tracy/public/tracy/Tracy.hpp"

#ifdef _DEBUG
	#undef DOH_DEBUG_DEFINES_EMPTY
	
	//#define DOH_DEBUG_MEM_USE_CRT
	#define DOH_DEBUG_MEM_USE_DOH
	//#define DOH_DEBUG_MEM_PREFER_CRT
	//#define DOH_DEBUG_MEM_PREFER_DOH

	#include "dough/Debug.h"
#endif

int main() {

	TracySetProgramName("Dough Editor");

	//DOH_DEBUG_MEM_TRACK_START;

	int code = DOH::Application::start(
		std::make_shared<DOH::EDITOR::EditorAppLogic>(
			std::make_shared<DOH::EDITOR::DemoLiciousAppLogic>()
		),
		{
			//Title
			"Dough Editor",
			//Resolution
			1920, 1080,
			//Window display mode
			DOH::EWindowDisplayMode::WINDOWED,
			//Target forground FPS
			144.0f,
			//Target foground UPS
			144.0f
		}
	);

	//DOH_DEBUG_MEM_TRACK_END;

	//DOH_DEBUG_MEM_DUMP_TRACK;
	//DOH_DEBUG_MEM_DUMP_LEAKS;

	return code;
}
