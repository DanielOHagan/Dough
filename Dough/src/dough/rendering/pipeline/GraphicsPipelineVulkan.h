#pragma once

#include "dough/rendering/SwapChainVulkan.h"
#include "dough/rendering/pipeline/DescriptorVulkan.h"
#include "dough/rendering/TextureVulkan.h"
#include "dough/rendering/buffer/VertexArrayVulkan.h"
#include "dough/rendering/pipeline/shader/ShaderProgramVulkan.h"

namespace DOH {

	class GraphicsPipelineVulkan {

	private:
		
		VkPipeline mGraphicsPipeline;
		VkPipelineLayout mGraphicsPipelineLayout;
		//NOTE:: Descriptor Pool may be used by other Pipeline
		//			instances so its lifetime is not matched to this
		VkDescriptorPool mDescriptorPool;

		ShaderProgramVulkan& mShaderProgram;
		std::vector<std::reference_wrapper<VertexArrayVulkan>> mVaoDrawList;

	public:
		GraphicsPipelineVulkan() = delete;
		GraphicsPipelineVulkan(const GraphicsPipelineVulkan& copy) = delete;
		GraphicsPipelineVulkan operator=(const GraphicsPipelineVulkan& assignment) = delete;

		GraphicsPipelineVulkan(ShaderProgramVulkan& shaderProgram);
		GraphicsPipelineVulkan(
			VkDevice logicDevice,
			VkCommandPool cmdPool,
			VkExtent2D extent,
			VkRenderPass renderPass,
			ShaderProgramVulkan& shaderProgram,
			VkVertexInputBindingDescription vertexInputBindingDesc,
			std::vector<VkVertexInputAttributeDescription>& vertexAttributes
		);

		void createUniformObjects(VkDevice logicDevice);
		void uploadShaderUniforms(VkDevice logicDevice, VkPhysicalDevice physicalDevice, uint32_t imageCount);
		void recordDrawCommands(uint32_t imageIndex, VkCommandBuffer cmd);
		inline void addVaoToDraw(VertexArrayVulkan& vertexArray) { mVaoDrawList.push_back(vertexArray); }
		inline void clearVaoToDraw() { mVaoDrawList.clear(); }
		void bind(VkCommandBuffer cmdBuffer);
		void close(VkDevice logicDevice);

		inline VkPipelineLayout getPipelineLayout() const { return mGraphicsPipelineLayout; }
		inline void setDescriptorPool(VkDescriptorPool descPool) { mDescriptorPool = descPool; }
		inline const VkDescriptorPool getDescriptorPool() const { return mDescriptorPool; }
		inline DescriptorVulkan& getShaderDescriptor() const { return mShaderProgram.getShaderDescriptor(); }
		inline ShaderProgramVulkan& getShaderProgram() const { return mShaderProgram; }

		inline VkPipeline get() const { return mGraphicsPipeline; }

	private:
		void init(
			VkDevice logicDevice,
			VkVertexInputBindingDescription bindingDesc,
			std::vector<VkVertexInputAttributeDescription>& vertexAttributes,
			VkExtent2D extent,
			VkRenderPass renderPass
		);
	};
}
