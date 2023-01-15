#include "dough/rendering/pipeline/shader/ShaderVulkan.h"

#include "dough/files/ResourceHandler.h"
#include "dough/Utils.h"

namespace DOH {

	ShaderVulkan::ShaderVulkan(EShaderType type, const std::string& filePath)
	:	mShaderType(type),
		mFilePath(filePath),
		mShaderModule(VK_NULL_HANDLE)
	{}

	void ShaderVulkan::loadModule(VkDevice logicDevice) {
		mShaderModule = ShaderVulkan::createShaderModule(logicDevice, ResourceHandler::readFile(mFilePath));
		mUsingGpuResource = true;
	}

	void ShaderVulkan::closeModule(VkDevice logicDevice) {
		if (isUsingGpuResource()) {
			vkDestroyShaderModule(logicDevice, mShaderModule, nullptr);
			mUsingGpuResource = false;
		}
	}
	
	void ShaderVulkan::close(VkDevice logicDevice) {
		closeModule(logicDevice);
	}

	VkShaderModule ShaderVulkan::createShaderModule(VkDevice logicDevice, const std::vector<char>& shaderByteCode) {
		VkShaderModuleCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = shaderByteCode.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(shaderByteCode.data());

		VkShaderModule shaderModule;
		VK_TRY(
			vkCreateShaderModule(logicDevice, &createInfo, nullptr, &shaderModule),
			"Failed to create shader module."
		);
		return shaderModule;
	}
}
