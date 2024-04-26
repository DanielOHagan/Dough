#include "dough/rendering/pipeline_2/ShaderProgram_2.h"

namespace DOH {

	void ShaderProgram_2::init(VkDevice logicDevice) {
		if (!mVertexShader->isModuleLoaded()) {
			mVertexShader->init(logicDevice);
		}
		if (!mFragmentShader->isModuleLoaded()) {
			mFragmentShader->init(logicDevice);
		}
	}

	void ShaderProgram_2::close(VkDevice logicDevice) {
		if (mVertexShader->isModuleLoaded()) {
			mVertexShader->close(logicDevice);
		}
		if (mFragmentShader->isModuleLoaded()) {
			mFragmentShader->close(logicDevice);
		}
	}
}
