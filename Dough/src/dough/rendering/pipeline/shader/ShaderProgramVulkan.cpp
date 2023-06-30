#include "dough/rendering/pipeline/shader/ShaderProgramVulkan.h"

namespace DOH {

	ShaderProgramVulkan::ShaderProgramVulkan(
		std::shared_ptr<ShaderVulkan> vertShader,
		std::shared_ptr<ShaderVulkan> fragShader
	) : mShaderUniformLayout(std::make_unique<ShaderUniformLayout>()),
		mShaderDescriptorLayout(std::make_unique<DescriptorSetLayoutVulkan>(*mShaderUniformLayout)),
		mVertexShader(vertShader),
		mFragmentShader(fragShader)
	{}

	void ShaderProgramVulkan::loadModules(VkDevice logicDevice) {
		mVertexShader->loadModule(logicDevice);
		mFragmentShader->loadModule(logicDevice);

		mUsingGpuResource = true;
	}
	
	void ShaderProgramVulkan::closeModules(VkDevice logicDevice) {
		mVertexShader->closeModule(logicDevice);
		mFragmentShader->closeModule(logicDevice);
	}

	void ShaderProgramVulkan::closePipelineSpecificObjects(VkDevice logicDevice) {
		closeModules(logicDevice);
		mShaderDescriptorLayout->closeBuffers(logicDevice);
	}

	void ShaderProgramVulkan::close(VkDevice logicDevice) {
		mShaderDescriptorLayout->close(logicDevice);
		mShaderUniformLayout->close();

		mVertexShader->close(logicDevice);
		mFragmentShader->close(logicDevice);

		mUsingGpuResource = false;
	}
}
