#pragma once

#include "dough/rendering/textures/TextureArray.h"
#include "dough/rendering/buffer/BufferVulkan.h"

#include <variant>

namespace DOH {

	class AShaderDescriptor;
	class DescriptorSetLayoutVulkan;

	using DescriptorLayout = std::variant<
		VkDeviceSize,
		std::reference_wrapper<TextureVulkan>,
		std::reference_wrapper<TextureArray>
	>;

	using DescriptorRef = std::variant<
		std::reference_wrapper<BufferVulkan>,
		std::reference_wrapper<TextureVulkan>,
		std::reference_wrapper<TextureArray>
	>;

	struct DescriptorUpdate {
		AShaderDescriptor& Descriptor;
		DescriptorRef UpdateInstance;
	};

	//TODO:: is it possible to turn this into a std::array<DescirptorUpdate, *template* Count> to save performing an allocation?
	struct DescriptorSetUpdate {
		std::vector<DescriptorUpdate> Updates;
		VkDescriptorSet DescSet;
	};

	class DescriptorApiVulkan {
	public:
		static VkDescriptorSet allocateDescriptorSetFromLayout(
			VkDevice logicDevice,
			VkDescriptorPool descPool,
			DescriptorSetLayoutVulkan& layout
		);
		static std::vector<VkDescriptorSet> allocateDescriptorSetsFromLayout(
			VkDevice logicDevice,
			VkDescriptorPool descPool,
			DescriptorSetLayoutVulkan& layout,
			uint32_t count
		);

		//TODO:: this handles all shader types, maybe add ones with just values/texture/textureArray to simplify and remove redundant checks/initialisations
		static void updateDescriptorSet(VkDevice logicDevice, DescriptorSetUpdate& update);
		//TODO:: update multiple descriptor set updates at once
		//static void updateDescriptorSets(VkDevice logicDevice, const std::vector<DescriptorSetUpdate>& updates);

		//TODO:: individual type update functions to save checking each update?
		//static void updateValueDescriptorSet(VkDevice logicDevice, DescriptorSetUpdate& valueUpdate);
		//static void updateTextureDescriptorSet(VkDevice logicDevice, DescriptorSetUpdate& textureUpdate);
		//static void updateTextureArrayDescriptorSet(VkDevice logicDevice, DescriptorSetUpdate& textureArrayUpdate);

		//TODO:: individual type update functions to save checking each update?
		//static void updateValueDescriptorSets(VkDevice logicDevice, const std::vector<DescriptorSetUpdate>& valueUpdates);
		//static void updateTextureDescriptorSets(VkDevice logicDevice, const std::vector<DescriptorSetUpdate>& textureUpdates);
		//static void updateTextureArrayDescriptorSets(VkDevice logicDevice, const std::vector<DescriptorSetUpdate>& textureArrayUpdates);

		
		static VkDescriptorSetLayoutBinding createSetLayoutBinding(
			VkDescriptorType descriptorType,
			VkShaderStageFlags stages,
			uint32_t descriptorCount,
			uint32_t binding
		);
	};
}
