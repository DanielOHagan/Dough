#pragma once

#include "dough/rendering/pipeline/GraphicsPipelineVulkan.h"
#include "dough/rendering/renderer2d/RenderBatchQuad.h"
#include "dough/rendering/renderables/SimpleRenderable.h"
#include "dough/rendering/textures/TextureArray.h"
#include "dough/rendering/textures/TextureAtlasVulkan.h"

//Currently batching has significantly different performance on different configurations
#define MAX_BATCH_QUAD_COUNT
#if defined (_DEBUG)
	#undef MAX_BATCH_QUAD_COUNT
	#define MAX_BATCH_QUAD_COUNT 4
#else
	#undef MAX_BATCH_QUAD_COUNT
	#define MAX_BATCH_QUAD_COUNT 10
#endif

namespace DOH {

	class RenderingContextVulkan;

	class Renderer2dStorageVulkan {

	private:
		RenderingContextVulkan& mContext;

		static const std::string QUAD_SHADER_PATH_VERT;
		static const std::string QUAD_SHADER_PATH_FRAG;

		VkDescriptorPool mDescriptorPool;

		std::shared_ptr<ShaderProgramVulkan> mQuadShaderProgram;
		std::unique_ptr<GraphicsPipelineInstanceInfo> mQuadGraphicsPipelineInstanceInfo;
		std::shared_ptr<GraphicsPipelineVulkan> mQuadGraphicsPipeline;

		std::vector<RenderBatchQuad> mQuadRenderBatches;
		std::vector<std::shared_ptr<SimpleRenderable>> mRenderableQuadBatches;
		//Quad indices buffer to be shared between Quad VAOs
		std::shared_ptr<IndexBufferVulkan> mQuadSharedIndexBuffer;
		std::unique_ptr<TextureArray> mQuadBatchTextureArray;

		//Textures
		std::shared_ptr<TextureVulkan> mWhiteTexture;

		//TEMP::
		const char* testTexturesPath = "res/images/test textures/";
		std::vector<std::shared_ptr<TextureVulkan>> mTestTextures;
		std::shared_ptr<MonoSpaceTextureAtlasVulkan> mTestTexturesAtlas;

		void initForQuads(VkDevice logicDevice);

	public:
		//Max count of objects allowed per batch
		
		//BATCH_QUAD_COUNT is given an arbitrary number, it can be higher or lower depending on need.
		//Different batches may even use different sizes and not stick to this pre-determined limit,
		//which isn't even enforced by the engine.
		//Maybe, for optimisation, the renderer might grant a higher limit to scene batches than UI batches.
		enum BatchSizeLimits {
			MAX_BATCH_COUNT_QUAD = MAX_BATCH_QUAD_COUNT,

			SINGLE_QUAD_VERTEX_COUNT = 4,
			SINGLE_QUAD_INDEX_COUNT = 6,
			SINGLE_QUAD_COMPONENT_COUNT = Vertex3dTextured::COMPONENT_COUNT * SINGLE_QUAD_VERTEX_COUNT,
			SINGLE_QUAD_BYTE_SIZE = Vertex3dTextured::BYTE_SIZE * SINGLE_QUAD_VERTEX_COUNT,
			
			BATCH_MAX_GEO_COUNT_QUAD = 10000,
			BATCH_MAX_COUNT_TEXTURE = 8,

			BATCH_QUAD_VERTEX_COUNT = BATCH_MAX_GEO_COUNT_QUAD * SINGLE_QUAD_VERTEX_COUNT,
			BATCH_QUAD_INDEX_COUNT = BATCH_MAX_GEO_COUNT_QUAD * SINGLE_QUAD_INDEX_COUNT,
			BATCH_QUAD_COMPONENT_COUNT = BATCH_MAX_GEO_COUNT_QUAD * SINGLE_QUAD_COMPONENT_COUNT,
			BATCH_QUAD_BYTE_SIZE = BATCH_MAX_GEO_COUNT_QUAD * SINGLE_QUAD_BYTE_SIZE
		};

		Renderer2dStorageVulkan(RenderingContextVulkan& context);
		Renderer2dStorageVulkan(const Renderer2dStorageVulkan& copy) = delete;
		void operator=(const Renderer2dStorageVulkan& assignment) = delete;

		void init(VkDevice logicDevice);
		void close(VkDevice logicDevice);
		void onSwapChainResize(VkDevice logicDevice, SwapChainVulkan& swapChain);
		size_t createNewBatchQuad();

		inline VkDescriptorPool getDescriptorPool() const { return mDescriptorPool; }
		inline GraphicsPipelineVulkan& getQuadGraphicsPipeline() const { return *mQuadGraphicsPipeline; }
		inline const TextureVulkan& getWhiteTexture() const { return *mWhiteTexture; }
		inline std::vector<RenderBatchQuad>& getQuadRenderBatches() { return mQuadRenderBatches; }
		inline std::vector<std::shared_ptr<SimpleRenderable>>& getRenderableQuadBatches() { return mRenderableQuadBatches; }
		inline const std::vector<std::shared_ptr<TextureVulkan>>& getTestTextures() const { return mTestTextures; }
		inline const std::shared_ptr<MonoSpaceTextureAtlasVulkan> getTestTextureAtlas() const { return mTestTexturesAtlas; }
		inline TextureArray& getQuadBatchTextureArray() const { return *mQuadBatchTextureArray; }
		inline IndexBufferVulkan& getQuadBatchIndexBuffer() const { return *mQuadSharedIndexBuffer; }
	};
}
