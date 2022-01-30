#include "dough/application/Application.h"
#include "testGame/TG_AppLogic.h"

#define DEBUG_MEM_DUMP

#ifdef _DEBUG
	//#define DEBUG_MEM_USE_CRT
	//#define DEBUG_MEM_PREFER_CRT
	//#define DEBUG_MEM_PREFER_DOH

	#include "dough/Debug.h"
#endif

int main() {

	std::shared_ptr<DOH::IApplicationLogic> appLogic = std::make_shared<TG::TG_AppLogic>();

	int code = DOH::Application::start(appLogic);

	DEBUG_MEM_DUMP;

	return code;
}
