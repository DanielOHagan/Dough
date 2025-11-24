#pragma once

#include "dough/Utils.h"
#include "dough/rendering/SwapChainVulkan.h"
#include "dough/scene/geometry/collections/TextString.h"
#include "dough/rendering/pipeline/GraphicsPipelineVulkan.h"
#include "dough/rendering/batches/RenderBatchQuad.h"
#include "dough/rendering/batches/RenderBatchCircle.h"
#include "dough/rendering/renderables/SimpleRenderable.h"
#include "dough/rendering/textures/TextureArray.h"
#include "dough/rendering/textures/TextureAtlas.h"
#include "dough/rendering/ShapeRenderingObjects.h"

#include <vulkan/vulkan_core.h>

namespace DOH {

	//TODO:: create a DynamicBatchArray<RenderBatchQuad> or something like that.

	class RenderingContextVulkan;
	enum class EImGuiContainerType;

	class ShapeRenderer {
		friend class RenderingContextVulkan;

	private:
		static std::unique_ptr<ShapeRenderer> INSTANCE;

		static const char* QUAD_SHADER_PATH_VERT;
		static const char* QUAD_SHADER_PATH_FRAG;
		static const char* CIRCLE_SHADER_PATH_VERT;
		static const char* CIRCLE_SHADER_PATH_FRAG;

		static const char* NAME_SHORT_HAND;
		static const char* NAME_LONG_HAND;

		static const uint32_t CAMERA_UBO_SLOT = 0u;

		RenderingContextVulkan& mContext;

		ShapeRenderingObjects<RenderBatchQuad> mQuadScene;
		ShapeRenderingObjects<RenderBatchQuad> mQuadUi;
		ShapeRenderingObjects<RenderBatchCircle> mCircleScene;
		ShapeRenderingObjects<RenderBatchCircle> mCircleUi;

		std::shared_ptr<CameraGpuData> mSceneCameraData;
		std::shared_ptr<CameraGpuData> mUiCameraData;

		std::shared_ptr<IndexBufferVulkan> mQuadSharedIndexBuffer;
		std::unique_ptr<TextureArray> mTextureArray;
		VkDescriptorSet mTextureArrayDescSet;
		std::shared_ptr<ShaderDescriptorSetLayoutsVulkan> mShapeDescSetLayouts;
		//std::shared_ptr<DescriptorSetsInstanceVulkan> mShapesDescSetsInstance;
		std::shared_ptr<DescriptorSetsInstanceVulkan> mShapesDescSetsInstanceScene;
		std::shared_ptr<DescriptorSetsInstanceVulkan> mShapesDescSetsInstanceUi;

		//TEMP:: Leftover from when rebinding descriptors was not available. Need to move this into demo code.
		const char* testTexturesPath = "Dough/Dough/res/images/test textures/";
		//std::vector<std::shared_ptr<TextureVulkan>> mTestTextures;
		std::shared_ptr<MonoSpaceTextureAtlas> mTestMonoSpaceTextureAtlas;
		std::shared_ptr<IndexedTextureAtlas> mTestIndexedTextureAtlas;

		//TODO:: A better way of doing no rendering with ShapeRenderer would be to "unload" and "load" when needed,
		// including just removing it from the "render order" as needed.
		// 
		//When drawScene/Ui is called, if their respective camera data is null then the function is ended.
		//By default this prints a warning but by setting these values to false that warning is prevented.
		bool mWarnOnNullSceneCameraData;
		bool mWarnOnNullUiCameraData;

		//-----Debug information-----
		uint32_t mDrawnQuadCount;
		uint32_t mTruncatedQuadCount;
		uint32_t mDrawnCircleCount;
		uint32_t mTruncatedCircleCount;
		//uint32_t mDrawnTriangleCount;
		//uint32_t mTruncatedTriangleCount;

	private:
		void initImpl();
		void initQuad();
		void initCircle();
		//void initTriangle();
		void closeImpl();
		void onSwapChainResizeImpl(SwapChainVulkan& swapChain);

		void updateTextureArrayDescriptorSetImpl();

		void drawSceneImpl(uint32_t imageIndex, VkCommandBuffer cmd, CurrentBindingsState& currentBindings);
		void drawUiImpl(uint32_t imageIndex, VkCommandBuffer cmd, CurrentBindingsState& currentBindings);

		size_t createNewBatchQuad(ShapeRenderingObjects<RenderBatchQuad>& shapeRendering);
		size_t createNewBatchCircle(ShapeRenderingObjects<RenderBatchCircle>& shapeRendering);
		//size_t createNewBatchTriangle();

		void closeEmptyQuadBatchesImpl();
		void closeEmptyCircleBatchesImpl();
		//void closeEmptyTriangleBatchesImpl();

		//Quad
		void drawQuad(ShapeRenderingObjects<RenderBatchQuad>& quadGroup, const Quad& quad);
		void drawQuadTextured(ShapeRenderingObjects<RenderBatchQuad>& quadGroup, const Quad& quad);
		void drawQuadArray(ShapeRenderingObjects<RenderBatchQuad>& quadGroup, const std::vector<Quad>& quadArr);
		void drawQuadArrayTextured(ShapeRenderingObjects<RenderBatchQuad>& quadGroup, const std::vector<Quad>& quadArr);
		void drawQuadArraySameTexture(ShapeRenderingObjects<RenderBatchQuad>& quadGroup, const std::vector<Quad>& quadArr);

		//Circle
		void drawCircle(ShapeRenderingObjects<RenderBatchCircle>& circleGroup, const Circle& circle);
		void drawCircleTextured(ShapeRenderingObjects<RenderBatchCircle>& circleGroup, const Circle& circle);
		void drawCircleArray(ShapeRenderingObjects<RenderBatchCircle>& circleGroup, const std::vector<Circle>& circleArr);
		void drawCircleArrayTextured(ShapeRenderingObjects<RenderBatchCircle>& circleGroup, const std::vector<Circle>& circleArr);
		void drawCircleArraySameTexture(ShapeRenderingObjects<RenderBatchCircle>& circleGroup, const std::vector<Circle>& circleArr);

		void setSceneCameraDataImpl(std::shared_ptr<CameraGpuData> cameraData);
		void setUiCameraDataImpl(std::shared_ptr<CameraGpuData> cameraData);

		void drawImGuiImpl(EImGuiContainerType type);

	public:
		ShapeRenderer(RenderingContextVulkan& context);

		static void init(RenderingContextVulkan& context);
		static void close();
		static void onSwapChainResize(SwapChainVulkan& swapChain);

		//TEMP:: Updates mTextureArrayDescSet to point to textures currently in mTextureArray.
		//TODO:: Rework this system to allow for more textures and not rely on the app logic to call this function.
		static void updateTextureArrayDescriptorSet();

		static inline void drawScene(uint32_t imageIndex, VkCommandBuffer cmd, CurrentBindingsState& currentBindings) { INSTANCE->drawSceneImpl(imageIndex, cmd, currentBindings); }
		static inline void drawUi(uint32_t imageIndex, VkCommandBuffer cmd, CurrentBindingsState& currentBindings) { INSTANCE->drawUiImpl(imageIndex, cmd, currentBindings); }

		static void closeEmptyQuadBatches();
		static void closeEmptyCircleBatches();
		//static void closeEmptyTriangleBatches();

		static inline uint32_t getQuadBatchCount() { return INSTANCE->mQuadScene.getBatchCount() + INSTANCE->mQuadUi.getBatchCount(); }
		static inline uint32_t getDrawnQuadCount() { return INSTANCE->mDrawnQuadCount; }
		static inline uint32_t getTruncatedQuadCount() { return INSTANCE->mTruncatedQuadCount; }
		static void resetLocalDebugInfo();

		static std::vector<DescriptorTypeInfo> getEngineDescriptorTypeInfos();

		//-----Shape Objects-----
		//Quad
		static inline void drawQuadScene(const Quad& quad) { INSTANCE->drawQuad(INSTANCE->mQuadScene, quad); }
		static inline void drawQuadTexturedScene(const Quad& quad) { INSTANCE->drawQuadTextured(INSTANCE->mQuadScene, quad); }
		static inline void drawQuadArrayScene(const std::vector<Quad>& quadArr) { INSTANCE->drawQuadArray(INSTANCE->mQuadScene, quadArr); }
		static inline void drawQuadArrayTexturedScene(const std::vector<Quad>& quadArr) { INSTANCE->drawQuadArrayTextured(INSTANCE->mQuadScene, quadArr); }
		static inline void drawQuadArraySameTextureScene(const std::vector<Quad>& quadArr) { INSTANCE->drawQuadArraySameTexture(INSTANCE->mQuadScene, quadArr); }
		static inline void drawQuadUi(Quad& quad) { INSTANCE->drawQuad(INSTANCE->mQuadUi, quad); }
		static inline void drawQuadTexturedUi(const Quad& quad) { INSTANCE->drawQuadTextured(INSTANCE->mQuadUi, quad); }
		static inline void drawQuadArrayUi(const std::vector<Quad>& quadArr) { INSTANCE->drawQuadArray(INSTANCE->mQuadUi, quadArr); }
		static inline void drawQuadArrayTexturedUi(const std::vector<Quad>& quadArr) { INSTANCE->drawQuadArrayTextured(INSTANCE->mQuadUi, quadArr); }
		static inline void drawQuadArraySameTextureUi(const std::vector<Quad>& quadArr) { INSTANCE->drawQuadArraySameTexture(INSTANCE->mQuadUi, quadArr); }
		//Circle
		static inline void drawCircleScene(const Circle& circle) { INSTANCE->drawCircle(INSTANCE->mCircleScene, circle); }
		static inline void drawCircleTexturedScene(const Circle& circle) { INSTANCE->drawCircleTextured(INSTANCE->mCircleScene, circle); }
		static inline void drawCircleArrayScene(const std::vector<Circle>& circleArr) { INSTANCE->drawCircleArray(INSTANCE->mCircleScene, circleArr); }
		static inline void drawCircleArrayTexturedScene(const std::vector<Circle>& circleArr) { INSTANCE->drawCircleArrayTextured(INSTANCE->mCircleScene, circleArr); }
		static inline void drawCircleArraySameTextureScene(const std::vector<Circle>& circleArr) { INSTANCE->drawCircleArraySameTexture(INSTANCE->mCircleScene, circleArr); }
		static inline void drawCircleUi(const Circle& circle) { INSTANCE->drawCircle(INSTANCE->mCircleUi, circle); }
		static inline void drawCircleTexturedUi(const Circle& circle) { INSTANCE->drawCircleTextured(INSTANCE->mCircleUi, circle); }
		static inline void drawCircleArrayUi(const std::vector<Circle>& circleArr) { INSTANCE->drawCircleArray(INSTANCE->mCircleUi, circleArr); }
		static inline void drawCircleArrayTexturedUi(const std::vector<Circle>& circleArr) { INSTANCE->drawCircleArrayTextured(INSTANCE->mCircleUi, circleArr); }
		static inline void drawCircleArraySameTextureUi(const std::vector<Circle>& circleArr) { INSTANCE->drawCircleArraySameTexture(INSTANCE->mCircleUi, circleArr); }

		static inline std::shared_ptr<IndexBufferVulkan> getQuadSharedIndexBufferPtr() { return INSTANCE->mQuadSharedIndexBuffer; }
		static inline TextureArray& getShapesTextureArray() { return *INSTANCE->mTextureArray; }
		static inline std::vector<std::shared_ptr<RenderBatchQuad>>& getQuadSceneRenderBatches() { return INSTANCE->mQuadScene.GeoBatches; }
		static inline std::vector<std::shared_ptr<RenderBatchQuad>>& getQuadUiRenderBatches() { return INSTANCE->mQuadUi.GeoBatches; }
		static inline uint32_t getQuadSceneBatchCount() { return INSTANCE->mQuadScene.getBatchCount(); }
		static inline uint32_t getQuadUiBatchCount() { return INSTANCE->mQuadUi.getBatchCount(); }
		static inline uint32_t getCircleSceneBatchCount() { return INSTANCE->mCircleScene.getBatchCount(); }

		//static inline const std::vector<std::shared_ptr<TextureVulkan>>& getTestTextures() { return INSTANCE->mTestTextures; }
		static inline const std::shared_ptr<MonoSpaceTextureAtlas> getTestMonoSpaceTextureAtlas() { return INSTANCE->mTestMonoSpaceTextureAtlas; }
		static inline const std::shared_ptr<IndexedTextureAtlas> getTestIndexedTextureAtlas() { return INSTANCE->mTestIndexedTextureAtlas; }
		static inline const std::vector<std::shared_ptr<RenderBatchCircle>>& getCircleSceneBatches() { return INSTANCE->mCircleScene.GeoBatches; }
		static inline const std::vector<std::shared_ptr<RenderBatchCircle>>& getCircleUiBatches() { return INSTANCE->mCircleUi.GeoBatches; }
		static inline uint32_t getCircleUiBatchCount() { return INSTANCE->mCircleUi.getBatchCount(); }

		static inline void setSceneCameraData(std::shared_ptr<CameraGpuData> cameraData) { INSTANCE->setSceneCameraDataImpl(cameraData); }
		static inline void setUiCameraData(std::shared_ptr<CameraGpuData> cameraData) { INSTANCE->setUiCameraDataImpl(cameraData); }
		static inline void setWarnOnNullSceneCameraData(bool enabled) { INSTANCE->mWarnOnNullSceneCameraData = enabled; }
		static inline void setWarnOnNullUiCameraData(bool enabled) { INSTANCE->mWarnOnNullUiCameraData = enabled; }

		//Draw ImGui elements.
		static inline void drawImGui(EImGuiContainerType type) { INSTANCE->drawImGuiImpl(type); }
	};
}
