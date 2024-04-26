#include "dough/rendering/pipeline_2/ShaderUniformLayoutVulkan_2.h"

#include "dough/Logging.h"
#include "dough/Utils.h"
#include "dough/rendering/ObjInit.h"
#include "dough/rendering/pipeline_2/DescriptorApiVulkan.h"

namespace DOH {

	VkDescriptorSetLayoutBinding ShaderUniform::getLayoutBindingVulkan(const ShaderUniform& uniform) {
		switch (uniform.getUniformType()) {
			case EShaderUniformType::VALUE:
				return DescriptorApiVulkan::createSetLayoutBinding(
					VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
					VK_SHADER_STAGE_VERTEX_BIT,
					1,
					uniform.getBinding()
				);

			case EShaderUniformType::TEXTURE:
				return DescriptorApiVulkan::createSetLayoutBinding(
					VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
					VK_SHADER_STAGE_FRAGMENT_BIT,
					1,
					uniform.getBinding()
				);

			case EShaderUniformType::TEXTURE_ARRAY:
			{
				const auto& texArrUni = static_cast<const TextureArrayShaderUniform&>(uniform);
				return DescriptorApiVulkan::createSetLayoutBinding(
					VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
					VK_SHADER_STAGE_FRAGMENT_BIT,
					texArrUni.getTextureArray().getMaxTextureCount(),
					texArrUni.getBinding()
				);
			}

			case EShaderUniformType::NONE:
			default:
				LOG_ERR("ShaderUniformVulkan::getLayoutBinding Unknown or NONE ShaderUniformType!");
				return {};
		}
	}

	VkDescriptorType ShaderUniform::getUniformTypeNativeEnum(const ShaderUniform& uniform) {
		switch (uniform.getUniformType()) {
			case EShaderUniformType::VALUE: return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			case EShaderUniformType::TEXTURE: return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			case EShaderUniformType::TEXTURE_ARRAY: return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

			case EShaderUniformType::NONE:
			default:
				LOG_ERR("ShaderUniformVulkan::getUniformTypeNativeEnum Unknown or NONE ShaderUniformType!");
				return VK_DESCRIPTOR_TYPE_MAX_ENUM;
		}
	}

	ShaderUniformSetLayoutVulkan::~ShaderUniformSetLayoutVulkan() {
		if (isUsingGpuResource()) {
			LOG_ERR("ShaderUniformSetVulkan GPU resource NOT released before destructor was called. Uniforms: ");

			for (const auto& uniform : mUniforms) {
				LOG_ERR("Binding: " << uniform.getBinding() << " Type: " << EShaderUniformTypeStrings[static_cast<uint32_t>(uniform.getUniformType())]);
			}
		}
	}

	void ShaderUniformSetLayoutVulkan::close(VkDevice logicDevice) {
		vkDestroyDescriptorSetLayout(logicDevice, mLayout, nullptr);
		mUsingGpuResource = false;
	}

	void ShaderUniformSetLayoutVulkan::init(VkDevice logicDevice) {
		createDescriptorSetLayoutBindings();
		createDescriptorSetLayout(logicDevice);
	}

	void ShaderUniformSetLayoutVulkan::createDescriptorSetLayoutBindings() {
		mBindings.reserve(mUniforms.size());

		for (const auto& uniform : mUniforms) {
			mBindings.emplace_back(ShaderUniform::getLayoutBindingVulkan(uniform));
		}
	}

	void ShaderUniformSetLayoutVulkan::createDescriptorSetLayout(VkDevice logicDevice) {
		VkDescriptorSetLayoutCreateInfo dslCreateInfo = {};
		dslCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		dslCreateInfo.bindingCount = static_cast<uint32_t>(mBindings.size());
		dslCreateInfo.pBindings = mBindings.data();

		VK_TRY(
			vkCreateDescriptorSetLayout(logicDevice, &dslCreateInfo, nullptr, &mLayout),
			"Failed to create descriptor set layout."
		);

		mUsingGpuResource = true;
	}

	std::vector<VkDescriptorSetLayout> ShaderUniformLayoutVulkan_2::getNativeSetLayouts() const {
		std::vector<VkDescriptorSetLayout> layouts;
		layouts.reserve(mUniformSetLayouts.size());
		for (const auto& set : mUniformSetLayouts) {
			layouts.emplace_back(set.get().getLayout());
		}
		return layouts;
	}

	std::array<uint32_t, 4> ShaderUniformLayoutVulkan_2::getUniformCounts() const {
		std::array<uint32_t, 4> counts = { 0, 0, 0, 0 };
		for (const auto& set : mUniformSetLayouts) {
			for (const auto& uniform : set.get().getUniforms()) {
				switch (uniform.getUniformType()) {
					case EShaderUniformType::VALUE:
						counts[static_cast<uint32_t>(EShaderUniformType::VALUE)]++;
						break;
					case EShaderUniformType::TEXTURE:
						counts[static_cast<uint32_t>(EShaderUniformType::TEXTURE)]++;
						break;
					case EShaderUniformType::TEXTURE_ARRAY:
						counts[static_cast<uint32_t>(EShaderUniformType::TEXTURE_ARRAY)]++;
						break;

					case EShaderUniformType::NONE:
					default:
						LOG_WARN("EShaderUniformType not recognised or NONE.");
						counts[static_cast<uint32_t>(EShaderUniformType::NONE)]++;
						break;
				}
			}
		}
		return counts;
	}

	uint32_t ShaderUniformLayoutVulkan_2::getPushConstantOffset() const {
		uint32_t offset = 0;
		for (auto& pushConstant : mPushConstants) {
			offset += pushConstant.size;
		}
		return offset;
	}
}
