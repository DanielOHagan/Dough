#include "dough/rendering/pipeline/shader/ShaderUniformLayout.h"
#include "dough/Utils.h"

namespace DOH {

	ShaderUniformLayout::ShaderUniformLayout()
	:	mValueUniformMap(std::make_unique<std::map<uint32_t, ValueUniformInfo>>()),
		mTextureUniformMap(std::make_unique<std::map<uint32_t, TextureUniformInfo>>()),
		mHasUniforms(false)
	{}

	void ShaderUniformLayout::initDescriptorSetLayoutBindings(uint32_t count) {
		if (count < 0) {
			count = 0;
		}
		mDescriptorSetLayoutBindings = std::vector<VkDescriptorSetLayoutBinding>(count);
	}

	bool ShaderUniformLayout::isBindingAvailable(uint32_t binding) const {
		std::map<uint32_t, ValueUniformInfo>::iterator valueIt = mValueUniformMap->find(binding);
		if (valueIt != mValueUniformMap->end()) {
			return false;
		}

		std::map<uint32_t, TextureUniformInfo>::iterator textureIt = mTextureUniformMap->find(binding);
		if (textureIt != mTextureUniformMap->end()) {
			return false;
		}

		return true;
	}

	void ShaderUniformLayout::setValue(uint32_t binding, ValueUniformInfo value) {
		if (isBindingAvailable(binding)) {
			mValueUniformMap->emplace(binding, value);
			mHasUniforms = true;
		}
	}

	void ShaderUniformLayout::setTexture(uint32_t binding, TextureUniformInfo textureInfo) {
		if (isBindingAvailable(binding)) {
			mTextureUniformMap->emplace(binding, textureInfo);
			mHasUniforms = true;
		}
	}

	void ShaderUniformLayout::addPushConstant(VkShaderStageFlags shaderStages, uint32_t size) {
		//TODO:: Only the 128 bits guaranteed by Vulkan are supported right now, set a limit based off device capabilities
		uint32_t offset = getPushConstantOffset();
		TRY(offset + size > 128, "Currently only 128bit push constants are supported");
		
		VkPushConstantRange pushConstant = {};
		pushConstant.stageFlags = shaderStages;
		pushConstant.size = size;
		pushConstant.offset = offset;

		mPushConstantRanges.push_back(pushConstant);
	}

	void ShaderUniformLayout::clearDescriptorSetLayoutBindings() {
		mDescriptorSetLayoutBindings.clear();
	}

	void ShaderUniformLayout::clearUniforms() {
		mValueUniformMap->clear();
		mTextureUniformMap->clear();
		mPushConstantRanges.clear();
		mHasUniforms = false;
	}

	void ShaderUniformLayout::close() {
		clearDescriptorSetLayoutBindings();
		clearUniforms();
	}

	std::vector<VkDescriptorType> ShaderUniformLayout::asDescriptorTypes() const {
		std::vector<VkDescriptorType> descTypes = {};
		for (const VkDescriptorSetLayoutBinding& layoutBinding : mDescriptorSetLayoutBindings) {
			descTypes.push_back(layoutBinding.descriptorType);
		}

		return descTypes;
	}

	uint32_t ShaderUniformLayout::getPushConstantOffset() const {
		uint32_t size = 0;
		for (VkPushConstantRange pushConstant : mPushConstantRanges) {
			size += pushConstant.size;
		}

		return size;
	}
}
