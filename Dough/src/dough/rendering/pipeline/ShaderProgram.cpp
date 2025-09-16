#include "dough/rendering/pipeline/ShaderProgram.h"

#include "dough/rendering/RenderingContextVulkan.h"

#include <tracy/public/tracy/Tracy.hpp>

namespace DOH {

	void ShaderProgram::addOwnedResourcesToClose(RenderingContextVulkan& context) {
		if (mVertexShader->isModuleLoaded()) {
			context.addGpuResourceToClose(mVertexShader);
		}

		if (mFragmentShader->isModuleLoaded()) {
			context.addGpuResourceToClose(mFragmentShader);
		}
	}

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
