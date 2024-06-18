#pragma once

#include "dough/Utils.h"
#include "dough/rendering/SwapChainVulkan.h"
#include "dough/scene/geometry/collections/TextString.h"
#include "dough/rendering/pipeline/GraphicsPipelineVulkan.h"
#include "dough/rendering/batches/RenderBatchQuad.h"
#include "dough/rendering/renderables/SimpleRenderable.h"
#include "dough/rendering/textures/TextureArray.h"
#include "dough/rendering/textures/TextureAtlas.h"
#include "dough/rendering/SwapChainVulkan.h"

#include <vulkan/vulkan_core.h>

namespace DOH {

	class RenderingContextVulkan;

	class ShapeRenderer {

		friend class RenderingContextVulkan;

	private:
		static std::unique_ptr<ShapeRenderer> INSTANCE;

		static const char* QUAD_SHADER_PATH_VERT;
		static const char* QUAD_SHADER_PATH_FRAG;

		RenderingContextVulkan& mContext;

		std::shared_ptr<ShaderProgram> mQuadShaderProgram;
		std::shared_ptr<ShaderVulkan> mQuadVertexShader;
		std::shared_ptr<ShaderVulkan> mQuadFragmentShader;
		std::unique_ptr<GraphicsPipelineInstanceInfo> mQuadGraphicsPipelineInstanceInfo;
		std::shared_ptr<GraphicsPipelineVulkan> mQuadGraphicsPipeline;

		std::vector<RenderBatchQuad> mQuadRenderBatches;
		std::vector<std::shared_ptr<SimpleRenderable>> mRenderableQuadBatches; // mQuadBatchRenderables;
		//Quad indices buffer to be shared between Quad VAOs
		std::shared_ptr<IndexBufferVulkan> mQuadSharedIndexBuffer;
		std::unique_ptr<TextureArray> mQuadBatchTextureArray;
		VkDescriptorSet mQuadBatchTextureArrayDescSet;
		std::shared_ptr<DescriptorSetsInstanceVulkan> mSceneBatchDescriptorSetInstance;

		//TEMP:: Leftover from when rebinding descriptors was not available. Need to move this into demo code.
		const char* testTexturesPath = "Dough/res/images/test textures/";
		//std::vector<std::shared_ptr<TextureVulkan>> mTestTextures;
		std::shared_ptr<MonoSpaceTextureAtlas> mTestMonoSpaceTextureAtlas;
		std::shared_ptr<IndexedTextureAtlas> mTestIndexedTextureAtlas;

		//-----Debug information-----
		uint32_t mDrawnQuadCount;
		uint32_t mTruncatedQuadCount;
		//uint32_t mDrawnCircleCount;
		//uint32_t mTruncatedCircleCount;
		//uint32_t mDrawnTriangleCount;
		//uint32_t mTruncatedTriangleCount;

	private:
		void initImpl();
		void initQuadImpl();
		//void initCircleImpl();
		//void initTriangleImpl();
		void closeImpl();
		void onSwapChainResizeImpl(SwapChainVulkan& swapChain);

		void drawSceneImpl(uint32_t imageIndex, VkCommandBuffer cmd, CurrentBindingsState& currentBindings);
		void drawUiImpl(uint32_t imageIndex, VkCommandBuffer cmd, CurrentBindingsState& currentBindings);

		size_t createNewBatchQuadImpl();
		//size_t createNewBatchCircleImpl();
		//size_t createNewBatchTriangleImpl();
		void closeEmptyQuadBatchesImpl();
		//void closeEmptyCircleBatchesImpl();
		//void closeEmptyTriangleBatchesImpl();

		void drawQuadSceneImpl(const Quad& quad);
		void drawQuadTexturedSceneImpl(const Quad& quad);
		void drawQuadArraySceneImpl(const std::vector<Quad>& quadArr);
		void drawQuadArrayTexturedSceneImpl(const std::vector<Quad>& quadArr);
		void drawQuadArraySameTextureSceneImpl(const std::vector<Quad>& quadArr);
		//void drawQuadUiImpl(Quad& quad);
		//void drawQuadTexturedUiImpl(const Quad& quad);
		//void drawQuadArrayUiImpl(const std::vector<Quad>& quadArr);
		//void drawQuadArrayTexturedUiImpl(const std::vector<Quad>& quadArr);
		//void drawQuadArraySameTextureUiImpl(const std::vector<Quad>& quadArr);

	public:
		ShapeRenderer(RenderingContextVulkan& context);

		static void init(RenderingContextVulkan& context);
		static void initQuad();
		//TODO::
		//static void initCircle();
		//static void initTriangle();
		static void close();
		static void onSwapChainResize(SwapChainVulkan& swapChain);

		static inline void drawScene(uint32_t imageIndex, VkCommandBuffer cmd, CurrentBindingsState& currentBindings) { INSTANCE->drawSceneImpl(imageIndex, cmd, currentBindings); }
		static inline void drawUi(uint32_t imageIndex, VkCommandBuffer cmd, CurrentBindingsState& currentBindings) { INSTANCE->drawUiImpl(imageIndex, cmd, currentBindings); }

		//Close the dynamic batches that have a geo count of 0
		static size_t createNewBatchQuad();
		//static size_t createNewBatchCircle();
		//static size_t createNewBatchTriangle();
		static void closeEmptyQuadBatches();
		//static void closeEmptyCircleBatches();
		//static void closeEmptyTriangleBatches();


		static inline size_t getQuadBatchCount() { return INSTANCE->mQuadRenderBatches.size(); }
		static inline uint32_t getDrawnQuadCount() { return INSTANCE->mDrawnQuadCount; }
		static inline uint32_t getTruncatedQuadCount() { return INSTANCE->mTruncatedQuadCount; }
		static void resetLocalDebugInfo();

		static std::vector<DescriptorTypeInfo> getEngineDescriptorTypeInfos();

		//-----Shape Objects-----
		static inline void drawQuadScene(const Quad& quad) { INSTANCE->drawQuadSceneImpl(quad); }
		static inline void drawQuadTexturedScene(const Quad& quad) { INSTANCE->drawQuadTexturedSceneImpl(quad); }
		static inline void drawQuadArrayScene(const std::vector<Quad>& quadArr) { INSTANCE->drawQuadArraySceneImpl(quadArr); }
		static inline void drawQuadArrayTexturedScene(const std::vector<Quad>& quadArr) { INSTANCE->drawQuadArrayTexturedSceneImpl(quadArr); }
		static inline void drawQuadArraySameTextureScene(const std::vector<Quad>& quadArr) { INSTANCE->drawQuadArraySameTextureSceneImpl(quadArr); }
		//static inline void drawQuadUi(Quad& quad);
		//static inline void drawQuadTexturedUi(const Quad& quad);
		//static inline void drawQuadArrayUi(const std::vector<Quad>& quadArr);
		//static inline void drawQuadArrayTexturedUi(const std::vector<Quad>& quadArr);
		//static inline void drawQuadArraySameTextureUi(const std::vector<Quad>& quadArr);

		static inline std::shared_ptr<IndexBufferVulkan> getQuadSharedIndexBufferPtr() { return INSTANCE->mQuadSharedIndexBuffer; }

		//TEMP:: 
		static inline TextureArray& getQuadBatchTextureArray() { return *INSTANCE->mQuadBatchTextureArray; }
		static inline std::vector<RenderBatchQuad>& getQuadRenderBatches() { return INSTANCE->mQuadRenderBatches; }
		//static inline const std::vector<std::shared_ptr<TextureVulkan>>& getTestTextures() { return INSTANCE->mTestTextures; }
		static inline const std::shared_ptr<MonoSpaceTextureAtlas> getTestMonoSpaceTextureAtlas() { return INSTANCE->mTestMonoSpaceTextureAtlas; }
		static inline const std::shared_ptr<IndexedTextureAtlas> getTestIndexedTextureAtlas() { return INSTANCE->mTestIndexedTextureAtlas; }
	};
}
