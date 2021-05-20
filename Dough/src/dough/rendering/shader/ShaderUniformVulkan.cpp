#include "dough/rendering/shader/ShaderUniformVulkan.h"

namespace DOH {

	ShaderUniformVulkan::ShaderUniformVulkan(size_t bufferSize)
	:	mBufferSize(bufferSize),
		mDescriptorSetLayout(VK_NULL_HANDLE)
	{
	}

	void ShaderUniformVulkan::createDescriptorSetLayout(VkDevice logicDevice) {
		VkDescriptorSetLayoutBinding layoutBinding = {};
		layoutBinding.binding = 0;
		layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		layoutBinding.descriptorCount = 1;
		layoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

		VkDescriptorSetLayoutCreateInfo dslCreateInfo = {};
		dslCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		dslCreateInfo.bindingCount = 1;
		dslCreateInfo.pBindings = &layoutBinding;

		TRY(
			vkCreateDescriptorSetLayout(logicDevice, &dslCreateInfo, nullptr, &mDescriptorSetLayout) != VK_SUCCESS,
			"Failed to create descriptor set layout."
		);
	}

	void ShaderUniformVulkan::createBuffers(VkDevice logicDevice, VkPhysicalDevice physicalDevice, size_t count) {
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

	void ShaderUniformVulkan::createDescriptorSets(VkDevice logicDevice, size_t count, VkDescriptorPool descPool) {
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

	void ShaderUniformVulkan::updateDescriptorSets(VkDevice logicDevice, size_t count) {
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

	void ShaderUniformVulkan::bindDescriptorSets(
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

	void ShaderUniformVulkan::close(VkDevice logicDevice) {
		for (BufferVulkan& buffers : mBuffers) {
			buffers.close(logicDevice);
		}

		vkDestroyDescriptorSetLayout(logicDevice, mDescriptorSetLayout, nullptr);
	}
}
