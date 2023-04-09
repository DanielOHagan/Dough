#pragma once

#include "dough/rendering/pipeline/shader/ShaderVulkan.h"
#include "dough/rendering/pipeline/shader/ShaderUniformLayout.h"
#include "dough/rendering/pipeline/DescriptorSetLayoutVulkan.h"

namespace DOH {

	class ShaderProgramVulkan : public IGPUResourceVulkan {

	private:
		std::unique_ptr<ShaderUniformLayout> mShaderUniformLayout;
		std::unique_ptr<DescriptorSetLayoutVulkan> mShaderDescriptorLayout;

		std::shared_ptr<ShaderVulkan> mVertexShader;
		std::shared_ptr<ShaderVulkan> mFragmentShader;

	public:
		ShaderProgramVulkan() = delete;
		ShaderProgramVulkan(const ShaderProgramVulkan& copy) = delete;
		ShaderProgramVulkan operator=(const ShaderProgramVulkan& assignment) = delete;
		
		ShaderProgramVulkan(
			std::shared_ptr<ShaderVulkan> vertShader,
			std::shared_ptr<ShaderVulkan> fragShader
		);

		void loadModules(VkDevice logicDevice);
		void closeModules(VkDevice logicDevice);
		void closePipelineSpecificObjects(VkDevice logicDevice);
		virtual void close(VkDevice logicDevice) override;

		inline DescriptorSetLayoutVulkan& getShaderDescriptorLayout() const { return *mShaderDescriptorLayout; }
		inline ShaderUniformLayout& getUniformLayout() const { return *mShaderUniformLayout; }
		inline ShaderVulkan& getVertexShader() const { return *mVertexShader; }
		inline ShaderVulkan& getFragmentShader() const { return *mFragmentShader; }
		inline bool areShadersLoaded() const { return mVertexShader->isUsingGpuResource() && mFragmentShader->isUsingGpuResource(); }
	};
}
