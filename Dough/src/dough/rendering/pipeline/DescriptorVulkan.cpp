#include "dough/rendering/pipeline/DescriptorVulkan.h"

#include "dough/rendering/ObjInit.h"

namespace DOH {

	DescriptorVulkan::DescriptorVulkan(ShaderUniformLayout& uniformLayout)
	:	mDescriptorSetLayout(VK_NULL_HANDLE),
		mUniformLayout(uniformLayout),
		mDescriptorSetLayoutCreated(false),
		mValueBuffersCreated(false)
	{}

	void DescriptorVulkan::createDescriptorSetLayout(VkDevice logicDevice) {
		if (mDescriptorSetLayoutCreated) {
			return;
		}

		VkDescriptorSetLayoutCreateInfo dslCreateInfo = {};
		dslCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		dslCreateInfo.bindingCount = static_cast<uint32_t>(mUniformLayout.getDescriptorSetLayoutBindings().size());
		dslCreateInfo.pBindings = mUniformLayout.getDescriptorSetLayoutBindings().data();

		VK_TRY(
			vkCreateDescriptorSetLayout(logicDevice, &dslCreateInfo, nullptr, &mDescriptorSetLayout),
			"Failed to create descriptor set layout."
		);

		mDescriptorSetLayoutCreated = true;
	}

	void DescriptorVulkan::createValueBuffers(VkDevice logicDevice, VkPhysicalDevice physicalDevice, uint32_t count) {
		if (mValueBuffersCreated) {
			return;
		}

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

			mValueBuffersCreated = true;
		}
	}

	void DescriptorVulkan::createDescriptorSets(VkDevice logicDevice, uint32_t descSetCount, VkDescriptorPool descPool) {
		std::vector<VkDescriptorSetLayout> layouts(descSetCount, mDescriptorSetLayout);

		VkDescriptorSetAllocateInfo allocation = {};
		allocation.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocation.descriptorPool = descPool;
		allocation.descriptorSetCount = descSetCount;
		allocation.pSetLayouts = layouts.data();

		mDescriptorSets.resize(descSetCount);

		VK_TRY(
			vkAllocateDescriptorSets(logicDevice, &allocation, mDescriptorSets.data()),
			"Failed to allocate descriptor sets."
		);
	}

	void DescriptorVulkan::updateDescriptorSets(VkDevice logicDevice, uint32_t imageCount) {
		for (uint32_t swapChainImageIndex = 0; swapChainImageIndex < imageCount; swapChainImageIndex++) {
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
				for (std::reference_wrapper<TextureVulkan> texture : texArray.get().getTextureSlots()) {
					VkDescriptorImageInfo imageInfo = {};
					imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
					imageInfo.imageView = texture.get().getImageView();
					imageInfo.sampler = texture.get().getSampler();
					imageInfos[imageIndex] = imageInfo;
					imageIndex++;
				}

				//Fill remaining spaces with fallback texture 
				const TextureVulkan& fallbackTexture = texArray.get().getFallbackTexture();
				for (uint32_t i = texArray.get().getNextTextureSlotIndex(); i < texArray.get().getMaxTextureCount(); i++) {
					VkDescriptorImageInfo imageInfo = {};
					imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
					imageInfo.imageView = fallbackTexture.getImageView();
					imageInfo.sampler = fallbackTexture.getSampler();
					imageInfos[imageIndex] = imageInfo;
					imageIndex++;
				}

				write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				write.dstSet = mDescriptorSets[swapChainImageIndex];
				write.dstBinding = binding;
				write.dstArrayElement = 0;
				write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				write.descriptorCount = texArray.get().getMaxTextureCount();
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
		uint32_t descriptorSetIndex
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

		mValueBuffersCreated = false;
	}

	void DescriptorVulkan::closeDescriptorSetLayout(VkDevice logicDevice) {
		vkDestroyDescriptorSetLayout(logicDevice, mDescriptorSetLayout, nullptr);
		mDescriptorSetLayout = VK_NULL_HANDLE;
		mDescriptorSetLayoutCreated = false;
	}

	void DescriptorVulkan::close(VkDevice logicDevice) {
		closeBuffers(logicDevice);
		closeDescriptorSetLayout(logicDevice);

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
