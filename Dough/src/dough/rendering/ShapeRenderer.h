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

	private:

		static std::unique_ptr<ShapeRenderer> INSTANCE;

		static const std::string QUAD_SHADER_PATH_VERT;
		static const std::string QUAD_SHADER_PATH_FRAG;

		RenderingContextVulkan& mContext;

		std::shared_ptr<ShaderProgramVulkan> mQuadShaderProgram;
		std::unique_ptr<GraphicsPipelineInstanceInfo> mQuadGraphicsPipelineInstanceInfo;
		std::shared_ptr<GraphicsPipelineVulkan> mQuadGraphicsPipeline;

		std::vector<RenderBatchQuad> mQuadRenderBatches;
		std::vector<std::shared_ptr<SimpleRenderable>> mRenderableQuadBatches; // mQuadBatchRenderables;
		//Quad indices buffer to be shared between Quad VAOs
		std::shared_ptr<IndexBufferVulkan> mQuadSharedIndexBuffer;
		std::unique_ptr<TextureArray> mQuadBatchTextureArray;

		//Textures
		//TODO:: move this to context
		std::shared_ptr<TextureVulkan> mWhiteTexture;

		//TEMP:: Rebinding descriptors not available, all textures must be bound first. Needs fixing!
		const char* testTexturesPath = "Dough/res/images/test textures/";
		std::vector<std::shared_ptr<TextureVulkan>> mTestTextures;
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
		void createPipelineUniformObjectsImpl(VkDescriptorPool descPool);

		void setUniformDataImpl(uint32_t currentImage, uint32_t uboBinding, glm::mat4x4& sceneProjView, glm::mat4x4& uiProjView);
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
		static void createPipelineUniformObjects(VkDescriptorPool descPool);

		static void setUniformData(uint32_t currentImage, uint32_t uboBinding, glm::mat4x4& sceneProjView, glm::mat4x4& uiProjView);
		static void drawScene(uint32_t imageIndex, VkCommandBuffer cmd, CurrentBindingsState& currentBindings);
		static void drawUi(uint32_t imageIndex, VkCommandBuffer cmd, CurrentBindingsState& currentBindings);

		//Close the dynamic batches that have a geo count of 0
		static size_t createNewBatchQuad();
		//static size_t createNewBatchCircle();
		//static size_t createNewBatchTriangle();
		static void closeEmptyQuadBatches();
		//static void closeEmptyCircleBatches();
		//static void closeEmptyTriangleBatches();


		static size_t getQuadBatchCount();
		static uint32_t getDrawnQuadCount();
		static uint32_t getTruncatedQuadCount();
		static void resetLocalDebugInfo();

		static std::vector<DescriptorTypeInfo> getDescriptorTypeInfos();

		//-----Shape Objects-----
		static void drawQuadScene(const Quad& quad);
		static void drawQuadTexturedScene(const Quad& quad);
		static void drawQuadArrayScene(const std::vector<Quad>& quadArr);
		static void drawQuadArrayTexturedScene(const std::vector<Quad>& quadArr);
		static void drawQuadArraySameTextureScene(const std::vector<Quad>& quadArr);
		//static void drawQuadUi(Quad& quad);
		//static void drawQuadTexturedUi(const Quad& quad);
		//static void drawQuadArrayUi(const std::vector<Quad>& quadArr);
		//static void drawQuadArrayTexturedUi(const std::vector<Quad>& quadArr);
		//static void drawQuadArraySameTextureUi(const std::vector<Quad>& quadArr);


		//TEMP:: 
		static inline TextureVulkan& getWhiteTexture() { return *INSTANCE->mWhiteTexture; }
		static inline std::shared_ptr<IndexBufferVulkan> getQuadSharedIndexBufferPtr() { return INSTANCE->mQuadSharedIndexBuffer; }
		static inline TextureArray& getQuadBatchTextureArray() { return *INSTANCE->mQuadBatchTextureArray; }
		static inline std::vector<RenderBatchQuad>& getQuadRenderBatches() { return INSTANCE->mQuadRenderBatches; }
		static inline const std::vector<std::shared_ptr<TextureVulkan>>& getTestTextures() { return INSTANCE->mTestTextures; }
		static inline const std::shared_ptr<MonoSpaceTextureAtlas> getTestMonoSpaceTextureAtlas() { return INSTANCE->mTestMonoSpaceTextureAtlas; }
		static inline const std::shared_ptr<IndexedTextureAtlas> getTestIndexedTextureAtlas() { return INSTANCE->mTestIndexedTextureAtlas; }
	};
}
