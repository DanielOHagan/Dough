#pragma once

#include "dough/rendering/pipeline_2/ShaderVulkan_2.h"
#include "dough/rendering/pipeline_2/ShaderUniformLayoutVulkan_2.h"

namespace DOH {

	class ShaderProgram_2 {

	private:
		//std::unique_ptr<ShaderUniformLayout> mShaderUniformLayout;
		//std::unique_ptr<DescriptorSetLayoutVulkan> mShaderDescriptorLayout;

		std::shared_ptr<ShaderVulkan_2> mVertexShader;
		std::shared_ptr<ShaderVulkan_2> mFragmentShader;

		//NOTE:: ShaderProgram DOES NOT have ownership of uniform layout
		std::shared_ptr<ShaderUniformLayoutVulkan_2> mUniformLayout;

	public:
		ShaderProgram_2(const ShaderProgram_2& copy) = delete;
		ShaderProgram_2 operator=(const ShaderProgram_2& assignment) = delete;

		ShaderProgram_2(
			std::shared_ptr<ShaderVulkan_2> vertShader,
			std::shared_ptr<ShaderVulkan_2> fragShader,
			std::shared_ptr<ShaderUniformLayoutVulkan_2> uniformLayout
		) : mVertexShader(vertShader),
			mFragmentShader(fragShader),
			mUniformLayout(uniformLayout)
		{}

		void init(VkDevice logicDevice);
		void close(VkDevice logicDevice);

		inline bool areShadersLoaded() const { return mVertexShader->isModuleLoaded() && mFragmentShader->isModuleLoaded(); }

		inline ShaderVulkan_2& getVertexShader() const { return *mVertexShader; }
		inline ShaderVulkan_2& getFragmentShader() const { return *mFragmentShader; }
		inline bool hasUniformLayout() const { return mUniformLayout != nullptr; }
		inline ShaderUniformLayoutVulkan_2& getUniformLayout() const { return *mUniformLayout; }
	};
}
