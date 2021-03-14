#include "dough/rendering/shader/ShaderVulkan.h"

namespace DOH {

	ShaderVulkan::ShaderVulkan(EShaderType type, VkShaderModule shaderModule)
	:	mShaderType(type),
		mShaderModule(shaderModule)
	{
	}

	void ShaderVulkan::close(VkDevice logicDevice) {
		vkDestroyShaderModule(logicDevice, mShaderModule, nullptr);
	}

	VkShaderModule ShaderVulkan::createShaderModule(VkDevice logicDevice, const std::vector<char>& shaderByteCode) {
		VkShaderModuleCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = shaderByteCode.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(shaderByteCode.data());

		VkShaderModule shaderModule;
		TRY(
			vkCreateShaderModule(logicDevice, &createInfo, nullptr, &shaderModule) != VK_SUCCESS,
			"Failed to create shader module."
		);

		return shaderModule;
	}

	ShaderVulkan ShaderVulkan::create(VkDevice logicDevice, EShaderType type, const std::string& filePath) {
		std::vector<char> shaderByteCode = readFile(filePath);
		return ShaderVulkan(type, ShaderVulkan::createShaderModule(logicDevice, shaderByteCode));
	}
}