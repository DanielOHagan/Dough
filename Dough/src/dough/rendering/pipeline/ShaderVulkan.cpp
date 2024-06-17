#include "dough/rendering/pipeline/ShaderVulkan.h"

#include "dough/files/ResourceHandler.h"
#include "dough/Utils.h"

namespace DOH {

	ShaderVulkan::~ShaderVulkan() {
		if (isUsingGpuResource()) {
			LOG_ERR(
				"ShaderVulkan_2 GPU resource NOT released before destructor was called." <<
				" Handle: " << mShaderModule << " FilePath: " << mFilePath <<
				" Stage: " << EShaderStageStrings[static_cast<uint32_t>(mShaderStage)]
			);

			//NOTE:: This is to stop the IGPUResource::~IGPUReource from logging a misleading error message.
			mUsingGpuResource = false;
		}
	}

	void ShaderVulkan::close(VkDevice logicDevice) {
		vkDestroyShaderModule(logicDevice, mShaderModule, nullptr);
		mShaderModule = VK_NULL_HANDLE;
		mUsingGpuResource = false;
	}

	void ShaderVulkan::init(VkDevice logicDevice) {
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
