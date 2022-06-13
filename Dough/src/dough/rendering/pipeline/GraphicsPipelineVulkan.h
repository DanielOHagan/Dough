#pragma once

#include "dough/rendering/SwapChainVulkan.h"
#include "dough/rendering/pipeline/DescriptorVulkan.h"
#include "dough/rendering/TextureVulkan.h"
#include "dough/rendering/renderables/IRenderable.h"
#include "dough/rendering/pipeline/shader/ShaderProgramVulkan.h"

namespace DOH {

	class GraphicsPipelineVulkan {

	private:

		VkPipeline mGraphicsPipeline;
		VkPipelineLayout mGraphicsPipelineLayout;
		EVertexType mVertexType;
		ShaderProgramVulkan& mShaderProgram;
		std::vector<std::reference_wrapper<IRenderable>> mRenderableDrawList;
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
		inline void addRenderableToDraw(IRenderable& renderable) { mRenderableDrawList.push_back(renderable); }
		inline void clearRenderableToDraw() { mRenderableDrawList.clear(); }
		void bind(VkCommandBuffer cmdBuffer);
		void close(VkDevice logicDevice);
		void recreate(VkDevice logicDevice, VkExtent2D extent, VkRenderPass renderPass);

		inline VkPipelineLayout getPipelineLayout() const { return mGraphicsPipelineLayout; }
		inline DescriptorVulkan& getShaderDescriptor() const { return mShaderProgram.getShaderDescriptor(); }
		inline ShaderProgramVulkan& getShaderProgram() const { return mShaderProgram; }
		inline bool isReady() const { return mGraphicsPipeline != VK_NULL_HANDLE; }
		inline uint32_t getVaoDrawCount() const { return static_cast<uint32_t>(mRenderableDrawList.size()); }
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

	class PipelineRenderableConveyer {

	private:
		std::optional<std::reference_wrapper<GraphicsPipelineVulkan>> mPipeline;

	public:
		PipelineRenderableConveyer()
		:	mPipeline()
		{}
		PipelineRenderableConveyer(std::reference_wrapper<GraphicsPipelineVulkan> pipeline)
		:	mPipeline(pipeline)
		{}

		inline bool isValid() const { return mPipeline.has_value(); }
		inline void addRenderable(IRenderable& renderable) { mPipeline.value().get().addRenderableToDraw(renderable); }
	};
}
