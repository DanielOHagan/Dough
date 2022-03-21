#include "dough/rendering/pipeline/DescriptorVulkan.h"

#include "dough/rendering/ObjInit.h"

namespace DOH {

	DescriptorVulkan::DescriptorVulkan(ShaderUniformLayout& uniformLayout)
	:	mDescriptorSetLayout(VK_NULL_HANDLE),
		mUniformLayout(uniformLayout)
	{}

	void DescriptorVulkan::createDescriptorSetLayout(VkDevice logicDevice) {
		VkDescriptorSetLayoutCreateInfo dslCreateInfo = {};
		dslCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		dslCreateInfo.bindingCount = static_cast<uint32_t>(mUniformLayout.getDescriptorSetLayoutBindings().size());
		dslCreateInfo.pBindings = mUniformLayout.getDescriptorSetLayoutBindings().data();

		VK_TRY(
			vkCreateDescriptorSetLayout(logicDevice, &dslCreateInfo, nullptr, &mDescriptorSetLayout),
			"Failed to create descriptor set layout."
		);
	}

	void DescriptorVulkan::createValueBuffers(VkDevice logicDevice, VkPhysicalDevice physicalDevice, size_t count) {
		mValueBufferMap.clear();

		for (auto& [binding, value] : mUniformLayout.getValueUniformMap()) {
			mValueBufferMap.emplace(binding, std::vector<std::shared_ptr<BufferVulkan>>(count));
			
			for (size_t i = 0; i < count; i++) {
				mValueBufferMap[binding][i] = ObjInit::buffer(
					mUniformLayout.getValueUniformMap().at(binding),
					VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
					VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
				);
			}
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

		VK_TRY(
			vkAllocateDescriptorSets(logicDevice, &allocation, mDescriptorSets.data()),
			"Failed to allocate descriptor sets."
		);
	}

	void DescriptorVulkan::updateDescriptorSets(VkDevice logicDevice, size_t count) {
		const uint32_t preTextureWriteCount = static_cast<uint32_t>(mUniformLayout.getValueUniformMap().size());
		for (size_t swapChainImageIndex = 0; swapChainImageIndex < count; swapChainImageIndex++) {
			std::vector<VkWriteDescriptorSet> descWrites{};
			std::vector<VkDescriptorImageInfo> imageInfos{};
			for (uint32_t i = 0; i < mUniformLayout.getTotalUniformCount(); i++) {
				VkWriteDescriptorSet write{};
				descWrites.push_back(write);

				std::map<uint32_t, TextureUniformInfo>::iterator it = mUniformLayout.getTextureUniformMap().find(i);
				if (it != mUniformLayout.getTextureUniformMap().end()) {
					//TODO:: This assumes the textures are the only things for 'i' to cycle through to 'writeCount', 
					//			will need to be changed when others are available
					VkDescriptorImageInfo imgInfo{};
					imageInfos.push_back(imgInfo);
				}
			}

			for (const auto& [key, value] : mUniformLayout.getValueUniformMap()) {
				VkDescriptorBufferInfo bufferInfo = {};
				bufferInfo.buffer = mValueBufferMap.at(key).at(swapChainImageIndex)->getBuffer();
				bufferInfo.offset = 0;
				bufferInfo.range = value;

				descWrites[key].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descWrites[key].dstSet = mDescriptorSets[swapChainImageIndex];
				descWrites[key].dstBinding = key;
				descWrites[key].dstArrayElement = 0;
				descWrites[key].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				descWrites[key].descriptorCount = 1;
				descWrites[key].pBufferInfo = &bufferInfo;
			}

			//Attached Textures
			for (const auto& [key, value] : mUniformLayout.getTextureUniformMap()) {
				const uint32_t imageKey = key > 0 ? key - preTextureWriteCount : 0;
				descWrites[key].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descWrites[key].dstSet = mDescriptorSets[swapChainImageIndex];
				//descWrites[descWriteIndex].dstBinding = 1; //Sampler binding destination (NOTE:: it's 1 because the UBO is 0 and the sampler is set to "(binding = 1)" in frag shader)
				//descWrites[descWriteIndex].dstArrayElement = key;
				descWrites[key].dstBinding = key;
				descWrites[key].dstArrayElement = 0;
				descWrites[key].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				descWrites[key].descriptorCount = 1;
				imageInfos[imageKey].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				imageInfos[imageKey].imageView = value.first;
				imageInfos[imageKey].sampler = value.second;
				descWrites[key].pImageInfo = &imageInfos[imageKey];
			}

			vkUpdateDescriptorSets(
				logicDevice,
				static_cast<uint32_t>(descWrites.size()),
				descWrites.data(),
				0,
				nullptr
			);
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

	void DescriptorVulkan::closeBuffers(VkDevice logicDevice) {
		for (auto& [binding, buffers] : mValueBufferMap) {
			for (std::shared_ptr<BufferVulkan> buffer : buffers) {
				buffer->close(logicDevice);
			}
		}
	}

	void DescriptorVulkan::close(VkDevice logicDevice) {
		closeBuffers(logicDevice);

		vkDestroyDescriptorSetLayout(logicDevice, mDescriptorSetLayout, nullptr);
	}

	VkDescriptorSetLayoutBinding DescriptorVulkan::createLayoutBinding(
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
		layoutBinding.pImmutableSamplers = nullptr;
		return layoutBinding;
	}
}
