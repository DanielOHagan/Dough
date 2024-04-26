#include "dough/rendering/pipeline_2/ShaderVulkan_2.h"

#include "dough/files/ResourceHandler.h"
#include "dough/Utils.h"

namespace DOH {

	ShaderVulkan_2::~ShaderVulkan_2() {
		if (isUsingGpuResource()) {
			LOG_ERR(
				"ShaderVulkan_2 GPU resource NOT released before destructor was called." <<
				" Handle: " << mShaderModule << " FilePath: " << mFilePath <<
				" Stage: " << EShaderStage_2Strings[static_cast<uint32_t>(mShaderStage)]
			);
		}
	}

	void ShaderVulkan_2::close(VkDevice logicDevice) {
		vkDestroyShaderModule(logicDevice, mShaderModule, nullptr);
		mShaderModule = VK_NULL_HANDLE;
		mUsingGpuResource = false;
	}

	void ShaderVulkan_2::init(VkDevice logicDevice) {
		std::vector<char> shaderByteCode = ResourceHandler::readFile(mFilePath);

		VkShaderModuleCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = shaderByteCode.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(shaderByteCode.data());

		VK_TRY(
			vkCreateShaderModule(logicDevice, &createInfo, nullptr, &mShaderModule),
			"Failed to create shader module"
		);
		mUsingGpuResource = true;
	}
}
