#include "dough/rendering/pipeline/DescriptorVulkan.h"

#include "dough/rendering/TextureVulkan.h"

namespace DOH {

	DescriptorVulkan::DescriptorVulkan(size_t bufferSize)
	:	mBufferSize(bufferSize),
		mDescriptorSetLayout(VK_NULL_HANDLE)
	{
	}

	void DescriptorVulkan::createDescriptorSetLayout(
		VkDevice logicDevice,
		std::vector<VkDescriptorSetLayoutBinding>& layoutBindings,
		uint32_t bindingCount
	) {
		VkDescriptorSetLayoutCreateInfo dslCreateInfo = {};
		dslCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		dslCreateInfo.bindingCount = bindingCount;
		dslCreateInfo.pBindings = layoutBindings.data();

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
		//TODO:: Could this be optimised by being created at pipeline creation and being re-used?
		//		Maybe a struct like which is stored when this pipeline is in use:
		//		struct DescriptorSets {
		//			std::vector<VkWriteDescriptorSet> DescWrites{};
		// 
		//			std::vector<VkDescriptorImageInfo> ImageInfos{};
		//			//... +Any Other inputs that require descriptors
		//		}
		//TODO:: clean this up when adding texture arrays

		const uint32_t preTextureWritesCount = 1;
		const uint32_t writeCount = preTextureWritesCount + static_cast<uint32_t>(mTextureMap.size());

		for (size_t i = 0; i < count; i++) {
			std::vector<VkWriteDescriptorSet> descWrites{};
			std::vector<VkDescriptorImageInfo> imageInfos{};
			for (uint32_t i = 0; i < writeCount; i++) {
				VkWriteDescriptorSet write{};
				descWrites.push_back(write);

				if (i >= preTextureWritesCount) {
					//TODO:: This assumes the textures are the only things for 'i' to cycle through to 'writeCount', 
					//			will need to be changed when others are available
					VkDescriptorImageInfo imgInfo{};
					imageInfos.push_back(imgInfo);
				}
			}

			VkDescriptorBufferInfo bufferInfo = {};
			bufferInfo.buffer = mBuffers[i].getBuffer();
			bufferInfo.offset = 0;
			bufferInfo.range = mBufferSize;

			uint32_t descWriteIndex = 0;
			descWrites[descWriteIndex].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descWrites[descWriteIndex].dstSet = mDescriptorSets[i];
			descWrites[descWriteIndex].dstBinding = descWriteIndex;
			descWrites[descWriteIndex].dstArrayElement = 0;
			descWrites[descWriteIndex].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descWrites[descWriteIndex].descriptorCount = 1;
			descWrites[descWriteIndex].pBufferInfo = &bufferInfo;
			descWriteIndex++;

			//Attached Textures
			for (const auto& [key, value] : mTextureMap) {
				descWrites[descWriteIndex].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descWrites[descWriteIndex].dstSet = mDescriptorSets[i];
				//descWrites[descWriteIndex].dstBinding = 1; //Sampler binding destination (NOTE:: it's 1 because the UBO is 0 and the sampler is set to "(binding = 1)" in frag shader)
				//descWrites[descWriteIndex].dstArrayElement = key;
				descWrites[descWriteIndex].dstBinding = descWriteIndex;
				descWrites[descWriteIndex].dstArrayElement = 0;
				descWrites[descWriteIndex].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				descWrites[descWriteIndex].descriptorCount = 1;
				imageInfos[key].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				imageInfos[key].imageView = value.first;
				imageInfos[key].sampler = value.second;
				descWrites[descWriteIndex].pImageInfo = &imageInfos[key];

				//descWrites[descWriteIndex].pImageInfo = &imageInfo;
				descWriteIndex++;
			}

			/*
			TODO::
			NOTE::
			HELP::
			Please...
				For some reason, the descWrites[descWriteIndex].pImageInfo = &imageInfo; is assigning the value to both descWrites[1] and descWrites[2] on the second iteration.
				Had to force a very horrible fix that isn't a fix, it just has two VkDescriptorImageInfo created before the loop.
				See if I can fix.
				Commented out code is ideal end product (with texture array in shader but that can come later)
			*/

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

	void DescriptorVulkan::setTexture(uint32_t binding, TextureDescriptorInfo texDescInfo) {
		mTextureMap.emplace(binding, texDescInfo);
	}

	void DescriptorVulkan::close(VkDevice logicDevice) {
		for (BufferVulkan& buffers : mBuffers) {
			buffers.close(logicDevice);
		}

		mTextureMap.clear();

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
