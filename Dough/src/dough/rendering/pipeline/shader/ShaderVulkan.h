#pragma once

#include "dough/rendering/Config.h"

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
		std::string& mFilePath;

	public:
		ShaderVulkan() = delete;
		ShaderVulkan(const ShaderVulkan& copy) = delete;
		ShaderVulkan operator=(const ShaderVulkan& assignment) = delete;

		ShaderVulkan(EShaderType type, std::string& filePath);

		void loadModule(VkDevice logicDeivce);
		void closeModule(VkDevice logicDevice);
		void close(VkDevice logicDevice);

		inline bool isLoaded() const { return mShaderModule != VK_NULL_HANDLE; }

		VkShaderModule getShaderModule() const { return mShaderModule; }
		EShaderType getShaderType() const { return mShaderType; }

	private:
		static VkShaderModule createShaderModule(VkDevice logicDevice, const std::vector<char>& shaderByteCode);
	};
}
