#include "dough/rendering/pipeline/shader/ShaderUniformLayout.h"
#include "dough/Utils.h"
#include "dough/Logging.h"

namespace DOH {

	ShaderUniformLayout::ShaderUniformLayout()
	:	mValueUniformMap(std::make_unique<std::unordered_map<uint32_t, ValueUniformInfo>>()),
		mTextureUniformMap(std::make_unique<std::unordered_map<uint32_t, TextureUniformInfo>>()),
		mTextureArrayUniformMap(std::make_unique<std::unordered_map<uint32_t, std::reference_wrapper<TextureArray>>>()),
		mHasUniforms(false)
	{}

	void ShaderUniformLayout::initDescriptorSetLayoutBindings(uint32_t count) {
		if (count < 0) {
			count = 0;
		}
		mDescriptorSetLayoutBindings = std::vector<VkDescriptorSetLayoutBinding>(count);
	}

	bool ShaderUniformLayout::isBindingAvailable(uint32_t binding) const {
		std::unordered_map<uint32_t, ValueUniformInfo>::iterator valueIt = mValueUniformMap->find(binding);
		if (valueIt != mValueUniformMap->end()) {
			return false;
		}

		std::unordered_map<uint32_t, TextureUniformInfo>::iterator textureIt = mTextureUniformMap->find(binding);
		if (textureIt != mTextureUniformMap->end()) {
			return false;
		}

		std::unordered_map<uint32_t, std::reference_wrapper<TextureArray>>::iterator textureArrIt = mTextureArrayUniformMap->find(binding);
		if (textureArrIt != mTextureArrayUniformMap->end()) {
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

	void ShaderUniformLayout::setTextureArray(uint32_t binding, TextureArray& texArr) {
		if (isBindingAvailable(binding)) {
			mTextureArrayUniformMap->emplace(binding, texArr);
			mHasUniforms = true;
		} else {
			LOG_WARN("Binding: " << binding << " not available for texture array");
		}
		
	}

	void ShaderUniformLayout::addPushConstant(VkShaderStageFlags shaderStages, uint32_t size) {
		//TODO:: Only the 128 bits guaranteed by Vulkan are supported right now, set a limit based off device capabilities
		uint32_t offset = getPushConstantOffset();
		if (offset + size > 128) {
			LOG_ERR("Currently only 128bit push constants are supported");
			return;
		}

		VkPushConstantRange pushConstant = {};
		pushConstant.stageFlags = shaderStages;
		pushConstant.size = size;
		pushConstant.offset = offset;

		mPushConstantRanges.push_back(pushConstant);
	}

	void ShaderUniformLayout::clearDescriptorSetLayoutBindings() {
		mDescriptorSetLayoutBindings.clear();
	}

	void ShaderUniformLayout::clearTextureUniforms() {
		mTextureUniformMap->clear();
		mTextureArrayUniformMap->clear();
		mHasUniforms = getTotalValueCount() > 0 ? true : false;
	}

	void ShaderUniformLayout::clearUniforms() {
		mValueUniformMap->clear();
		clearTextureUniforms();
		mPushConstantRanges.clear();
		mHasUniforms = false;
	}

	void ShaderUniformLayout::close() {
		clearDescriptorSetLayoutBindings();
		clearUniforms();
	}

	std::vector<DescriptorTypeInfo> ShaderUniformLayout::asDescriptorTypes() const {
		std::vector<DescriptorTypeInfo> descTypes = {};
		for (const VkDescriptorSetLayoutBinding& layoutBinding : mDescriptorSetLayoutBindings) {
			descTypes.push_back({ layoutBinding.descriptorType, layoutBinding.descriptorCount});
		}

		return descTypes;
	}

	uint32_t ShaderUniformLayout::getPushConstantOffset() const {
		uint32_t size = 0;
		for (const VkPushConstantRange& pushConstant : mPushConstantRanges) {
			size += pushConstant.size;
		}

		return size;
	}

	const uint32_t ShaderUniformLayout::getTotalTextureCountInTextureArrayMap() const {
		uint32_t count = 0;

		std::unordered_map<uint32_t, std::reference_wrapper<TextureArray>>::iterator textureArrIt;
		for (textureArrIt = mTextureArrayUniformMap->begin(); textureArrIt != mTextureArrayUniformMap->end(); textureArrIt++) {
			count += textureArrIt->second.get().getMaxTextureCount();
		}

		return count;
	}
}
