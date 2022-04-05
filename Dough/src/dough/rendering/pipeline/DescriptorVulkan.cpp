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

		mUsingGpuResource = true;
	}

	void DescriptorVulkan::createValueBuffers(VkDevice logicDevice, VkPhysicalDevice physicalDevice, size_t count) {
		mValueBufferMap.clear();

		bool bufferCreated = false;
		for (auto& [binding, value] : mUniformLayout.getValueUniformMap()) {
			mValueBufferMap.emplace(binding, std::vector<std::shared_ptr<BufferVulkan>>(count));
			
			for (size_t i = 0; i < count; i++) {
				mValueBufferMap[binding][i] = ObjInit::buffer(
					mUniformLayout.getValueUniformMap().at(binding),
					VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
					VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
				);
			}

			bufferCreated = true;
		}

		if (bufferCreated) {
			mUsingGpuResource = true;
		}
	}

	void DescriptorVulkan::createDescriptorSets(VkDevice logicDevice, size_t descSetCount, VkDescriptorPool descPool) {
		std::vector<VkDescriptorSetLayout> layouts(descSetCount, mDescriptorSetLayout);

		VkDescriptorSetAllocateInfo allocation = {};
		allocation.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocation.descriptorPool = descPool;
		allocation.descriptorSetCount = static_cast<uint32_t>(descSetCount);
		allocation.pSetLayouts = layouts.data();

		mDescriptorSets.resize(descSetCount);

		VK_TRY(
			vkAllocateDescriptorSets(logicDevice, &allocation, mDescriptorSets.data()),
			"Failed to allocate descriptor sets."
		);

		mUsingGpuResource = true;
	}

	void DescriptorVulkan::updateDescriptorSets(VkDevice logicDevice, size_t imageCount) {
		for (size_t swapChainImageIndex = 0; swapChainImageIndex < imageCount; swapChainImageIndex++) {
			std::vector<VkWriteDescriptorSet> descWrites(mUniformLayout.getTotalUniformCount());
			std::vector<VkDescriptorImageInfo> imageInfos(mUniformLayout.getTotalTextureCount());

			uint32_t writeIndex = 0;
			uint32_t imageIndex = 0;

			//Attached values
			for (const auto& [binding, value] : mUniformLayout.getValueUniformMap()) {
				VkWriteDescriptorSet write = {};
				VkDescriptorBufferInfo bufferInfo = {};
				bufferInfo.buffer = mValueBufferMap.at(binding).at(swapChainImageIndex)->getBuffer();
				bufferInfo.offset = 0;
				bufferInfo.range = value;

				write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				write.dstSet = mDescriptorSets[swapChainImageIndex];
				write.dstBinding = binding;
				write.dstArrayElement = 0;
				write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				write.descriptorCount = 1;
				write.pBufferInfo = &bufferInfo;

				descWrites[writeIndex] = write;
				writeIndex++;
			}

			//Attached single textures
			for (const auto& [binding, texture] : mUniformLayout.getTextureUniformMap()) {
				VkWriteDescriptorSet write = {};
				VkDescriptorImageInfo imageInfo = {};

				imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				imageInfo.imageView = texture.first;
				imageInfo.sampler = texture.second;

				imageInfos[imageIndex] = imageInfo;

				write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				write.dstSet = mDescriptorSets[swapChainImageIndex];
				write.dstBinding = binding;
				write.dstArrayElement = 0;
				write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				write.descriptorCount = 1;
				write.pImageInfo = &imageInfos[imageIndex];

				descWrites[writeIndex] = write;

				writeIndex++;
				imageIndex++;
			}

			//Attached texture arrays
			for (const auto& [binding, texArray] : mUniformLayout.getTextureArrayUniformMap()) {
				VkWriteDescriptorSet write = {};

				const uint32_t arrayImagesStart = imageIndex;
				for (const TextureUniformInfo& info : texArray.TextureUniforms) {
					VkDescriptorImageInfo imageInfo = {};
					imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
					imageInfo.imageView = info.first;
					imageInfo.sampler = info.second;
					imageInfos[imageIndex] = imageInfo;
					imageIndex++;
				}

				write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				write.dstSet = mDescriptorSets[swapChainImageIndex];
				write.dstBinding = binding;
				write.dstArrayElement = 0;
				write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				write.descriptorCount = texArray.Count;
				write.pImageInfo = &imageInfos[arrayImagesStart];

				descWrites[writeIndex] = write;
				writeIndex++;
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

		mUsingGpuResource = false;
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
