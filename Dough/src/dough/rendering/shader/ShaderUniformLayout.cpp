#include "dough/rendering/shader/ShaderUniformLayout.h"

namespace DOH {

	ShaderUniformLayout::ShaderUniformLayout()
	:	mValueUniformMap(std::make_unique<std::map<uint32_t, ValueUniformInfo>>()),
		mTextureUniformMap(std::make_unique<std::map<uint32_t, TextureUniformInfo>>())
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
		}
	}

	void ShaderUniformLayout::setTexture(uint32_t binding, TextureUniformInfo textureInfo) {
		if (isBindingAvailable(binding)) {
			mTextureUniformMap->emplace(binding, textureInfo);
		}
	}

	void ShaderUniformLayout::clearDescriptorSetLayoutBindings() {
		mDescriptorSetLayoutBindings.clear();
	}

	void ShaderUniformLayout::clearUniforms() const {
		mValueUniformMap->clear();
		mTextureUniformMap->clear();
	}

	void ShaderUniformLayout::close() {
		clearDescriptorSetLayoutBindings();
		clearUniforms();
	}

	std::vector<VkDescriptorType> ShaderUniformLayout::asDescriptorTypes() const {
		std::vector<VkDescriptorType> descTypes{};
		for (const VkDescriptorSetLayoutBinding& layoutBinding : mDescriptorSetLayoutBindings) {
			descTypes.push_back(layoutBinding.descriptorType);
		}

		return descTypes;
	}
}
