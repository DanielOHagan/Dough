#include "dough/rendering/pipeline/shader/ShaderVulkan.h"

#include "dough/ResourceHandler.h"
#include "dough/Utils.h"

namespace DOH {

	ShaderVulkan::ShaderVulkan(EShaderType type, std::string& filePath)
	:	mShaderType(type),
		mFilePath(filePath),
		mShaderModule(VK_NULL_HANDLE)
	{}

	void ShaderVulkan::loadModule(VkDevice logicDevice) {
		mShaderModule = ShaderVulkan::createShaderModule(logicDevice, ResourceHandler::readFile(mFilePath));
	}

	void ShaderVulkan::closeModule(VkDevice logicDevice) {
		vkDestroyShaderModule(logicDevice, mShaderModule, nullptr);

		//TODO:: Test to see if vkDestroyShaderModule already sets this to VK_NULL_HANDLE
		mShaderModule = VK_NULL_HANDLE;
	}
	
	void ShaderVulkan::close(VkDevice logicDevice) {
		if (isLoaded()) {
			closeModule(logicDevice);
		}

		//TODO:: release anything else
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
