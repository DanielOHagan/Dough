#pragma once

#include "dough/Utils.h"

#include <vulkan/vulkan_core.h>

namespace DOH {

	enum class EShaderType {
		NONE = 0,

		VERTEX,
		FRAGMENT
	};

	class ShaderVulkan {

	private:
		EShaderType mShaderType;
		VkShaderModule mShaderModule;

	public:
		static const std::string& NO_PATH;

	private:
		ShaderVulkan(EShaderType type, VkShaderModule shaderModule);

		static VkShaderModule createShaderModule(VkDevice logicDevice, const std::vector<char>& shaderByteCode);

	public:

		void close(VkDevice logicDevice);

		VkShaderModule getShaderModule() const { return mShaderModule; }
		EShaderType getShaderType() const { return mShaderType; }

	public:
		static ShaderVulkan create(VkDevice logicDevice, EShaderType type, const std::string& filePath);
	};
}