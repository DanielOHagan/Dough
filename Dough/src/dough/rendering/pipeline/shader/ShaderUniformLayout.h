#pragma once

#include "dough/rendering/Config.h"
#include "dough/rendering/textures/TextureArray.h"

namespace DOH {

	class ShaderUniformLayout {

	private:
		std::unordered_map<uint32_t, ValueUniformInfo> mValueUniformMap;
		std::unordered_map<uint32_t, TextureUniformInfo> mTextureUniformMap;
		std::unordered_map<uint32_t, std::reference_wrapper<TextureArray>> mTextureArrayUniformMap;
		std::vector<VkPushConstantRange> mPushConstantRanges;

		bool mHasUniforms;

	public:
		ShaderUniformLayout();
		ShaderUniformLayout(const ShaderUniformLayout& copy) = delete;
		ShaderUniformLayout operator=(const ShaderUniformLayout& assignment) = delete;

		bool isBindingAvailable(const uint32_t binding) const;
		void setValue(const uint32_t binding, ValueUniformInfo value);
		void setTexture(const uint32_t binding, TextureUniformInfo textureInfo);
		inline void setTexture(const uint32_t binding, TextureVulkan& texture) { setTexture(binding, { texture.getImageView(), texture.getSampler() }); }
		void setTextureArray(const uint32_t binding, TextureArray& texArr);
		void addPushConstant(const VkShaderStageFlags shaderStages, const uint32_t size);
		void clearTextureUniforms();
		void clearUniforms();
		void close();

		const uint32_t getTotalTextureCountInTextureArrayMap() const;
		inline const uint32_t getTotalValueCount() const { return static_cast<uint32_t>(mValueUniformMap.size()); }
		inline const uint32_t getTotalTextureCount() const { return static_cast<uint32_t>(mTextureUniformMap.size()) + getTotalTextureCountInTextureArrayMap(); };
		inline const uint32_t getTotalTextureUniformCount() const { return static_cast<uint32_t>(mTextureUniformMap.size() + mTextureArrayUniformMap.size()); }
		inline const uint32_t getTotalUniformCount() const { return getTotalValueCount() + getTotalTextureUniformCount(); }
		inline bool hasPushConstant() const { return mPushConstantRanges.size() > 0; }
		inline bool hasUniforms() const { return mHasUniforms; }
		inline std::vector<VkPushConstantRange>& getPushConstantRanges() { return mPushConstantRanges; }
		inline const std::unordered_map<uint32_t, ValueUniformInfo>& getValueUniformMap() const { return mValueUniformMap; }
		inline const std::unordered_map<uint32_t, TextureUniformInfo>& getTextureUniformMap() const { return mTextureUniformMap; }
		inline const std::unordered_map<uint32_t, std::reference_wrapper<TextureArray>>& getTextureArrayUniformMap() const { return mTextureArrayUniformMap; }

	private:
		uint32_t getPushConstantOffset() const;
	};
}
