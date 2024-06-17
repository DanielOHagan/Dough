#include "dough/rendering/pipeline/AShaderDescriptor.h"

namespace DOH {

	VkDescriptorSetLayoutBinding AShaderDescriptor::getLayoutBindingVulkan(const AShaderDescriptor& desc) {
		switch (desc.getType()) {
			case EShaderDescriptorType::VALUE:
				return DescriptorApiVulkan::createSetLayoutBinding(
					VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
					VK_SHADER_STAGE_VERTEX_BIT,
					1,
					desc.getBinding()
				);

			case EShaderDescriptorType::TEXTURE:
				return DescriptorApiVulkan::createSetLayoutBinding(
					VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
					VK_SHADER_STAGE_FRAGMENT_BIT,
					1,
					desc.getBinding()
				);

			case EShaderDescriptorType::TEXTURE_ARRAY:
			{
				const auto& texArrUni = static_cast<const TextureArrayDescriptor&>(desc);
				return DescriptorApiVulkan::createSetLayoutBinding(
					VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
					VK_SHADER_STAGE_FRAGMENT_BIT,
					texArrUni.getTextureArray().getMaxTextureCount(),
					texArrUni.getBinding()
				);
			}

			case EShaderDescriptorType::NONE:
			default:
				LOG_ERR("AShaderDescriptor::getLayoutBinding Unknown or NONE EShaderDescriptorType!");
				return {};
		}
	}

	VkDescriptorType AShaderDescriptor::getTypeVulkanEnum(const AShaderDescriptor& desc) {
		switch (desc.getType()) {
			case EShaderDescriptorType::VALUE: return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			case EShaderDescriptorType::TEXTURE: return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			case EShaderDescriptorType::TEXTURE_ARRAY: return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

			case EShaderDescriptorType::NONE:
			default:
				LOG_ERR("AShaderDescriptor::getUniformTypeNativeEnum Unknown or NONE EShaderDescriptorType!");
				return VK_DESCRIPTOR_TYPE_MAX_ENUM;
		}
	}
}
