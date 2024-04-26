#pragma once

#include "dough/rendering/IGPUResourceVulkan.h"
#include "dough/rendering/pipeline_2/EShaderStage.h"

namespace DOH {

	class ShaderVulkan_2 : public IGPUResourceVulkan {

	private:
		VkShaderModule mShaderModule;
		const char* mFilePath;
		const EShaderStage_2 mShaderStage;

	public:
		ShaderVulkan_2() = delete;
		ShaderVulkan_2(const ShaderVulkan_2& copy) = delete;
		ShaderVulkan_2 operator=(const ShaderVulkan_2& assignment) = delete;

		ShaderVulkan_2(const EShaderStage_2 stage, const char* filePath)
		:	mShaderModule(VK_NULL_HANDLE),
			mShaderStage(stage),
			mFilePath(filePath)
		{}

		virtual ~ShaderVulkan_2() override;
		virtual void close(VkDevice logicDevice) override;

		void init(VkDevice logicDevice);

		inline VkShaderModule getShaderModule() const { return mShaderModule; }
		inline const EShaderStage_2 getShaderStage() const { return mShaderStage; }
		inline bool isModuleLoaded() const { return mShaderModule != VK_NULL_HANDLE; }
	};
}
