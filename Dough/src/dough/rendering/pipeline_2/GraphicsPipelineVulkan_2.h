#pragma once

#include "dough/rendering/IGPUResourceVulkan.h"
#include "dough/rendering/pipeline_2/ShaderProgram_2.h"

#include <vector>

namespace DOH {

	class AVertexInputLayout;
	class IRenderable;
	enum class ERenderPass;
	struct CurrentBindingsState;

	/**
	* Fields used in the creation of a GraphicsPipeline that have default values
	* for easier creation.
	*/
	struct GraphicsPipelineOptionalFields_2 {
		GraphicsPipelineOptionalFields_2() = default;

		VkPolygonMode PolygonMode = VK_POLYGON_MODE_FILL;
		VkPrimitiveTopology Topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		VkFrontFace FrontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

		VkCompareOp DepthCompareOp = VK_COMPARE_OP_NEVER;

		VkBlendFactor ColourBlendSrcFactor = VK_BLEND_FACTOR_ONE;
		VkBlendFactor ColourBlendDstFactor = VK_BLEND_FACTOR_ZERO;
		VkBlendFactor AlphaBlendSrcFactor = VK_BLEND_FACTOR_ONE;
		VkBlendFactor AlphaBlendDstFactor = VK_BLEND_FACTOR_ZERO;
		VkBlendOp ColourBlendOp = VK_BLEND_OP_ADD;
		VkBlendOp AlphaBlendOp = VK_BLEND_OP_ADD;

		VkCullModeFlags CullMode = VK_CULL_MODE_BACK_BIT;

		bool BlendingEnabled = false;
		bool DepthTestingEnabled = false;
		//TODO:: have the objects drawn by this pipeline instance be controlled by a "DrawStream" object or modify the PipelineConveyor class
		bool ClearRenderablesAfterDraw = true;

		constexpr inline void setDepthTesting(bool enabled, VkCompareOp compareOp) {
			DepthTestingEnabled = enabled;
			DepthCompareOp = compareOp;
		}
		constexpr inline void setBlending(
			bool enabled,
			VkBlendOp colourBlendOp,
			VkBlendFactor colourSrcFactor,
			VkBlendFactor colourDstFactor,
			VkBlendOp alphaBlendOp,
			VkBlendFactor alphaSrcFactor,
			VkBlendFactor alphaDstFactor
		) {
			BlendingEnabled = enabled;
			ColourBlendOp = colourBlendOp;
			ColourBlendSrcFactor = colourSrcFactor;
			ColourBlendDstFactor = colourDstFactor;
			AlphaBlendOp = alphaBlendOp;
			AlphaBlendSrcFactor = alphaSrcFactor;
			AlphaBlendDstFactor = alphaDstFactor;
		}
	};

	class GraphicsPipelineInstanceInfo_2 {
	
	private:
		const AVertexInputLayout& mVertexInputLayout;
		ShaderProgram_2& mShaderProgram;
		ERenderPass mRenderPass;

		std::unique_ptr<GraphicsPipelineOptionalFields_2> mOptionalFields;

	public:
		GraphicsPipelineInstanceInfo_2(
			const AVertexInputLayout& vertexInputLayout,
			ShaderProgram_2& shaderProgram,
			ERenderPass renderPass
		) : mVertexInputLayout(vertexInputLayout),
			mShaderProgram(shaderProgram),
			mRenderPass(renderPass),
			mOptionalFields(nullptr)
		{}

		inline const AVertexInputLayout& getVertexInputLayout() const { return mVertexInputLayout; }
		inline ShaderProgram_2& getShaderProgram() const { return mShaderProgram; }
		inline ERenderPass getRenderPass() const { return mRenderPass; }
		inline GraphicsPipelineOptionalFields_2& getOptionalFields() const { return *mOptionalFields; }
		inline bool hasOptionalFields() const { return mOptionalFields != nullptr; }
		void enableOptionalFields();
	};

	class GraphicsPipelineVulkan_2 : public IGPUResourceVulkan {

	private:
		VkPipeline mGraphicsPipeline;
		VkPipelineLayout mGraphicsPipelineLayout;
		GraphicsPipelineInstanceInfo_2& mInstanceInfo;
		std::vector<std::shared_ptr<IRenderable>> mRenderableDrawList;

	public:
		GraphicsPipelineVulkan_2() = delete;
		GraphicsPipelineVulkan_2(const GraphicsPipelineVulkan_2& copy) = delete;
		GraphicsPipelineVulkan_2 operator=(const GraphicsPipelineVulkan_2& assignment) = delete;

		GraphicsPipelineVulkan_2(GraphicsPipelineInstanceInfo_2& instanceInfo);

		virtual ~GraphicsPipelineVulkan_2() override;
		virtual void close(VkDevice logicDevice) override;

		void init(VkDevice logicDevice, VkExtent2D extent, VkRenderPass renderPass);
		void recreate(VkDevice logicDevice, VkExtent2D extent, VkRenderPass renderPass);
		void recordDrawCommands(uint32_t imageIndex, VkCommandBuffer cmd, CurrentBindingsState& currentBindings);
		inline void addRenderableToDraw(std::shared_ptr<IRenderable> renderable) { mRenderableDrawList.emplace_back(renderable); }
		inline void clearRenderableToDraw() { mRenderableDrawList.clear(); }
		inline void bind(VkCommandBuffer cmd) const { vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, mGraphicsPipeline); }

		//Only for updating CurrentBindings instance
		inline VkPipeline get() const { return mGraphicsPipeline; }
		inline uint32_t getVaoDrawCount() const { return static_cast<uint32_t>(mRenderableDrawList.size()); }
		inline VkPipelineLayout getGraphicsPipelineLayout() const { return mGraphicsPipelineLayout; }

	private:
		void createPipelineLayout(VkDevice logicDevice);
		void createPipeline(VkDevice logicDevice, VkExtent2D extent, VkRenderPass renderPass);

	};
}
