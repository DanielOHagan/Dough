#include "dough/application/Application.h"

#define DEBUG_MEM_DUMP

#ifdef _DEBUG
	//#define DEBUG_MEM_USE_CRT
	//#define DEBUG_MEM_PREFER_CRT
	//#define DEBUG_MEM_PREFER_DOH

	#include "dough/Debug.h"
#endif

int main() {

	int code = DOH::Application::start(/* appLogic */);

	DEBUG_MEM_DUMP

	return code;
}
