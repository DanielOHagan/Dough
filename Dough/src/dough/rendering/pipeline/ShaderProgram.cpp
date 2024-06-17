#include "dough/rendering/pipeline/ShaderProgram.h"

namespace DOH {

	void ShaderProgram::init(VkDevice logicDevice) {
		if (!mVertexShader->isModuleLoaded()) {
			mVertexShader->init(logicDevice);
		}
		if (!mFragmentShader->isModuleLoaded()) {
			mFragmentShader->init(logicDevice);
		}
	}

	void ShaderProgram::close(VkDevice logicDevice) {
		if (mVertexShader->isModuleLoaded()) {
			mVertexShader->close(logicDevice);
		}
		if (mFragmentShader->isModuleLoaded()) {
			mFragmentShader->close(logicDevice);
		}
	}
}
