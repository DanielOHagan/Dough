#include "dough/rendering/pipeline/DescriptorSetLayoutVulkan.h"

#include "dough/rendering/ObjInit.h"
#include "dough/Logging.h"

namespace DOH {

	DescriptorSetLayoutVulkan::DescriptorSetLayoutVulkan(ShaderUniformLayout& uniformLayout)
	:	mDescriptorSetLayout(VK_NULL_HANDLE),
		mUniformLayout(uniformLayout),
		mDescriptorSetLayoutCreated(false),
		mValueBuffersCreated(false)
	{}

	void DescriptorSetLayoutVulkan::createDescriptorSetLayout(VkDevice logicDevice) {
		if (mDescriptorSetLayoutCreated) {
			//TODO:: Since pipelines can share the same instance of this class and createDescLayout() is called for each pipeline
			// which results in this being called multiple times when it only needs it done once
			// When Shader programs are separated from pipeline ownership they will be owned by "current render states" or something like that
			// 
			// 
			//LOG_WARN("Attempting to create descriptor set layout layout when it already exists");
			return;
		}

		VkDescriptorSetLayoutCreateInfo dslCreateInfo = {};
		dslCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		dslCreateInfo.bindingCount = static_cast<uint32_t>(mDescriptorSetLayoutBindings.size());
		dslCreateInfo.pBindings = mDescriptorSetLayoutBindings.data();

		VK_TRY(
			vkCreateDescriptorSetLayout(logicDevice, &dslCreateInfo, nullptr, &mDescriptorSetLayout),
			"Failed to create descriptor set layout."
		);

		mDescriptorSetLayoutCreated = true;
	}

	void DescriptorSetLayoutVulkan::createValueBuffers(VkDevice logicDevice, VkPhysicalDevice physicalDevice, uint32_t count) {
		if (mValueBuffersCreated) {
			//TODO:: Since pipelines can share the same instance of this class and createDescLayout() is called for each pipeline
			// which results in this being called multiple times when it only needs it done once
			// When Shader programs are separated from pipeline ownership they will be owned by "current render states" or something like that
			// 
			// 
			//LOG_WARN("Attempting to create value buffers when they already exist.");
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

	void DescriptorSetLayoutVulkan::createDescriptorSets(VkDevice logicDevice, uint32_t descSetCount, VkDescriptorPool descPool) {
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

	void DescriptorSetLayoutVulkan::createDescriptorSetLayoutBindings(VkDevice logicDevice, uint32_t count) {
		mDescriptorSetLayoutBindings = std::vector<VkDescriptorSetLayoutBinding>(count);

		for (const auto& [binding, value] : mUniformLayout.getValueUniformMap()) {
			mDescriptorSetLayoutBindings[binding] = DescriptorSetLayoutVulkan::createLayoutBinding(
				VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
				VK_SHADER_STAGE_VERTEX_BIT,
				1,
				binding
			);
		}

		//Create textures' layout binding
		for (const auto& [binding, value] : mUniformLayout.getTextureUniformMap()) {
			mDescriptorSetLayoutBindings[binding] = DescriptorSetLayoutVulkan::createLayoutBinding(
				VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
				VK_SHADER_STAGE_FRAGMENT_BIT,
				1,
				binding
			);
		}

		//Create texture arrays' layout binding
		for (const auto& [binding, texArr] : mUniformLayout.getTextureArrayUniformMap()) {
			mDescriptorSetLayoutBindings[binding] = DescriptorSetLayoutVulkan::createLayoutBinding(
				VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
				VK_SHADER_STAGE_FRAGMENT_BIT,
				texArr.get().getMaxTextureCount(),
				binding
			);
		}
	}

	void DescriptorSetLayoutVulkan::updateAllDescriptorSets(VkDevice logicDevice, uint32_t imageCount) {
		for (uint32_t swapChainImageIndex = 0; swapChainImageIndex < imageCount; swapChainImageIndex++) {
			std::vector<VkWriteDescriptorSet> descWrites(mUniformLayout.getTotalUniformCount());
			std::vector<VkDescriptorBufferInfo> valueInfos(mUniformLayout.getTotalValueCount());
			std::vector<VkDescriptorImageInfo> imageInfos(mUniformLayout.getTotalTextureCount());

			uint32_t writeIndex = 0;
			uint32_t valueIndex = 0;
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

				valueInfos[valueIndex] = bufferInfo;
				write.pBufferInfo = &valueInfos[valueIndex];
				valueIndex++;

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

	void DescriptorSetLayoutVulkan::bindDescriptorSets(
		VkCommandBuffer cmdBuffer,
		VkPipelineLayout pipelineLayout,
		uint32_t imageIndex
	) {
		vkCmdBindDescriptorSets(
			cmdBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			pipelineLayout,
			0,
			1,
			&mDescriptorSets[imageIndex],
			0,
			nullptr
		);
	}

	void DescriptorSetLayoutVulkan::closeBuffers(VkDevice logicDevice) {
		for (auto& [binding, buffers] : mValueBufferMap) {
			for (std::shared_ptr<BufferVulkan> buffer : buffers) {
				buffer->close(logicDevice);
			}
		}

		mValueBuffersCreated = false;
	}

	void DescriptorSetLayoutVulkan::closeDescriptorSetLayout(VkDevice logicDevice) {
		if (mDescriptorSetLayout != VK_NULL_HANDLE) {
			vkDestroyDescriptorSetLayout(logicDevice, mDescriptorSetLayout, nullptr);
			mDescriptorSetLayout = VK_NULL_HANDLE;
			mDescriptorSetLayoutCreated = false;
		}
	}

	void DescriptorSetLayoutVulkan::close(VkDevice logicDevice) {
		closeBuffers(logicDevice);
		closeDescriptorSetLayout(logicDevice);

		mDescriptorSetLayoutBindings.clear();

		mUsingGpuResource = false;
	}

	std::vector<DescriptorTypeInfo> DescriptorSetLayoutVulkan::asDescriptorTypes() const {
		std::vector<DescriptorTypeInfo> descTypes = {};
		descTypes.reserve(mDescriptorSetLayoutBindings.size());

		for (const VkDescriptorSetLayoutBinding& layoutBinding : mDescriptorSetLayoutBindings) {
			descTypes.emplace_back(layoutBinding.descriptorType, layoutBinding.descriptorCount);
		}

		return descTypes;
	}

	VkDescriptorSetLayoutBinding DescriptorSetLayoutVulkan::createLayoutBinding(
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
