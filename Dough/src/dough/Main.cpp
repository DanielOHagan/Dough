#include "dough/application/Application.h"
#include "editor/EditorAppLogic.h"

#define DEBUG_MEM_DUMP

#ifdef _DEBUG
	//#define DEBUG_MEM_USE_CRT
	//#define DEBUG_MEM_PREFER_CRT
	//#define DEBUG_MEM_PREFER_DOH

	//#include "dough/Debug.h"
#endif

int main() {

	std::shared_ptr<DOH::IApplicationLogic> appLogic = std::make_shared<DOH::EDITOR::EditorAppLogic>();

	int code = DOH::Application::start(appLogic, {
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
	});

	DEBUG_MEM_DUMP;

	return code;
}
