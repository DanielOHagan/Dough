#pragma once

#include "dough/rendering/pipeline/GraphicsPipelineVulkan.h"
#include "dough/rendering/renderer2d/RenderBatchQuad.h"

namespace DOH {

	class RenderingContextVulkan;

	class Renderer2dStorageVulkan {

	private:
		RenderingContextVulkan& mContext;

		const static std::string QUAD_SHADER_PATH_VERT;
		const static std::string QUAD_SHADER_PATH_FRAG;

		VkDescriptorPool mDescriptorPool;

		std::shared_ptr<ShaderProgramVulkan> mQuadShaderProgram;
		std::shared_ptr<GraphicsPipelineVulkan> mQuadGraphicsPipeline;
		std::shared_ptr<VertexArrayVulkan> mQuadVao;

		//std::unique_ptr<RenderBatchQuad> mRenderBatchQuad;
		std::vector<RenderBatchQuad> mQuadRenderBatches;

		//Textures
		std::shared_ptr<TextureVulkan> mWhiteTexture;

		//TEMP::
		const std::string testTexturesPath = "res/images/test textures/";
		std::vector<std::shared_ptr<TextureVulkan>> mTestTextures;

		void initForQuads(VkDevice logicDevice);

	public:
		//Max count of objects allowed per batch
		
		//BATCH_QUAD_COUNT is given an arbitrary number, it can be higher or lower depending on need.
		//Different batches may even use different sizes and not stick to this pre-determined limit,
		//which isn't even enforced by the engine.
		//Maybe, for optimisation, the renderer might grant a higher limit to scene batches than UI batches.
		enum BatchSizeLimits {
			SINGLE_QUAD_VERTEX_COUNT = 4,
			SINGLE_QUAD_INDEX_COUNT = 6,
			SINGLE_QUAD_COMPONENT_COUNT = Vertex3dTextured::COMPONENT_COUNT * SINGLE_QUAD_VERTEX_COUNT,
			SINGLE_QUAD_BYTE_SIZE = Vertex3dTextured::BYTE_SIZE * SINGLE_QUAD_VERTEX_COUNT,
			
			BATCH_QUAD_COUNT = 10000,
			BATCH_QUAD_VERTEX_COUNT = BATCH_QUAD_COUNT * SINGLE_QUAD_VERTEX_COUNT,
			BATCH_QUAD_INDEX_COUNT = BATCH_QUAD_COUNT * SINGLE_QUAD_INDEX_COUNT,
			BATCH_QUAD_COMPONENT_COUNT = BATCH_QUAD_COUNT * SINGLE_QUAD_COMPONENT_COUNT,
			BATCH_QUAD_BYTE_SIZE = BATCH_QUAD_COUNT * SINGLE_QUAD_BYTE_SIZE,

			BATCH_MAX_TEXTURE_COUNT = 8
		};

		Renderer2dStorageVulkan(RenderingContextVulkan& context);
		Renderer2dStorageVulkan(const Renderer2dStorageVulkan& copy) = delete;
		void operator=(const Renderer2dStorageVulkan& assignment) = delete;

		void init(VkDevice logicDevice);
		void close(VkDevice logicDevice);
		void closeSwapChainSpecificObjects(VkDevice logicDevice);
		void recreateSwapChainSpecificObjects();

		//void addSceneQuad(Quad& quad);
		//void addSceneQuad(Quad& quad, uint32_t textureSlotIndex);
		//void addSceneQuadArray(std::vector<Quad>& quadArr);
		//void addSceneQuadArray(std::vector<Quad>& quadArr, uint32_t textureSlotIndex);

		inline VkDescriptorPool getDescriptorPool() const { return mDescriptorPool; }
		inline GraphicsPipelineVulkan& getQuadGraphicsPipeline() const { return *mQuadGraphicsPipeline; }
		inline const TextureVulkan& getWhiteTexture() const { return *mWhiteTexture; }
		inline VertexArrayVulkan& getQuadVao() const { return *mQuadVao; }
		inline std::vector<RenderBatchQuad>& getQuadRenderBatches() { return mQuadRenderBatches; }
		inline const std::vector<std::shared_ptr<TextureVulkan>>& getTestTextures() const { return mTestTextures; }

		//inline RenderBatchQuad& getRenderBatchQuad() const { return *mRenderBatchQuad; }
		//inline bool sceneQuadBatchHasSpace(size_t quadCount) const {
		//	return mSceneQuadCount + quadCount <= BatchSizeLimits::QUAD_COUNT;
		//};
		//inline bool sceneQuadBatchHasSpace(size_t quadCount) const {
		//	return mRenderBatchQuad->hasSpace(static_cast<uint32_t>(quadCount));
		//};
	};
}
