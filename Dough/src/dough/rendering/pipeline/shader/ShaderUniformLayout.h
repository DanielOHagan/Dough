#pragma once

#include "dough/rendering/Config.h"

namespace DOH {

	class ShaderUniformLayout {

		//TODO:: Add limit to texture array size based off device limitations and batch sizing

	private:
		std::vector<VkDescriptorSetLayoutBinding> mDescriptorSetLayoutBindings;

		std::unique_ptr<std::map<uint32_t, ValueUniformInfo>> mValueUniformMap;
		std::unique_ptr<std::map<uint32_t, TextureUniformInfo>> mTextureUniformMap;
		std::unique_ptr<std::map<uint32_t, TextureArrayUniformInfo>> mTextureArrayUniformMap;
		std::vector<VkPushConstantRange> mPushConstantRanges;

		bool mHasUniforms;

	public:
		ShaderUniformLayout();
		ShaderUniformLayout(const ShaderUniformLayout& copy) = delete;
		ShaderUniformLayout operator=(const ShaderUniformLayout& assignment) = delete;

		void initDescriptorSetLayoutBindings(uint32_t count);
		bool isBindingAvailable(uint32_t binding) const;
		void setValue(uint32_t binding, ValueUniformInfo value);
		void setTexture(uint32_t binding, TextureUniformInfo textureInfo);
		void setTextureArray(
			uint32_t binding,
			TextureArrayUniformInfo texArrayInfo,
			uint32_t countToFill,
			TextureUniformInfo fillTexture
		);
		void addPushConstant(VkShaderStageFlags shaderStages, uint32_t size);
		void clearDescriptorSetLayoutBindings();
		void clearUniforms();
		void close();
		std::vector<DescriptorTypeInfo> asDescriptorTypes() const;

		const uint32_t getTotalTextureCountInTextureArrayMap() const;
		inline const uint32_t getTotalTextureCount() const { return static_cast<uint32_t>(mTextureUniformMap->size()) + getTotalTextureCountInTextureArrayMap(); };
		inline const uint32_t getTotalTextureUniformCount() const { return static_cast<uint32_t>(mTextureUniformMap->size() + mTextureArrayUniformMap->size()); }
		inline const uint32_t getTotalUniformCount() const { return static_cast<uint32_t>(mValueUniformMap->size() + getTotalTextureUniformCount()); }
		inline bool hasPushConstant() const { return mPushConstantRanges.size() > 0; }
		inline bool hasUniforms() const { return mHasUniforms; }
		inline std::vector<VkPushConstantRange>& getPushConstantRanges() { return mPushConstantRanges; }
		inline std::vector<VkDescriptorSetLayoutBinding>& getDescriptorSetLayoutBindings() { return mDescriptorSetLayoutBindings; }
		inline std::map<uint32_t, ValueUniformInfo>& getValueUniformMap() const { return *mValueUniformMap; }
		inline std::map<uint32_t, TextureUniformInfo>& getTextureUniformMap() const { return *mTextureUniformMap; }
		inline std::map<uint32_t, TextureArrayUniformInfo>& getTextureArrayUniformMap() const { return *mTextureArrayUniformMap; }

	private:
		uint32_t getPushConstantOffset() const;
	};
}