#pragma once

#include "dough/rendering/Config.h"
#include "dough/rendering/IGPUResourceVulkan.h"

namespace DOH {

	enum class EShaderType {
		NONE = 0,

		VERTEX,
		FRAGMENT
	};

	class ShaderVulkan : public IGPUResourceVulkan {

	private:
		const EShaderType mShaderType;
		VkShaderModule mShaderModule;
		const std::string& mFilePath;

	public:
		ShaderVulkan() = delete;
		ShaderVulkan(const ShaderVulkan& copy) = delete;
		ShaderVulkan operator=(const ShaderVulkan& assignment) = delete;

		ShaderVulkan(const EShaderType type, const std::string& filePath);

		void loadModule(VkDevice logicDeivce);
		void closeModule(VkDevice logicDevice);
		void close(VkDevice logicDevice);

		VkShaderModule getShaderModule() const { return mShaderModule; }
		EShaderType getShaderType() const { return mShaderType; }

	private:
		static VkShaderModule createShaderModule(VkDevice logicDevice, const std::vector<char>& shaderByteCode);
	};
}
