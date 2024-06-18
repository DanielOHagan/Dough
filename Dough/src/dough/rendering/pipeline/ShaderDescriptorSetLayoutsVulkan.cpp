#include "dough/rendering/pipeline/ShaderDescriptorSetLayoutsVulkan.h"

#include "dough/Logging.h"
#include "dough/Utils.h"
#include "dough/rendering/pipeline/DescriptorApiVulkan.h"
#include "dough/rendering/pipeline/AShaderDescriptor.h"

#include <tracy/public/tracy/Tracy.hpp>

namespace DOH {

	DescriptorSetLayoutVulkan::~DescriptorSetLayoutVulkan() {
		if (isUsingGpuResource()) {
			LOG_ERR("DescriptorSetLayoutVulkan GPU resource NOT released before destructor was called. Descriptors: ");

			for (const auto& desc : mDescriptors) {
				LOG_ERR("Binding: " << desc.getBinding() << " Type: " << EShaderDescriptorTypeStrings[static_cast<uint32_t>(desc.getType())]);
			}

			//NOTE:: This is to stop the IGPUResource::~IGPUReource from logging a misleading error message.
			mUsingGpuResource = false;
		}
	}

	void DescriptorSetLayoutVulkan::close(VkDevice logicDevice) {
		ZoneScoped;

		vkDestroyDescriptorSetLayout(logicDevice, mLayout, nullptr);
		mUsingGpuResource = false;
	}

	void DescriptorSetLayoutVulkan::init(VkDevice logicDevice) {
		ZoneScoped;

		createDescriptorSetLayoutBindings();
		createDescriptorSetLayout(logicDevice);
	}

	void DescriptorSetLayoutVulkan::createDescriptorSetLayoutBindings() {
		ZoneScoped;

		mBindings.reserve(mDescriptors.size());

		for (const auto& desc : mDescriptors) {
			mBindings.emplace_back(AShaderDescriptor::getLayoutBindingVulkan(desc));
		}
	}

	void DescriptorSetLayoutVulkan::createDescriptorSetLayout(VkDevice logicDevice) {
		ZoneScoped;

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

	std::vector<VkDescriptorSetLayout> ShaderDescriptorSetLayoutsVulkan::getNativeSetLayouts() const {
		ZoneScoped;

		std::vector<VkDescriptorSetLayout> layouts;
		layouts.reserve(mDescriptorSetLayouts.size());
		for (const auto& set : mDescriptorSetLayouts) {
			layouts.emplace_back(set.get().getLayout());
		}
		return layouts;
	}

	std::array<uint32_t, 4> ShaderDescriptorSetLayoutsVulkan::getDescriptorCounts() const {
		std::array<uint32_t, 4> counts = { 0, 0, 0, 0 };
		for (const auto& set : mDescriptorSetLayouts) {
			for (const auto& desc : set.get().getDescriptors()) {
				switch (desc.getType()) {
					case EShaderDescriptorType::VALUE:
						counts[static_cast<uint32_t>(EShaderDescriptorType::VALUE)]++;
						break;
					case EShaderDescriptorType::TEXTURE:
						counts[static_cast<uint32_t>(EShaderDescriptorType::TEXTURE)]++;
						break;
					case EShaderDescriptorType::TEXTURE_ARRAY:
						counts[static_cast<uint32_t>(EShaderDescriptorType::TEXTURE_ARRAY)]++;
						break;

					case EShaderDescriptorType::NONE:
					default:
						LOG_WARN("EShaderDescriptorType not recognised or NONE.");
						counts[static_cast<uint32_t>(EShaderDescriptorType::NONE)]++;
						break;
				}
			}
		}
		return counts;
	}

	uint32_t ShaderDescriptorSetLayoutsVulkan::getPushConstantOffset() const {
		uint32_t offset = 0;
		for (auto& pushConstant : mPushConstants) {
			offset += pushConstant.size;
		}
		return offset;
	}
}
