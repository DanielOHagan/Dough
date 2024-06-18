#include "dough/rendering/pipeline/ShaderProgram.h"

#include <tracy/public/tracy/Tracy.hpp>

namespace DOH {

	void ShaderProgram::init(VkDevice logicDevice) {
		ZoneScoped;

		if (!mVertexShader->isModuleLoaded()) {
			mVertexShader->init(logicDevice);
		}
		if (!mFragmentShader->isModuleLoaded()) {
			mFragmentShader->init(logicDevice);
		}
	}

	void ShaderProgram::close(VkDevice logicDevice) {
		ZoneScoped;

		if (mVertexShader->isModuleLoaded()) {
			mVertexShader->close(logicDevice);
		}
		if (mFragmentShader->isModuleLoaded()) {
			mFragmentShader->close(logicDevice);
		}
	}
}
