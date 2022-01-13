#pragma once

#include "dough/rendering/shader/ShaderVulkan.h"
#include "dough/rendering/shader/ShaderUniformLayout.h"
#include "dough/rendering/pipeline/DescriptorVulkan.h"

namespace DOH {

	class ShaderProgramVulkan : public IGPUResourceVulkan {

	private:
		std::unique_ptr<ShaderUniformLayout> mShaderUniformLayout;
		std::unique_ptr<DescriptorVulkan> mShaderDescriptor;

		std::shared_ptr<ShaderVulkan> mVertexShader;
		std::shared_ptr<ShaderVulkan> mFragmentShader;

	public:
		ShaderProgramVulkan() = delete;
		ShaderProgramVulkan(const ShaderProgramVulkan& copy) = delete;
		ShaderProgramVulkan operator=(const ShaderProgramVulkan& assignment) = delete;
		
		ShaderProgramVulkan(std::shared_ptr<ShaderVulkan> vertShader, std::shared_ptr<ShaderVulkan> fragShader);

		void loadModules(VkDevice logicDevice);
		void closeModules(VkDevice logicDevice);
		void closePipelineSpecificObjects(VkDevice logicDevice);
		virtual void close(VkDevice logicDevice) override;

		inline DescriptorVulkan& getShaderDescriptor() const { return *mShaderDescriptor; }
		inline ShaderUniformLayout& getUniformLayout() const { return *mShaderUniformLayout; }
		inline ShaderVulkan& getVertexShader() const { return *mVertexShader; }
		inline ShaderVulkan& getFragmentShader() const { return *mFragmentShader; }
		inline bool areShadersLoaded() const { return mVertexShader->isLoaded() && mFragmentShader->isLoaded(); }
	};
}
