#pragma once

#include "dough/rendering/pipeline/GraphicsPipelineVulkan.h"
#include "dough/scene/geometry/Quad.h"

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
		std::shared_ptr<VertexBufferVulkan> mQuadVbo;

		std::vector<float> mSceneQuadData;
		uint32_t mSceneQuadDataIndex;
		uint32_t mSceneQuadCount;

		//Textures
		std::shared_ptr<TextureVulkan> mWhiteTexture;

		//TEMP::
		const std::string testTexturesPath = "res/images/test textures/";
		std::vector<std::shared_ptr<TextureVulkan>> mTestTextures;

		void initForQuads(VkDevice logicDevice);

		void addSceneQuadVertexToBatch(
			float posX,
			float posY,
			float posZ,
			float colourR,
			float colourG,
			float colourB,
			float colourA,
			float texCoordU,
			float texCoordV,
			float texIndex
		);

	public:
		//Max count of objects allowed per batch
		enum BatchSizeLimits {
			SINGLE_QUAD_VERTEX_COUNT = 4,
			SINGLE_QUAD_INDEX_COUNT = 6,
			QUAD_COUNT = 1000,
			QUAD_VERTEX_COUNT = QUAD_COUNT * SINGLE_QUAD_VERTEX_COUNT,
			QUAD_INDEX_COUNT = QUAD_COUNT * SINGLE_QUAD_INDEX_COUNT,
			QUAD_COMPONENT_COUNT = QUAD_COUNT * Vertex3dTextured::COMPONENT_COUNT,
			QUAD_BYTES = QUAD_COUNT * Vertex3dTextured::BYTE_SIZE
		};

		Renderer2dStorageVulkan(RenderingContextVulkan& context);
		Renderer2dStorageVulkan(const Renderer2dStorageVulkan& copy) = delete;
		void operator=(const Renderer2dStorageVulkan& assignment) = delete;

		void init(VkDevice logicDevice);
		void close(VkDevice logicDevice);
		void closeSwapChainSpecificObjects(VkDevice logicDevice);
		void recreateSwapChainSpecificObjects();

		void addSceneQuad(Quad& quad);
		void addSceneQuad(Quad& quad, uint32_t textureSlotIndex);
		void resetBatch();

		inline GraphicsPipelineVulkan& getQuadGraphicsPipeline() const { return *mQuadGraphicsPipeline; }
		inline VertexArrayVulkan& getQuadVao() const { return *mQuadVao; }
		inline VertexBufferVulkan& getQuadVbo() const { return *mQuadVbo; }
		inline const std::vector<float>& getSceneQuadData() const { return mSceneQuadData; }
		inline const uint32_t getSceneQuadCount() const { return mSceneQuadCount; }


		inline bool sceneQuadBatchHasSpace(size_t quadCount) const {
			return mSceneQuadCount + quadCount <= BatchSizeLimits::QUAD_COUNT;
		};
	};
}
