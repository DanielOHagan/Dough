#include "dough/rendering/shader/ShaderProgramVulkan.h"

namespace DOH {

	ShaderProgramVulkan::ShaderProgramVulkan(
		std::shared_ptr<ShaderVulkan> vertShader,
		std::shared_ptr<ShaderVulkan> fragShader
	) : mShaderUniformLayout(std::make_unique<ShaderUniformLayout>()),
		mShaderDescriptor(std::make_unique<DescriptorVulkan>(*mShaderUniformLayout)),
		mVertexShader(vertShader),
		mFragmentShader(fragShader)
	{}

	void ShaderProgramVulkan::loadModules(VkDevice logicDevice) {
		mVertexShader->loadModule(logicDevice);
		mFragmentShader->loadModule(logicDevice);
	}
	
	void ShaderProgramVulkan::closeModules(VkDevice logicDevice) {
		mVertexShader->closeModule(logicDevice);
		mFragmentShader->closeModule(logicDevice);
	}

	void ShaderProgramVulkan::closePipelineSpecificObjects(VkDevice logicDevice) {
		closeModules(logicDevice);
		mShaderDescriptor->close(logicDevice);
	}

	void ShaderProgramVulkan::close(VkDevice logicDevice) {
		mShaderDescriptor->close(logicDevice);
		mShaderUniformLayout->close();

		mVertexShader->close(logicDevice);
		mFragmentShader->close(logicDevice);
	}
}
