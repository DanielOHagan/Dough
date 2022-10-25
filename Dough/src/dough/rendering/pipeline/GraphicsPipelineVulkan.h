#pragma once

#include "dough/rendering/pipeline/DescriptorVulkan.h"
#include "dough/rendering/textures/TextureVulkan.h"
#include "dough/rendering/renderables/IRenderable.h"
#include "dough/rendering/pipeline/shader/ShaderProgramVulkan.h"

namespace DOH {

	enum class ERenderPass;

	struct GraphicsPipelineInstanceInfo {
		GraphicsPipelineInstanceInfo(
			EVertexType vertexType,
			ShaderProgramVulkan& shaderProgram,
			ERenderPass renderPass
		) : VertexType(vertexType),
			ShaderProgram(shaderProgram),
			RenderPass(renderPass)
		{}

		//TODO:: More fields, builder class?
		EVertexType VertexType;
		ShaderProgramVulkan& ShaderProgram;
		ERenderPass RenderPass;

		VkPolygonMode PolygonMode = VK_POLYGON_MODE_FILL;
		VkCullModeFlags CullMode = VK_CULL_MODE_BACK_BIT;
		VkFrontFace FrontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	};

	class GraphicsPipelineVulkan {

	private:

		VkPipeline mGraphicsPipeline;
		VkPipelineLayout mGraphicsPipelineLayout;
		GraphicsPipelineInstanceInfo& mInstanceInfo;
		std::vector<std::shared_ptr<IRenderable>> mRenderableDrawList;
		bool mEnabled;

	public:
		GraphicsPipelineVulkan() = delete;
		GraphicsPipelineVulkan(const GraphicsPipelineVulkan& copy) = delete;
		GraphicsPipelineVulkan operator=(const GraphicsPipelineVulkan& assignment) = delete;

		GraphicsPipelineVulkan(
			VkDevice logicDevice,
			GraphicsPipelineInstanceInfo& creationInfo,
			VkRenderPass renderPass,
			VkExtent2D extent
		);

		void createUniformObjects(VkDevice logicDevice);
		void uploadShaderUniforms(
			VkDevice logicDevice,
			VkPhysicalDevice physicalDevice,
			uint32_t imageCount,
			VkDescriptorPool descPool
		);
		void recordDrawCommands(uint32_t imageIndex, VkCommandBuffer cmd);
		inline void addRenderableToDraw(std::shared_ptr<IRenderable> renderable) { mRenderableDrawList.emplace_back(renderable); }
		inline void clearRenderableToDraw() { mRenderableDrawList.clear(); }
		void bind(VkCommandBuffer cmdBuffer);
		void close(VkDevice logicDevice);
		void recreate(VkDevice logicDevice, VkExtent2D extent, VkRenderPass renderPass);

		inline VkPipelineLayout getPipelineLayout() const { return mGraphicsPipelineLayout; }
		inline DescriptorVulkan& getShaderDescriptor() const { return mInstanceInfo.ShaderProgram.getShaderDescriptor(); }
		inline ShaderProgramVulkan& getShaderProgram() const { return mInstanceInfo.ShaderProgram; }
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
		inline void addRenderable(std::shared_ptr<IRenderable> renderable) { mPipeline->get().addRenderableToDraw(renderable); }
		inline void safeAddRenderable(std::shared_ptr<IRenderable> renderable) { if (isValid()) { mPipeline->get().addRenderableToDraw(renderable); }}
	};
}
