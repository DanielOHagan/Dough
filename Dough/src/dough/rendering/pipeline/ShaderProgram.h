#pragma once

#include "dough/rendering/pipeline/ShaderVulkan.h"
#include "dough/rendering/pipeline/ShaderDescriptorSetLayoutsVulkan.h"

namespace DOH {

	class ShaderProgram {
	private:
		std::shared_ptr<ShaderVulkan> mVertexShader;
		std::shared_ptr<ShaderVulkan> mFragmentShader;

		//TODO:: should ShaderDescriptorSetLayoutsVulkan object lifetime be left to shared_ptr lifetime? They are not IGPUResource
		std::shared_ptr<ShaderDescriptorSetLayoutsVulkan> mDescriptorSetLayout;

	public:
		ShaderProgram(const ShaderProgram& copy) = delete;
		ShaderProgram operator=(const ShaderProgram& assignment) = delete;

		ShaderProgram(
			std::shared_ptr<ShaderVulkan> vertShader,
			std::shared_ptr<ShaderVulkan> fragShader,
			std::shared_ptr<ShaderDescriptorSetLayoutsVulkan> descSetLayouts
		) : mVertexShader(vertShader),
			mFragmentShader(fragShader),
			mDescriptorSetLayout(descSetLayouts)
		{}

		void init(VkDevice logicDevice);
		void close(VkDevice logicDevice);

		inline bool areShadersLoaded() const { return mVertexShader->isModuleLoaded() && mFragmentShader->isModuleLoaded(); }

		inline ShaderVulkan& getVertexShader() const { return *mVertexShader; }
		inline ShaderVulkan& getFragmentShader() const { return *mFragmentShader; }
		//Also includes push constant information
		inline bool hasDescriptorSetLayouts() const { return mDescriptorSetLayout != nullptr; }
		//Also includes push constant information
		inline ShaderDescriptorSetLayoutsVulkan& getDescriptorSetLayouts() const { return *mDescriptorSetLayout; }
	};
}
