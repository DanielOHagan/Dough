#include "dough/rendering/DescriptorVulkan.h"

namespace DOH {

	DescriptorVulkan::DescriptorVulkan(size_t bufferSize)
	:	mBufferSize(bufferSize),
		mDescriptorSetLayout(VK_NULL_HANDLE)
	{
	}

	void DescriptorVulkan::createDescriptorSetLayout(
		VkDevice logicDevice,
		VkDescriptorSetLayoutBinding layoutBinding,
		uint32_t bindingCount
	) {
		VkDescriptorSetLayoutCreateInfo dslCreateInfo = {};
		dslCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		dslCreateInfo.bindingCount = bindingCount;
		dslCreateInfo.pBindings = &layoutBinding;

		TRY(
			vkCreateDescriptorSetLayout(logicDevice, &dslCreateInfo, nullptr, &mDescriptorSetLayout) != VK_SUCCESS,
			"Failed to create descriptor set layout."
		);
	}

	void DescriptorVulkan::createBuffers(VkDevice logicDevice, VkPhysicalDevice physicalDevice, size_t count) {
		mBuffers.resize(count);

		for (size_t i = 0; i < count; i++) {
			mBuffers[i] = BufferVulkan::createBuffer(
				logicDevice,
				physicalDevice,
				mBufferSize,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
			);
		}
	}

	void DescriptorVulkan::createDescriptorSets(VkDevice logicDevice, size_t count, VkDescriptorPool descPool) {
		std::vector<VkDescriptorSetLayout> layouts(count, mDescriptorSetLayout);

		VkDescriptorSetAllocateInfo allocation = {};
		allocation.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocation.descriptorPool = descPool;
		allocation.descriptorSetCount = static_cast<uint32_t>(count);
		allocation.pSetLayouts = layouts.data();

		mDescriptorSets.resize(count);

		TRY(
			vkAllocateDescriptorSets(logicDevice, &allocation, mDescriptorSets.data()) != VK_SUCCESS,
			"Failed to allocate descriptor sets."
		);
	}

	void DescriptorVulkan::updateDescriptorSets(VkDevice logicDevice, size_t count) {
		for (size_t i = 0; i < count; i++) {
			VkDescriptorBufferInfo bufferInfo = {};
			bufferInfo.buffer = mBuffers[i].getBuffer();
			bufferInfo.offset = 0;
			bufferInfo.range = mBufferSize;

			VkWriteDescriptorSet descWrite = {};
			descWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descWrite.dstSet = mDescriptorSets[i];
			descWrite.dstBinding = 0;
			descWrite.dstArrayElement = 0;
			descWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descWrite.descriptorCount = 1;
			descWrite.pBufferInfo = &bufferInfo;

			vkUpdateDescriptorSets(logicDevice, 1, &descWrite, 0, nullptr);
		}
	}

	void DescriptorVulkan::bindDescriptorSets(
		VkCommandBuffer cmdBuffer,
		VkPipelineLayout pipelineLayout,
		size_t descriptorSetIndex
	) {
		vkCmdBindDescriptorSets(
			cmdBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			pipelineLayout,
			0,
			1,
			&mDescriptorSets[descriptorSetIndex],
			0,
			nullptr
		);
	}

	void DescriptorVulkan::close(VkDevice logicDevice) {
		for (BufferVulkan& buffers : mBuffers) {
			buffers.close(logicDevice);
		}

		vkDestroyDescriptorSetLayout(logicDevice, mDescriptorSetLayout, nullptr);
	}

	VkDescriptorSetLayoutBinding DescriptorVulkan::createLayoutBinding(
		VkDevice logicDevice,
		VkDescriptorType descriptorType,
		VkShaderStageFlags stages,
		uint32_t descriptorCount,
		uint32_t binding
	) {
		VkDescriptorSetLayoutBinding layoutBinding = {};
		layoutBinding.binding = binding;
		layoutBinding.descriptorType = descriptorType;
		layoutBinding.descriptorCount = descriptorCount;
		layoutBinding.stageFlags = stages;

		return layoutBinding;
	}
}
