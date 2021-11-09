#pragma once

#include "dough/rendering/Config.h"

namespace DOH {

	class ShaderUniformLayout {

	private:
		std::vector<VkDescriptorSetLayoutBinding> mDescriptorSetLayoutBindings;

		std::unique_ptr<std::map<uint32_t, ValueUniformInfo>> mValueUniformMap;
		std::unique_ptr<std::map<uint32_t, TextureUniformInfo>> mTextureUniformMap;

	public:
		ShaderUniformLayout();
		ShaderUniformLayout(const ShaderUniformLayout& copy) = delete;
		ShaderUniformLayout operator=(const ShaderUniformLayout& assignment) = delete;

		void initDescriptorSetLayoutBindings(uint32_t count);
		bool isBindingAvailable(uint32_t binding) const;
		void setValue(uint32_t binding, ValueUniformInfo value);
		void setTexture(uint32_t binding, TextureUniformInfo textureInfo);
		void clearDescriptorSetLayoutBindings();
		void clearUniforms() const;
		void close();

		inline const uint32_t getTotalUniformCount() const { return static_cast<uint32_t>(mValueUniformMap->size() + mTextureUniformMap->size()); }
		inline std::vector<VkDescriptorSetLayoutBinding>& getDescriptorSetLayoutBindings() { return mDescriptorSetLayoutBindings; }
		inline std::map<uint32_t, ValueUniformInfo>& getValueUniformMap() const { return *mValueUniformMap; }
		inline std::map<uint32_t, TextureUniformInfo>& getTextureUniformMap() const { return *mTextureUniformMap; }
	};
}
