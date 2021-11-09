#pragma once

#include "dough/rendering/buffer/BufferVulkan.h"
#include "dough/rendering/shader/ShaderUniformLayout.h"

namespace DOH {

	class DescriptorVulkan {

	private:
		VkDescriptorSetLayout mDescriptorSetLayout;
		ShaderUniformLayout& mUniformLayout;
		
		std::vector<VkDescriptorSet> mDescriptorSets;
		std::map<uint32_t, std::vector<std::shared_ptr<BufferVulkan>>> mValueBufferMap;
		
	public:
		DescriptorVulkan(ShaderUniformLayout& uniformLayout);

		void createDescriptorSetLayout(VkDevice logicDevice);
		void createValueBuffers(VkDevice logicDevice, VkPhysicalDevice physicalDevice, size_t count);
		void createDescriptorSets(VkDevice logicDevice, size_t count, VkDescriptorPool descPool);

		void updateDescriptorSets(VkDevice logicDevice, size_t count);
		void bindDescriptorSets(VkCommandBuffer cmdBuffer, VkPipelineLayout pipelineLayout, size_t descriptorSetIndex);

		void close(VkDevice logicDevice);

		inline const VkDescriptorSetLayout& getDescriptorSetLayout() const { return mDescriptorSetLayout; }
		inline std::vector<std::shared_ptr<BufferVulkan>>& getBuffersFromBinding(uint32_t binding) { return mValueBufferMap.at(binding); }

	public:
		static VkDescriptorSetLayoutBinding createLayoutBinding(
			VkDescriptorType descriptorType,
			VkShaderStageFlags stages,
			uint32_t descriptorCount,
			uint32_t binding
		);
	};
}
