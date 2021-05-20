#pragma once

#include "dough/Utils.h"
#include "dough/rendering/buffer/BufferVulkan.h"


namespace DOH {

	class ShaderUniformVulkan {

	private:

		VkDescriptorSetLayout mDescriptorSetLayout;
		VkDeviceSize mBufferSize;
		std::vector<VkDescriptorSet> mDescriptorSets;
		std::vector<BufferVulkan> mBuffers;

	public:
		ShaderUniformVulkan(size_t bufferSize);

		void createDescriptorSetLayout(VkDevice logicDevice);
		void createBuffers(VkDevice logicDevice, VkPhysicalDevice physicalDevice, size_t count);
		void createDescriptorSets(VkDevice logicDevice, size_t count, VkDescriptorPool descPool);

		void updateDescriptorSets(VkDevice logicDevice, size_t count);
		void bindDescriptorSets(VkCommandBuffer cmdBuffer, VkPipelineLayout pipelineLayout, size_t descriptorSetIndex);

		void close(VkDevice logicDevice);

		inline void setBufferSize(size_t size) { mBufferSize = size; }

		inline const VkDescriptorSetLayout& getDescriptorSetLayout() const { return mDescriptorSetLayout; }
		inline std::vector<BufferVulkan>& getBuffers() { return mBuffers; }
	};
}