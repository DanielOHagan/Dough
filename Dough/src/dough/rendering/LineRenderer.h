#pragma once

#include "dough/Core.h"
#include "dough/rendering/batches/RenderBatchLine.h"
#include "dough/rendering/pipeline/GraphicsPipelineVulkan.h"
#include "dough/rendering/renderables/SimpleRenderable.h"
#include "dough/rendering/RenderPassVulkan.h"

namespace DOH {

	struct AppDebugInfo;
	class DescriptorSetsInstanceVulkan;

	//Primitives forward declarations
	class Quad;

	class LineRenderer {
	private:
		//App Scene
		static constexpr const EVertexType SCENE_LINE_VERTEX_TYPE = EVertexType::VERTEX_3D;
		std::unique_ptr<RenderBatchLineList> mSceneLineBatch;
		//std::unique_ptr<RenderBatchLineStrip> mSceneLineBatch;
		std::shared_ptr<ShaderProgram> mSceneLineShaderProgram;
		std::shared_ptr<ShaderVulkan> mSceneLineVertexShader;
		std::shared_ptr<ShaderVulkan> mSceneLineFragmentShader;
		std::unique_ptr<GraphicsPipelineInstanceInfo> mSceneLineGraphicsPipelineInfo;
		std::shared_ptr<GraphicsPipelineVulkan> mSceneLineGraphicsPipeline;
		std::shared_ptr<SimpleRenderable> mSceneLineRenderable;
		const char* mSceneLineRendererVertexShaderPath = "Dough/res/shaders/spv/LineRenderer3d.vert.spv";
		const char* mSceneLineRendererFragmentShaderPath = "Dough/res/shaders/spv/LineRenderer3d.frag.spv";
		//App UI
		static constexpr const EVertexType UI_LINE_VERTEX_TYPE = EVertexType::VERTEX_2D;
		std::unique_ptr<RenderBatchLineList> mUiLineBatch;
		//std::unique_ptr<RenderBatchLineStrip> mUiLineBatch;
		std::shared_ptr<ShaderProgram> mUiLineShaderProgram;
		std::shared_ptr<ShaderVulkan> mUiLineVertexShader;
		std::shared_ptr<ShaderVulkan> mUiLineFragmentShader;
		std::unique_ptr<GraphicsPipelineInstanceInfo> mUiLineGraphicsPipelineInfo;
		std::shared_ptr<GraphicsPipelineVulkan> mUiLineGraphicsPipeline;
		std::shared_ptr<SimpleRenderable> mUiLineRenderable;
		const char* mUiLineRendererVertexShaderPath = "Dough/res/shaders/spv/LineRenderer2d.vert.spv";
		const char* mUiLineRendererFragmentShaderPath = "Dough/res/shaders/spv/LineRenderer2d.frag.spv";

		std::unique_ptr<DescriptorSetsInstanceVulkan> mLineShadersDescriptorsInstance;

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

		//TODO:: rename to onSwapChain resize
		void recreateGraphicsPipelines(
			VkDevice logicDevice,
			VkExtent2D extent,
			const RenderPassVulkan& sceneRenderPass,
			const RenderPassVulkan& uiRenderPass,
			VkDescriptorPool descPool
		);

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
