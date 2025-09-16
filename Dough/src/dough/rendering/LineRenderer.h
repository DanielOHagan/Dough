#pragma once

#include "dough/Core.h"
#include "dough/rendering/batches/RenderBatchLine.h"
#include "dough/rendering/pipeline/GraphicsPipelineVulkan.h"
#include "dough/rendering/renderables/SimpleRenderable.h"
#include "dough/rendering/RenderPassVulkan.h"

namespace DOH {

	struct AppDebugInfo;
	class DescriptorSetsInstanceVulkan;
	class RenderingContextVulkan;
	class Quad;
	class SwapChainVulkan;
	class CameraGpuData;

	class LineRenderer {

		friend class RenderingContextVulkan;

	private:
		static std::unique_ptr<LineRenderer> INSTANCE;

		RenderingContextVulkan& mContext;

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
		static const char* SCENE_LINE_SHADER_PATH_VERT;
		static const char* SCENE_LINE_SHADER_PATH_FRAG;
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
		static const char* UI_LINE_SHADER_PATH_VERT;
		static const char* UI_LINE_SHADER_PATH_FRAG;

		std::unique_ptr<DescriptorSetsInstanceVulkan> mLineShadersDescriptorsInstance;

		std::shared_ptr<CameraGpuData> mSceneCameraData;
		std::shared_ptr<CameraGpuData> mUiCameraData;

	private:
		void initImpl();
		void closeImpl();
		void onSwapChainResizeImpl(SwapChainVulkan& swapChain);

		void drawSceneImpl(uint32_t imageIndex, VkCommandBuffer cmd, CurrentBindingsState& currentBindings);
		void drawUiImpl(uint32_t imageIndex, VkCommandBuffer cmd, CurrentBindingsState& currentBindings);

		void drawLineSceneImpl(const glm::vec3& start, const glm::vec3& end, const glm::vec4& colour);
		void drawLineUiImpl(const glm::vec2& start, const glm::vec2& end, const glm::vec4& colour);

		void drawQuadSceneImpl(const Quad& quad, const glm::vec4& colour);
		void drawQuadUiImpl(const Quad& quad, const glm::vec4& colour);

		static void drawScene(uint32_t imageIndex, VkCommandBuffer cmd, CurrentBindingsState& currentBindings);
		static void drawUi(uint32_t imageIndex, VkCommandBuffer cmd, CurrentBindingsState& currentBindings);

		static void init(RenderingContextVulkan& context);
		static void close();
		static void onSwapChainResize(SwapChainVulkan& swapChain);

	public:
		LineRenderer(RenderingContextVulkan& context);

		static inline void drawLineScene(const glm::vec3& start, const glm::vec3& end, const glm::vec4& colour) { INSTANCE->drawLineSceneImpl(start, end, colour); }
		static inline void drawLineUi(const glm::vec2 & start, const glm::vec2 & end, const glm::vec4 & colour) { INSTANCE->drawLineUiImpl(start, end, colour); }
		
		static inline void drawQuadScene(const Quad& quad, const glm::vec4& colour) { INSTANCE->drawQuadSceneImpl(quad, colour); }
		static inline void drawQuadUi(const Quad& quad, const glm::vec4& colour) { INSTANCE->drawQuadUiImpl(quad, colour); }

		static inline uint32_t getSceneLineCount() { return INSTANCE->mSceneLineBatch->getLineCount(); }
		static inline uint32_t getSceneMaxLineCount() { return INSTANCE->mSceneLineBatch->getMaxLineCount(); }
		static inline uint32_t getUiLineCount() { return INSTANCE->mUiLineBatch->getLineCount(); }
		static inline uint32_t getUiMaxLineCount() { return INSTANCE->mUiLineBatch->getMaxLineCount(); }

		static inline void setSceneCameraData(std::shared_ptr<CameraGpuData> cameraData) { INSTANCE->mSceneCameraData = cameraData; }
		static inline void setUiCameraData(std::shared_ptr<CameraGpuData> cameraData) { INSTANCE->mUiCameraData = cameraData; }
	};
}
