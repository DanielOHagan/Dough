#pragma once

#include "dough/rendering/pipeline/DescriptorApiVulkan.h"

namespace DOH {

	static std::array<const char*, 4> EShaderDescriptorTypeStrings = {
		"NONE",

		"VALUE",
		"TEXTURE",
		"TEXTURE_ARRAY"
	};

	enum class EShaderDescriptorType {
		NONE,

		VALUE,
		TEXTURE,
		TEXTURE_ARRAY
	};

	class AShaderDescriptor {
	protected:
		EShaderDescriptorType mType;
		uint32_t mBinding;
		//const char* mDebugName; //TODO:: Needed? Useful for debug? [slot][binding] should be enough
		DescriptorLayout mObject;

		AShaderDescriptor(EShaderDescriptorType type, uint32_t binding)
		:	mType(type),
			mBinding(binding)
		{}

	public:
		inline uint32_t getBinding() const { return mBinding; }
		inline EShaderDescriptorType getType() const { return mType; }
		//inline EShaderDescriptorType getType() const {
		//	switch (mObject.index()) {
		//		case static_cast<size_t>(EShaderDescriptorType::VALUE):
		//			return EShaderDescriptorType::VALUE;
		//
		//		case static_cast<size_t>(EShaderDescriptorType::TEXTURE):
		//			return EShaderDescriptorType::TEXTURE;
		//		case static_cast<size_t>(EShaderDescriptorType::TEXTURE_ARRAY):
		//			return EShaderDescriptorType::TEXTURE_ARRAY;
		//
		//		case static_cast<size_t>(EShaderDescriptorType::NONE):
		//		default:
		//			return EShaderDescriptorType::NONE;
		//	}
		//}

		static VkDescriptorSetLayoutBinding getLayoutBindingVulkan(const AShaderDescriptor& desc);
		static VkDescriptorType getTypeVulkanEnum(const AShaderDescriptor& desc);
	};

	class ValueDescriptor : public AShaderDescriptor {
	public:
		ValueDescriptor(uint32_t binding, VkDeviceSize valueSize)
		:	AShaderDescriptor(EShaderDescriptorType::VALUE, binding)
		{
			mObject.emplace<VkDeviceSize>(valueSize);
		}

		inline size_t getValueSize() const { return static_cast<size_t>(std::get<VkDeviceSize>(mObject)); }
	};

	class TextureDescriptor : public AShaderDescriptor {
	public:
		TextureDescriptor(uint32_t binding, TextureVulkan& texture)
		:	AShaderDescriptor(EShaderDescriptorType::TEXTURE, binding)
		{
			mObject.emplace<std::reference_wrapper<TextureVulkan>>(texture);
		}

		inline const TextureVulkan& getTexture() const { return std::get<std::reference_wrapper<TextureVulkan>>(mObject); }
	};

	class TextureArrayDescriptor : public AShaderDescriptor {
	public:
		TextureArrayDescriptor(uint32_t binding, TextureArray& textureArray)
		:	AShaderDescriptor(EShaderDescriptorType::TEXTURE_ARRAY, binding)
		{
			mObject.emplace<std::reference_wrapper<TextureArray>>(textureArray);
		}

		inline TextureArray& getTextureArray() const { return std::get<std::reference_wrapper<TextureArray>>(mObject); }
	};
}
