#include "dough/application/Application.h"
#include "editor/EditorAppLogic.h"

#ifdef _DEBUG
	//#define DEBUG_MEM_USE_CRT
	#define DEBUG_MEM_USE_DOH
	//#define DEBUG_MEM_PREFER_CRT
	//#define DEBUG_MEM_PREFER_DOH

	#include "dough/Debug.h"
#endif

int main() {

	DEBUG_MEM_TRACK_START;

	int code = DOH::Application::start(
		std::make_shared<DOH::EDITOR::EditorAppLogic>(),
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

	DEBUG_MEM_TRACK_END;

	DEBUG_MEM_DUMP_TRACK;
	DEBUG_MEM_DUMP_LEAKS;

	return code;
}
