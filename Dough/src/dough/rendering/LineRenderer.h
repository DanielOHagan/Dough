#pragma once

#include "dough/Core.h"
#include "dough/rendering/RenderBatchLine.h"
#include "dough/rendering/pipeline/GraphicsPipelineVulkan.h"
#include "dough/rendering/renderables/SimpleRenderable.h"
#include "dough/rendering/RenderPassVulkan.h"

namespace DOH {

	struct AppDebugInfo;

	//Primitives forward declarations
	class Quad;

	class LineRenderer {
	private:
		//App Scene
		static constexpr const EVertexType SCENE_LINE_VERTEX_TYPE = EVertexType::VERTEX_3D;
		std::unique_ptr<RenderBatchLineList> mSceneLineBatch;
		//std::unique_ptr<RenderBatchLineStrip> mSceneLineBatch;
		std::shared_ptr<ShaderProgramVulkan> mSceneLineShaderProgram;
		std::unique_ptr<GraphicsPipelineInstanceInfo> mSceneLineGraphicsPipelineInfo;
		std::shared_ptr<GraphicsPipelineVulkan> mSceneLineGraphicsPipeline;
		std::shared_ptr<SimpleRenderable> mSceneLineRenderable;
		const std::string mSceneLineRendererVertexShaderPath = "Dough/res/shaders/spv/LineRenderer3d.vert.spv";
		const std::string mSceneLineRendererFragmentShaderPath = "Dough/res/shaders/spv/LineRenderer3d.frag.spv";
		//App UI
		static constexpr const EVertexType UI_LINE_VERTEX_TYPE = EVertexType::VERTEX_2D;
		std::unique_ptr<RenderBatchLineList> mUiLineBatch;
		//std::unique_ptr<RenderBatchLineStrip> mUiLineBatch;
		std::shared_ptr<ShaderProgramVulkan> mUiLineShaderProgram;
		std::unique_ptr<GraphicsPipelineInstanceInfo> mUiLineGraphicsPipelineInfo;
		std::shared_ptr<GraphicsPipelineVulkan> mUiLineGraphicsPipeline;
		std::shared_ptr<SimpleRenderable> mUiLineRenderable;
		const std::string mUiLineRendererVertexShaderPath = "Dough/res/shaders/spv/LineRenderer2d.vert.spv";
		const std::string mUiLineRendererFragmentShaderPath = "Dough/res/shaders/spv/LineRenderer2d.frag.spv";

	public:
		LineRenderer() = default;
		LineRenderer(const LineRenderer& copy) = delete;
		LineRenderer operator=(const LineRenderer& assignment) = delete;

		void init(VkDevice logicDevice, VkExtent2D swapChainExtent, ValueUniformInfo uboSize);
		void close(VkDevice logicDevice);

		void drawScene(
			VkDevice logicDevice,
			const uint32_t imageIndex,
			void* ubo,
			size_t uboSize,
			VkCommandBuffer cmd,
			CurrentBindingsState& currentBindings,
			AppDebugInfo& debugInfo
		);
		void drawUi(
			VkDevice logicDevice,
			const uint32_t imageIndex,
			void* ubo,
			size_t uboSize,
			VkCommandBuffer cmd,
			CurrentBindingsState& currentBindings,
			AppDebugInfo& debugInfo
		);

		void recreateGraphicsPipelines(
			VkDevice logicDevice,
			VkExtent2D extent,
			const RenderPassVulkan& sceneRenderPass,
			const RenderPassVulkan& uiRenderPass,
			VkDescriptorPool descPool
		);
		void createShaderUniforms(VkDevice logicDevice, VkPhysicalDevice physicalDevice, const uint32_t imageCount, VkDescriptorPool descPool);
		void updateShaderUniforms(VkDevice logicDevice, const uint32_t imageCount);
		const std::vector<DescriptorTypeInfo> getDescriptorTypeInfo() const;

		inline PipelineRenderableConveyor getSceneLineConveyor() const { return { *mSceneLineGraphicsPipeline }; }
		inline PipelineRenderableConveyor getUiLineConveyor() const { return { *mUiLineGraphicsPipeline }; }
		inline uint32_t getSceneLineCount() const { return mSceneLineBatch->getLineCount(); }
		inline uint32_t getSceneMaxLineCount() const { return mSceneLineBatch->getMaxLineCount(); }
		inline uint32_t getUiLineCount() const { return mUiLineBatch->getLineCount(); }
		inline uint32_t getUiMaxLineCount() const { return mUiLineBatch->getMaxLineCount(); }


		void drawLineScene(const glm::vec3& start, const glm::vec3& end, const glm::vec4& colour);
		void drawLineUi(const glm::vec2& start, const glm::vec2& end, const glm::vec4& colour);

		void drawQuadScene(const Quad& quad, const glm::vec4& colour);
		void drawQuadUi(const Quad& quad, const glm::vec4& colour);
	};
}
