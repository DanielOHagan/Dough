#pragma once

#include "dough/rendering/pipeline/DescriptorSetLayoutVulkan.h"
#include "dough/rendering/textures/TextureVulkan.h"
#include "dough/rendering/renderables/IRenderable.h"
#include "dough/rendering/pipeline/shader/ShaderProgramVulkan.h"
#include "dough/rendering/VertexInputLayout.h"

namespace DOH {

	enum class ERenderPass;
	struct CurrentBindingsState;

	/**
	* Fields used in the creation of a GraphicsPipeline that have default values
	* for easier creation.
	*/
	struct GraphicsPipelineOptionalFields {
		GraphicsPipelineOptionalFields() = default;

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

	class GraphicsPipelineInstanceInfo {

	private:
		const AVertexInputLayout& mVertexInputLayout;
		ShaderProgramVulkan& mShaderProgram;
		const ERenderPass mRenderPass;

		std::unique_ptr<GraphicsPipelineOptionalFields> mOptionalFields;

	public:
		GraphicsPipelineInstanceInfo(
			const AVertexInputLayout& vertexInputLayout,
			ShaderProgramVulkan& shaderProgram,
			ERenderPass renderPass
		) : mVertexInputLayout(vertexInputLayout),
			mShaderProgram(shaderProgram),
			mRenderPass(renderPass),
			mOptionalFields(std::make_unique<GraphicsPipelineOptionalFields>())
		{}

		inline ShaderProgramVulkan& getShaderProgram() const { return mShaderProgram; }
		inline const AVertexInputLayout& getVertexInputLayout() const { return mVertexInputLayout; }
		inline const ERenderPass getRenderPass() const { return mRenderPass; }
		inline GraphicsPipelineOptionalFields& getOptionalFields() { return *mOptionalFields; }
	};

	class GraphicsPipelineVulkan : public IGPUResourceVulkan {

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

		virtual ~GraphicsPipelineVulkan() override;
		virtual void close(VkDevice logicDevice) override;

		//Create Layout objects
		void createUniformObjects(VkDevice logicDevice);
		void createShaderUniforms(
			VkDevice logicDevice,
			VkPhysicalDevice physicalDevice,
			uint32_t imageCount,
			VkDescriptorPool descPool
		);
		void updateShaderUniforms(VkDevice logicDevice, uint32_t imageCount);
		void setFrameUniformData(VkDevice logicDevice, uint32_t image, uint32_t binding, void* data, size_t size);
		void recordDrawCommands(uint32_t imageIndex, VkCommandBuffer cmd, CurrentBindingsState& currentBindings);
		inline void addRenderableToDraw(std::shared_ptr<IRenderable> renderable) { mRenderableDrawList.emplace_back(renderable); }
		inline void clearRenderableToDraw() { mRenderableDrawList.clear(); }
		inline void bind(VkCommandBuffer cmd) const { vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, mGraphicsPipeline); }
		void recreate(VkDevice logicDevice, VkExtent2D extent, VkRenderPass renderPass);

		inline VkPipelineLayout getPipelineLayout() const { return mGraphicsPipelineLayout; }
		inline ShaderProgramVulkan& getShaderProgram() const { return mInstanceInfo.getShaderProgram(); }
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

	class PipelineRenderableConveyor {

	private:
		std::optional<std::reference_wrapper<GraphicsPipelineVulkan>> mPipeline;

	public:
		PipelineRenderableConveyor()
		:	mPipeline()
		{}
		PipelineRenderableConveyor(std::reference_wrapper<GraphicsPipelineVulkan> pipeline)
		:	mPipeline(pipeline)
		{}

		inline bool isValid() const { return mPipeline.has_value(); }
		inline void addRenderable(std::shared_ptr<IRenderable> renderable) { mPipeline->get().addRenderableToDraw(renderable); }
		inline void safeAddRenderable(std::shared_ptr<IRenderable> renderable) { if (isValid()) { mPipeline->get().addRenderableToDraw(renderable); }}
	};
}
