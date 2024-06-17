#pragma once

#include "dough/rendering/Config.h"
#include "dough/rendering/IGPUResourceVulkan.h"
#include "dough/rendering/pipeline/DescriptorApiVulkan.h"
#include "dough/rendering/pipeline/AShaderDescriptor.h"

namespace DOH {
	
	//NOTE:: The term "Uniform" is used interchangably with "Descriptor".
	//Uniform used to be used throughout the engine I will be switching to Descriptor to keep it closer to Vulkan terminology.

	/**
	* Class describing one descriptor set layout and it's contained descriptors.
	*/
	class DescriptorSetLayoutVulkan : public IGPUResourceVulkan {
	private:
		std::vector<AShaderDescriptor> mDescriptors;
		std::vector<VkDescriptorSetLayoutBinding> mBindings;
		VkDescriptorSetLayout mLayout;

	public:
		DescriptorSetLayoutVulkan(const std::vector<AShaderDescriptor>& descs)
		:	mDescriptors(descs),
			mLayout(VK_NULL_HANDLE)
		{}

		virtual ~DescriptorSetLayoutVulkan() override;

		virtual void close(VkDevice logicDevice) override;

		void init(VkDevice logicDevice);

		inline std::vector<AShaderDescriptor>& getDescriptors() { return mDescriptors; }
		inline const VkDescriptorSetLayout& getLayout() const { return mLayout; }

	private:
		void createDescriptorSetLayoutBindings();
		void createDescriptorSetLayout(VkDevice logicDevice);
	};

	/**
	* Class describing the layout of the shader descriptor sets and push constants (if any) used by it.
	* DOH Descriptor Sets follow Vulkan's Descriptor Set grouping.
	* Every shader program must have one even if no descriptor or push constants are used.
	*/
	class ShaderDescriptorSetLayoutsVulkan {
	private:
		std::vector<std::reference_wrapper<DescriptorSetLayoutVulkan>> mDescriptorSetLayouts;
		std::vector<VkPushConstantRange> mPushConstants;

	public:
		ShaderDescriptorSetLayoutsVulkan() = default;
		ShaderDescriptorSetLayoutsVulkan(const std::vector<VkPushConstantRange>& pushConstants)
		:	mPushConstants(pushConstants)
		{}
		ShaderDescriptorSetLayoutsVulkan(const std::vector<std::reference_wrapper<DescriptorSetLayoutVulkan>>& setLayouts)
		: mDescriptorSetLayouts(setLayouts)
		{}
		ShaderDescriptorSetLayoutsVulkan(const std::vector<VkPushConstantRange>& pushConstants, const std::vector<std::reference_wrapper<DescriptorSetLayoutVulkan>>& setLayouts)
		:	mPushConstants(pushConstants),
			mDescriptorSetLayouts(setLayouts)
		{}

		/**
		* Get the Descriptor Set Layout native Vulkan objects.
		* 
		* @returns Array of VkDescriptorSetLayout handles in order.
		*/
		std::vector<VkDescriptorSetLayout> getNativeSetLayouts() const;

		/**
		* Get the number of sets used in this layout.
		* 
		* @returns The number of sets used for this layout.
		*/
		inline uint32_t getSetCount() const { return static_cast<uint32_t>(mDescriptorSetLayouts.size()); }
		
		/**
		* Cycle through the shader's descriptor sets and count the unique individual descriptors.
		* Indexes of each type match EShaderDescriptorType order.
		* 
		* @returns Array of the count of each different EShaderDescriptorType.
		*/
		std::array<uint32_t, 4> getDescriptorCounts() const;
		inline uint32_t getPushConstantCount() const { return static_cast<uint32_t>(mPushConstants.size()); }
		inline const std::vector<std::reference_wrapper<DescriptorSetLayoutVulkan>>& getSets() const { return mDescriptorSetLayouts; }
		inline const std::vector<VkPushConstantRange>& getPushConstants() const { return mPushConstants; }

		uint32_t getPushConstantOffset() const;
	};

	/**
	* Class describing the Descriptor Sets to be bound to their slot (array index) when required.
	*/
	class DescriptorSetsInstanceVulkan {
	private:
		std::vector<VkDescriptorSet> mDescriptorSets;
	
	public:
		DescriptorSetsInstanceVulkan(std::initializer_list<VkDescriptorSet> descSets)
		:	mDescriptorSets(descSets)
		{}
	
		inline std::vector<VkDescriptorSet>& getDescriptorSets() { return mDescriptorSets; }
	};
}
