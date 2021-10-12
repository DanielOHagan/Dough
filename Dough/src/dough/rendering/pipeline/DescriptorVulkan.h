#pragma once

#include <map>

#include "dough/Utils.h"
#include "dough/rendering/buffer/BufferVulkan.h"

using TextureDescriptorInfo = std::pair<VkImageView, VkSampler>;

namespace DOH {

	class DescriptorVulkan {

	private:

		VkDescriptorSetLayout mDescriptorSetLayout;
		VkDeviceSize mBufferSize;
		std::vector<VkDescriptorSet> mDescriptorSets;
		std::vector<BufferVulkan> mBuffers;
		
		//TEMP::
		std::map<uint32_t, TextureDescriptorInfo> mTextureMap;

	public:
		DescriptorVulkan(size_t bufferSize);

		void createDescriptorSetLayout(VkDevice logicDevice, std::vector<VkDescriptorSetLayoutBinding>& layoutBinding, uint32_t bindingCount);
		void createBuffers(VkDevice logicDevice, VkPhysicalDevice physicalDevice, size_t count);
		void createDescriptorSets(VkDevice logicDevice, size_t count, VkDescriptorPool descPool);

		void updateDescriptorSets(VkDevice logicDevice, size_t count);
		void bindDescriptorSets(VkCommandBuffer cmdBuffer, VkPipelineLayout pipelineLayout, size_t descriptorSetIndex);

		void setTexture(uint32_t binding, VkImageView imageView, VkSampler sampler);

		void close(VkDevice logicDevice);

		inline void setBufferSize(size_t size) { mBufferSize = size; }

		inline const VkDescriptorSetLayout& getDescriptorSetLayout() const { return mDescriptorSetLayout; }
		inline std::vector<BufferVulkan>& getBuffers() { return mBuffers; }

		static VkDescriptorSetLayoutBinding createLayoutBinding(
			VkDescriptorType descriptorType,
			VkShaderStageFlags stages,
			uint32_t descriptorCount,
			uint32_t binding
		);
	};
}
