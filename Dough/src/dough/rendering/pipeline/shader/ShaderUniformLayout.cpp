#include "dough/rendering/pipeline/shader/ShaderUniformLayout.h"
#include "dough/Utils.h"
#include "dough/Logging.h"

namespace DOH {

	ShaderUniformLayout::ShaderUniformLayout()
	:	mHasUniforms(false)
	{}

	bool ShaderUniformLayout::isBindingAvailable(uint32_t binding) const {
		if (mValueUniformMap.find(binding) != mValueUniformMap.end()) {
			return false;
		}

		if (mTextureUniformMap.find(binding) != mTextureUniformMap.end()) {
			return false;
		}

		if (mTextureArrayUniformMap.find(binding) != mTextureArrayUniformMap.end()) {
			return false;
		}

		return true;
	}

	void ShaderUniformLayout::setValue(uint32_t binding, ValueUniformInfo value) {
		if (isBindingAvailable(binding)) {
			mValueUniformMap.emplace(binding, value);
			mHasUniforms = true;
		}
	}

	void ShaderUniformLayout::setTexture(uint32_t binding, TextureUniformInfo textureInfo) {
		if (isBindingAvailable(binding)) {
			mTextureUniformMap.emplace(binding, textureInfo);
			mHasUniforms = true;
		}
	}

	void ShaderUniformLayout::setTextureArray(uint32_t binding, TextureArray& texArr) {
		if (isBindingAvailable(binding)) {
			mTextureArrayUniformMap.emplace(binding, texArr);
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

		mPushConstantRanges.emplace_back(pushConstant);
	}

	void ShaderUniformLayout::clearTextureUniforms() {
		mTextureUniformMap.clear();
		mTextureArrayUniformMap.clear();
		mHasUniforms = getTotalValueCount() > 0 ? true : false;
	}

	void ShaderUniformLayout::clearUniforms() {
		mValueUniformMap.clear();
		clearTextureUniforms();
		mPushConstantRanges.clear();
		mHasUniforms = false;
	}

	void ShaderUniformLayout::close() {
		clearUniforms();
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

		for (auto textureArrIt = mTextureArrayUniformMap.begin(); textureArrIt != mTextureArrayUniformMap.end(); textureArrIt++) {
			count += textureArrIt->second.get().getMaxTextureCount();
		}

		return count;
	}
}
