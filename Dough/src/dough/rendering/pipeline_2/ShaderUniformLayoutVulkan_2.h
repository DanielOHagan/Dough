#pragma once

#include "dough/rendering/Config.h"
#include "dough/rendering/IGPUResourceVulkan.h"
#include "dough/rendering/pipeline_2/DescriptorApiVulkan.h"

namespace DOH {

	static std::array<const char*, 4> EShaderUniformTypeStrings = {
		"NONE",

		"VALUE",
		"TEXTURE",
		"TEXTURE_ARRAY"
	};

	enum class EShaderUniformType {
		NONE,

		VALUE,
		TEXTURE,
		TEXTURE_ARRAY
	};

	class ShaderUniform {
	protected:
		EShaderUniformType mType;
		uint32_t mBinding;
		//const char* mDebugName; //TODO:: Needed? Useful for debug? [slot][binding] should be enough
		DescriptorLayout mObject;

		ShaderUniform(EShaderUniformType type, uint32_t binding)
		:	mType(type),
			mBinding(binding)
		{}

	public:
		inline uint32_t getBinding() const { return mBinding; }
		inline EShaderUniformType getUniformType() const { return mType; }
		//inline EShaderUniformType getUniformType() const {
		//	switch (mObject.index()) {
		//		case static_cast<size_t>(EShaderUniformType::VALUE):
		//			return EShaderUniformType::VALUE;
		//
		//		case static_cast<size_t>(EShaderUniformType::TEXTURE):
		//			return EShaderUniformType::TEXTURE;
		//		case static_cast<size_t>(EShaderUniformType::TEXTURE_ARRAY):
		//			return EShaderUniformType::TEXTURE_ARRAY;
		//
		//		case static_cast<size_t>(EShaderUniformType::NONE):
		//		default:
		//			return EShaderUniformType::NONE;
		//	}
		//}

		static VkDescriptorSetLayoutBinding getLayoutBindingVulkan(const ShaderUniform& uniform);
		static VkDescriptorType getUniformTypeNativeEnum(const ShaderUniform& uniform);
	};

	class ValueShaderUniform : public ShaderUniform {
	public:
		ValueShaderUniform(uint32_t binding, VkDeviceSize valueSize)
		:	ShaderUniform(EShaderUniformType::VALUE, binding)
		{
			mObject.emplace<VkDeviceSize>(valueSize);
		}

		inline size_t getValueSize() const { return static_cast<size_t>(std::get<VkDeviceSize>(mObject)); }
	};

	class TextureShaderUniform : public ShaderUniform {
	public:
		TextureShaderUniform(uint32_t binding, TextureVulkan& texture)
		:	ShaderUniform(EShaderUniformType::TEXTURE, binding)
		{
			mObject.emplace<std::reference_wrapper<TextureVulkan>>(texture);
		}

		inline const TextureVulkan& getTexture() const { return std::get<std::reference_wrapper<TextureVulkan>>(mObject); }
	};

	class TextureArrayShaderUniform : public ShaderUniform {
	public:
		TextureArrayShaderUniform(uint32_t binding, TextureArray& textureArray)
		:	ShaderUniform(EShaderUniformType::TEXTURE, binding)
		{
			mObject.emplace<std::reference_wrapper<TextureArray>>(textureArray);
		}

		inline TextureArray& getTextureArray() const { return std::get<std::reference_wrapper<TextureArray>>(mObject); }
	};

	class ShaderUniformSetLayoutVulkan : public IGPUResourceVulkan {
	private:
		std::vector<ShaderUniform> mUniforms;
		std::vector<VkDescriptorSetLayoutBinding> mBindings;
		VkDescriptorSetLayout mLayout;

	public:
		ShaderUniformSetLayoutVulkan(const std::vector<ShaderUniform>& uniforms)
		:	mUniforms(uniforms),
			mLayout(VK_NULL_HANDLE)
		{}

		virtual ~ShaderUniformSetLayoutVulkan() override;

		virtual void close(VkDevice logicDevice) override;

		void init(VkDevice logicDevice);

		inline std::vector<ShaderUniform>& getUniforms() { return mUniforms; }
		inline const VkDescriptorSetLayout& getLayout() const { return mLayout; }

	private:
		void createDescriptorSetLayoutBindings();
		void createDescriptorSetLayout(VkDevice logicDevice);
	};

	//TODO:: Rename so it is much less confusing
	class ShaderUniformLayoutVulkan_2 {
	private:
		std::vector<std::reference_wrapper<ShaderUniformSetLayoutVulkan>> mUniformSetLayouts;
		std::vector<VkPushConstantRange> mPushConstants;

	public:
		ShaderUniformLayoutVulkan_2() = default;
		ShaderUniformLayoutVulkan_2(const std::vector<VkPushConstantRange>& pushConstants)
		:	mPushConstants(pushConstants)
		{}
		ShaderUniformLayoutVulkan_2(const std::vector<std::reference_wrapper<ShaderUniformSetLayoutVulkan>>& sets)
		:	mUniformSetLayouts(sets)
		{}
		ShaderUniformLayoutVulkan_2(const std::vector<VkPushConstantRange>& pushConstants, const std::vector<std::reference_wrapper<ShaderUniformSetLayoutVulkan>>& sets)
		:	mPushConstants(pushConstants),
			mUniformSetLayouts(sets)
		{}

		std::vector<VkDescriptorSetLayout> getNativeSetLayouts() const;

		/**
		* Get the number of sets used in this layout.
		* 
		* @returns The number of sets used for this layout.
		*/
		inline uint32_t getSetCount() const { return static_cast<uint32_t>(mUniformSetLayouts.size()); }
		
		/**
		* Cycle through the shader's uniform sets and count the unique individual uniforms.
		* Indexes of each type match EShaderUniformType order.
		* 
		* @returns Array of the count of each different EShaderUniformType.
		*/
		std::array<uint32_t, 4> getUniformCounts() const;
		inline uint32_t getPushConstantCount() const { return static_cast<uint32_t>(mPushConstants.size()); }
		inline const std::vector<std::reference_wrapper<ShaderUniformSetLayoutVulkan>>& getSets() const { return mUniformSetLayouts; }
		inline const std::vector<VkPushConstantRange>& getPushConstants() const { return mPushConstants; }

		uint32_t getPushConstantOffset() const;
	};

	class ShaderUniformSetsInstanceVulkan {
	private:
		std::vector<VkDescriptorSet> mDescriptorSets;
	
	public:
		ShaderUniformSetsInstanceVulkan(std::initializer_list<VkDescriptorSet> descSets)
		:	mDescriptorSets(descSets)
		{}
	
		inline std::vector<VkDescriptorSet>& getDescriptorSets() { return mDescriptorSets; }
	};
}
