#include "dough/rendering/pipeline/DescriptorApiVulkan.h"

#include "dough/rendering/pipeline/ShaderDescriptorSetLayoutsVulkan.h"
#include "dough/rendering/pipeline/AShaderDescriptor.h"

#include <tracy/public/tracy/Tracy.hpp>

namespace DOH {

	VkDescriptorSet DescriptorApiVulkan::allocateDescriptorSetFromLayout(
		VkDevice logicDevice,
		VkDescriptorPool descPool,
		DescriptorSetLayoutVulkan& layout
	) {
		ZoneScoped;

		VkDescriptorSet descSet = VK_NULL_HANDLE;

		VkDescriptorSetAllocateInfo allocation = {};
		allocation.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocation.descriptorPool = descPool;
		allocation.descriptorSetCount = 1;
		allocation.pSetLayouts = &layout.getLayout();

		VK_TRY(
			vkAllocateDescriptorSets(logicDevice, &allocation, &descSet),
			"Failed to allocate descriptor set"
		);

		return descSet;
	}

	std::vector<VkDescriptorSet> DescriptorApiVulkan::allocateDescriptorSetsFromLayout(
		VkDevice logicDevice,
		VkDescriptorPool descPool,
		DescriptorSetLayoutVulkan& layout,
		uint32_t count
	) {
		ZoneScoped;

		std::vector<VkDescriptorSet> descSets = {};
		descSets.resize(count);

		std::vector<VkDescriptorSetLayout> layouts = {};
		layouts.reserve(count);
		for (uint32_t i = 0; i < count; i++) {
			layouts.emplace_back(layout.getLayout());
		}

		VkDescriptorSetAllocateInfo allocation = {};
		allocation.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocation.descriptorPool = descPool;
		allocation.descriptorSetCount = count;
		allocation.pSetLayouts = layouts.data();

		VK_TRY(
			vkAllocateDescriptorSets(logicDevice, &allocation, descSets.data()),
			"Failed to allocate descriptor sets"
		);

		return descSets;
	}

	void DescriptorApiVulkan::updateDescriptorSet(VkDevice logicDevice, DescriptorSetUpdate& setUpdate) {
		ZoneScoped;

		std::vector<VkWriteDescriptorSet> descWrites = {};
		std::vector<VkDescriptorBufferInfo> bufferInfos;
		std::vector<VkDescriptorImageInfo> imageInfos;
		descWrites.resize(setUpdate.Updates.size());
		uint32_t writeIndex = 0;
		uint32_t bufferIndex = 0;
		uint32_t imageIndex = 0;
		//Number of uniform updates that don't match between binding type and update type. +1 to account for Enum NONE = 0 and std::variant not including a "NONE" value
		uint32_t failedCount = 0;

		for (const DescriptorUpdate& update : setUpdate.Updates) {
			VkWriteDescriptorSet write = {};
			write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write.dstSet = setUpdate.DescSet;
			write.dstBinding = update.Descriptor.getBinding();
			write.dstArrayElement = 0;

			switch (update.Descriptor.getType()) {
				case EShaderDescriptorType::VALUE:
				{
					if ((update.UpdateInstance.index() + 1) != static_cast<size_t>(EShaderDescriptorType::VALUE)) {
						LOG_ERR("Descriptor Update instance does NOT match. Descriptor type: " <<
							EShaderDescriptorTypeStrings[static_cast<uint32_t>(EShaderDescriptorType::VALUE)] <<
							" update type: " << EShaderDescriptorTypeStrings[update.UpdateInstance.index() + 1]
						);
						failedCount++;
						break;
					}
					std::reference_wrapper<BufferVulkan> value = std::get<std::reference_wrapper<BufferVulkan>>(update.UpdateInstance);
					VkDescriptorBufferInfo bufferInfo = {};
					bufferInfo.buffer = value.get().getBuffer();
					bufferInfo.offset = 0;
					bufferInfo.range = value.get().getSize();

					bufferInfos.emplace_back(bufferInfo);

					write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
					write.descriptorCount = 1;
					write.pBufferInfo = &bufferInfos[bufferIndex];

					bufferIndex++;
					break;
				}

				case EShaderDescriptorType::TEXTURE:
				{
					if ((update.UpdateInstance.index() + 1) != static_cast<size_t>(EShaderDescriptorType::TEXTURE)) {
						LOG_ERR("Descriptor Update instance does NOT match. Descriptor type: " <<
							EShaderDescriptorTypeStrings[static_cast<uint32_t>(EShaderDescriptorType::TEXTURE)] <<
							" update type: " << EShaderDescriptorTypeStrings[update.UpdateInstance.index() + 1]
						);
						failedCount++;
						break;
					}
					std::reference_wrapper<TextureVulkan> texture = std::get<std::reference_wrapper<TextureVulkan>>(update.UpdateInstance);
					VkDescriptorImageInfo imageInfo = {};
					imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
					imageInfo.imageView = texture.get().getImageView();
					imageInfo.sampler = texture.get().getSampler();

					imageInfos.emplace_back(imageInfo);

					write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
					write.descriptorCount = 1;
					write.pImageInfo = &imageInfos[imageIndex];

					imageIndex++;
					break;
				}

				case EShaderDescriptorType::TEXTURE_ARRAY:
				{
					if ((update.UpdateInstance.index() + 1) != static_cast<size_t>(EShaderDescriptorType::TEXTURE_ARRAY)) {
						LOG_ERR("Descriptor Update instance does NOT match. Descriptor type: " <<
							EShaderDescriptorTypeStrings[static_cast<uint32_t>(EShaderDescriptorType::TEXTURE_ARRAY)] <<
							" update type: " << EShaderDescriptorTypeStrings[update.UpdateInstance.index() + 1]
						);
						failedCount++;
						break;
					}
					std::reference_wrapper<TextureArray> texArrUniform = std::get<std::reference_wrapper<TextureArray>>(update.UpdateInstance);
					const TextureArray& texArr = texArrUniform.get();
					const uint32_t arrayImagesStart = imageIndex;

					for (std::reference_wrapper<TextureVulkan> texture : texArr.getTextureSlots()) {
						VkDescriptorImageInfo imageInfo = {};
						imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
						imageInfo.imageView = texture.get().getImageView();
						imageInfo.sampler = texture.get().getSampler();
						imageInfos.emplace_back(imageInfo);
						imageIndex++;
					}

					//Fill remaining spaces with fallback texture
					const uint32_t maxTextureCount = texArr.getMaxTextureCount();
					for (uint32_t i = texArr.getNextTextureSlotIndex(); i < maxTextureCount; i++) {
						VkDescriptorImageInfo imageInfo = {};
						imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
						imageInfo.imageView = texArr.getFallbackTexture().getImageView();
						imageInfo.sampler = texArr.getFallbackTexture().getSampler();
						imageInfos.emplace_back(imageInfo);
						imageIndex++;
					}

					write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
					write.descriptorCount = maxTextureCount;
					write.pImageInfo = &imageInfos[arrayImagesStart];

					break;
				}

				case EShaderDescriptorType::NONE:
				default:
				{
					LOG_WARN("EShaderDescriptorType not recognised or is NONE when trying to update descriptor sets.");
					break;
				}
			}

			descWrites[writeIndex] = write;
			writeIndex++;
		}

		vkUpdateDescriptorSets(
			logicDevice,
			static_cast<uint32_t>(descWrites.size()) - failedCount,
			descWrites.data(),
			0,
			nullptr
		);
	}

	VkDescriptorSetLayoutBinding DescriptorApiVulkan::createSetLayoutBinding(
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
