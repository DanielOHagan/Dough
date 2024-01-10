#pragma once

#include "dough/rendering/buffer/BufferVulkan.h"
#include "dough/rendering/pipeline/shader/ShaderUniformLayout.h"

namespace DOH {

	class DescriptorSetLayoutVulkan : public IGPUResourceVulkan {

	private:
		VkDescriptorSetLayout mDescriptorSetLayout;
		ShaderUniformLayout& mUniformLayout;

		std::vector<VkDescriptorSet> mDescriptorSets;
		std::vector<VkDescriptorSetLayoutBinding> mDescriptorSetLayoutBindings;

		std::unordered_map<uint32_t, std::vector<std::shared_ptr<BufferVulkan>>> mValueBufferMap;

		//TODO:: Awkward state tracking to protect from multiple creations.
		// Work around?
		bool mDescriptorSetLayoutCreated;
		bool mValueBuffersCreated;

	public:
		DescriptorSetLayoutVulkan(ShaderUniformLayout& uniformLayout);

		DescriptorSetLayoutVulkan(const DescriptorSetLayoutVulkan& copy) = delete;
		DescriptorSetLayoutVulkan operator=(const DescriptorSetLayoutVulkan& assignment) = delete;

		virtual ~DescriptorSetLayoutVulkan() override;
		virtual void close(VkDevice logicDevice) override;
		bool isUsingGpuResource() const override { return mDescriptorSetLayoutCreated || mValueBuffersCreated; }

		void createDescriptorSetLayout(VkDevice logicDevice);
		void createValueBuffers(VkDevice logicDevice, VkPhysicalDevice physicalDevice, uint32_t count);
		void createDescriptorSets(VkDevice logicDevice, uint32_t descSetCount, VkDescriptorPool descPool);
		void createDescriptorSetLayoutBindings(VkDevice logicDevice, uint32_t count);

		void updateAllDescriptorSets(VkDevice logicDevice, uint32_t imageCount);
		void bindDescriptorSets(VkCommandBuffer cmdBuffer, VkPipelineLayout pipelineLayout, uint32_t imageIndex);
		std::vector<DescriptorTypeInfo> asDescriptorTypes() const;

		void closeBuffers(VkDevice logicDevice);
		void closeDescriptorSetLayout(VkDevice logicDevice);

		inline const VkDescriptorSetLayout& getDescriptorSetLayout() const { return mDescriptorSetLayout; }
		inline std::vector<std::shared_ptr<BufferVulkan>>& getBuffersFromBinding(uint32_t binding) { return mValueBufferMap.at(binding); }

		static VkDescriptorSetLayoutBinding createLayoutBinding(
			VkDescriptorType descriptorType,
			VkShaderStageFlags stages,
			uint32_t descriptorCount,
			uint32_t binding
		);
	};
}
