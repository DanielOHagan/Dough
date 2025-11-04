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
	enum class EImGuiContainerType;
	
	//NOTE:: Currently only allows for one batch per LineRenderingObjects instance.
	class LineRenderingObjects : public IGPUResourceOwnerVulkan {
	public:
		std::shared_ptr<ShaderProgram> ShaderProgram;
		std::shared_ptr<ShaderVulkan> VertexShader;
		std::shared_ptr<ShaderVulkan> FragmentShader;
		std::shared_ptr<GraphicsPipelineVulkan> GraphicsPipeline;
		std::shared_ptr<SimpleRenderable> Renderable;
		std::unique_ptr<GraphicsPipelineInstanceInfo> GraphicsPipelineInfo;
		std::unique_ptr<RenderBatchLineList> Batch; //TODO:: Allow for RenderBatchLineStrip

		virtual void addOwnedResourcesToClose(RenderingContextVulkan& context) override;
	};

	class LineRenderer {

		friend class RenderingContextVulkan;

	private:
		static std::unique_ptr<LineRenderer> INSTANCE;

		RenderingContextVulkan& mContext;

		static constexpr const EVertexType SCENE_LINE_VERTEX_TYPE = EVertexType::VERTEX_3D;
		static constexpr const EVertexType UI_LINE_VERTEX_TYPE = EVertexType::VERTEX_2D;
		static const char* SCENE_LINE_SHADER_PATH_VERT;
		static const char* SCENE_LINE_SHADER_PATH_FRAG;
		static const char* UI_LINE_SHADER_PATH_VERT;
		static const char* UI_LINE_SHADER_PATH_FRAG;

		static const char* NAME_SHORT_HAND;
		static const char* NAME_LONG_HAND;

		static const uint32_t CAMERA_UBO_SLOT = 0u;

		std::unique_ptr<LineRenderingObjects> mSceneLineList;
		std::unique_ptr<LineRenderingObjects> mUiLineList;
		std::unique_ptr<DescriptorSetsInstanceVulkan> mLineShadersDescriptorsInstance;

		std::shared_ptr<CameraGpuData> mSceneCameraData;
		std::shared_ptr<CameraGpuData> mUiCameraData;

		//TODO:: A better way of doing no rendering with LineRenderer would be to "unload" and "load" when needed,
		// including just removing it from the "render order" as needed.
		// 
		//When drawScene/Ui is called, if their respective camera data is null then the function is ended.
		//By default this prints a warning but by setting these values to false that warning is prevented.
		bool mWarnOnNullSceneCameraData;
		bool mWarnOnNullUiCameraData;

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

		void setSceneCameraDataImpl(std::shared_ptr<CameraGpuData> cameraData);
		void setUiCameraDataImpl(std::shared_ptr<CameraGpuData> cameraData);

		void drawImGuiImpl(EImGuiContainerType type);

		static void drawScene(uint32_t imageIndex, VkCommandBuffer cmd, CurrentBindingsState& currentBindings);
		static void drawUi(uint32_t imageIndex, VkCommandBuffer cmd, CurrentBindingsState& currentBindings);

		static void init(RenderingContextVulkan& context);
		static void close();
		static void onSwapChainResize(SwapChainVulkan& swapChain);

	public:
		LineRenderer(RenderingContextVulkan& context);
		LineRenderer(const LineRenderer& copy) = delete;
		void operator=(const LineRenderer& assignment) = delete;

		static inline void drawLineScene(const glm::vec3& start, const glm::vec3& end, const glm::vec4& colour) { INSTANCE->drawLineSceneImpl(start, end, colour); }
		static inline void drawLineUi(const glm::vec2 & start, const glm::vec2 & end, const glm::vec4 & colour) { INSTANCE->drawLineUiImpl(start, end, colour); }
		
		static inline void drawQuadScene(const Quad& quad, const glm::vec4& colour) { INSTANCE->drawQuadSceneImpl(quad, colour); }
		static inline void drawQuadUi(const Quad& quad, const glm::vec4& colour) { INSTANCE->drawQuadUiImpl(quad, colour); }

		static inline uint32_t getSceneLineCount() { return INSTANCE->mSceneLineList->Batch->getLineCount(); }
		static inline uint32_t getSceneMaxLineCount() { return INSTANCE->mSceneLineList->Batch->getMaxLineCount(); }
		static inline uint32_t getUiLineCount() { return INSTANCE->mUiLineList->Batch->getLineCount(); }
		static inline uint32_t getUiMaxLineCount() { return INSTANCE->mUiLineList->Batch->getMaxLineCount(); }

		static inline void setSceneCameraData(std::shared_ptr<CameraGpuData> cameraData) { INSTANCE->setSceneCameraDataImpl(cameraData); }
		static inline void setUiCameraData(std::shared_ptr<CameraGpuData> cameraData) { INSTANCE->setUiCameraDataImpl(cameraData); }
		//Set to true if you want warnings to display each frame the scene camera data is null.
		static inline void setWarnOnNullSceneCameraData(bool enabled) { INSTANCE->mWarnOnNullSceneCameraData = enabled; }
		//Set to true if you want warnings to display each frame the ui camera data is null.
		static inline void setWarnOnNullUiCameraData(bool enabled) { INSTANCE->mWarnOnNullUiCameraData = enabled; }

		static inline void drawImGui(EImGuiContainerType type) { INSTANCE->drawImGuiImpl(type); }
	};
}
