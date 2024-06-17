#pragma once

#include "dough/rendering/IGPUResourceVulkan.h"
#include "dough/rendering/pipeline/EShaderStage.h"

namespace DOH {

	class ShaderVulkan : public IGPUResourceVulkan {

	private:
		VkShaderModule mShaderModule;
		const char* mFilePath;
		const EShaderStage mShaderStage;

	public:
		ShaderVulkan() = delete;
		ShaderVulkan(const ShaderVulkan& copy) = delete;
		ShaderVulkan operator=(const ShaderVulkan& assignment) = delete;

		ShaderVulkan(const EShaderStage stage, const char* filePath)
		:	mShaderModule(VK_NULL_HANDLE),
			mShaderStage(stage),
			mFilePath(filePath)
		{}

		virtual ~ShaderVulkan() override;
		virtual void close(VkDevice logicDevice) override;

		void init(VkDevice logicDevice);

		inline VkShaderModule getShaderModule() const { return mShaderModule; }
		inline const EShaderStage getShaderStage() const { return mShaderStage; }
		inline bool isModuleLoaded() const { return mShaderModule != VK_NULL_HANDLE; }
	};
}
