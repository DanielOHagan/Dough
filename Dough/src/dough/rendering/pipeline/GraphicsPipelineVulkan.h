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
		EVertexType mVertexType;
		ShaderProgramVulkan& mShaderProgram;
		std::vector<std::reference_wrapper<VertexArrayVulkan>> mVaoDrawList;
		bool mEnabled;

	public:
		GraphicsPipelineVulkan() = delete;
		GraphicsPipelineVulkan(const GraphicsPipelineVulkan& copy) = delete;
		GraphicsPipelineVulkan operator=(const GraphicsPipelineVulkan& assignment) = delete;

		GraphicsPipelineVulkan(
			VkDevice logicDevice,
			VkCommandPool cmdPool,
			EVertexType vertexType,
			ShaderProgramVulkan& shaderProgram,
			VkExtent2D extent,
			VkRenderPass renderPass
		);

		void createUniformObjects(VkDevice logicDevice);
		void uploadShaderUniforms(
			VkDevice logicDevice,
			VkPhysicalDevice physicalDevice,
			uint32_t imageCount,
			VkDescriptorPool descPool
		);
		void recordDrawCommands(uint32_t imageIndex, VkCommandBuffer cmd);
		inline void addVaoToDraw(VertexArrayVulkan& vertexArray) { mVaoDrawList.push_back(vertexArray); }
		inline void clearVaoToDraw() { mVaoDrawList.clear(); }
		void bind(VkCommandBuffer cmdBuffer);
		void close(VkDevice logicDevice);
		void recreate(VkDevice logicDevice, VkExtent2D extent, VkRenderPass renderPass);

		inline VkPipelineLayout getPipelineLayout() const { return mGraphicsPipelineLayout; }
		inline DescriptorVulkan& getShaderDescriptor() const { return mShaderProgram.getShaderDescriptor(); }
		inline ShaderProgramVulkan& getShaderProgram() const { return mShaderProgram; }
		inline bool isReady() const { return mGraphicsPipeline != VK_NULL_HANDLE; }
		inline uint32_t getVaoDrawCount() const { return static_cast<uint32_t>(mVaoDrawList.size()); }
		inline VkPipeline get() const { return mGraphicsPipeline; }
		inline void setEnabled(bool enable) { mEnabled = enable; }
		inline bool isEnabled() const { return mEnabled; }

	private:
		void createPipelineLayout(VkDevice logicDevice);
		void createPipeline(
			VkDevice logicDevice,
			VkExtent2D extent,
			VkRenderPass renderPass
		);
	};
}
