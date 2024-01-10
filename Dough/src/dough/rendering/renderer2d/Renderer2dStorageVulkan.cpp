#include "dough/rendering/renderer2d/Renderer2dStorageVulkan.h"

#include "dough/rendering/RenderingContextVulkan.h"
#include "dough/rendering/ObjInit.h"
#include "dough/Logging.h"

namespace DOH {

	const std::string Renderer2dStorageVulkan::QUAD_SHADER_PATH_VERT = "Dough/res/shaders/spv/QuadBatch.vert.spv";
	const std::string Renderer2dStorageVulkan::QUAD_SHADER_PATH_FRAG = "Dough/res/shaders/spv/QuadBatch.frag.spv";

	Renderer2dStorageVulkan::Renderer2dStorageVulkan(RenderingContextVulkan& context)
	:	mContext(context),
		mDescriptorPool(VK_NULL_HANDLE)
	{}

	void Renderer2dStorageVulkan::init() {
		mWhiteTexture = ObjInit::texture(
			255.0f,
			255.0f,
			255.0f,
			255.0f,
			false,
			"White Texture"
		);

		initForQuads();

		TextRenderer::init(mContext, *mWhiteTexture, mQuadSharedIndexBuffer);

		createDescriptorPool();

		mContext.createPipelineUniformObjects(*mQuadGraphicsPipeline, mDescriptorPool);
		TextRenderer::createPipelineUniformObjects(mDescriptorPool);
	}

	void Renderer2dStorageVulkan::close() {
		VkDevice logicDevice = mContext.getLogicDevice();

		mQuadShaderProgram->close(logicDevice);
		mQuadGraphicsPipeline->close(logicDevice);

		TextRenderer::close();
		
		mTestMonoSpaceTextureAtlas->close(logicDevice);
		mTestIndexedTextureAtlas->close(logicDevice);

		mWhiteTexture->close(logicDevice);

		for (std::shared_ptr<TextureVulkan> texture : mTestTextures) {
			texture->close(logicDevice);
		}

		for (const auto& renderableQuadBatch : mRenderableQuadBatches) {
			renderableQuadBatch->getVao().close(logicDevice);
		}

		mQuadSharedIndexBuffer->close(logicDevice);

		vkDestroyDescriptorPool(logicDevice, mDescriptorPool, nullptr);
	}

	void Renderer2dStorageVulkan::createDescriptorPool() {
		std::vector<DescriptorTypeInfo> totalDescTypes;
		{
			std::vector<DescriptorTypeInfo> descTypes = mQuadGraphicsPipeline->getShaderProgram().getShaderDescriptorLayout().asDescriptorTypes();
			totalDescTypes.reserve(totalDescTypes.size() + descTypes.size());

			for (const DescriptorTypeInfo& descType : descTypes) {
				totalDescTypes.emplace_back(descType);
			}
		}

		{
			std::vector<DescriptorTypeInfo> descTypes = TextRenderer::getDescriptorTypeInfos();
			totalDescTypes.reserve(totalDescTypes.size() + descTypes.size());
			for (const DescriptorTypeInfo& descType : descTypes) {
				totalDescTypes.emplace_back(descType);
			}
		}

		if (totalDescTypes.size() == 0) {
			THROW("Quad / Text batch rednering pipelines should have at least one descriptor");
		}

		mDescriptorPool = mContext.createDescriptorPool(totalDescTypes);
	}

	void Renderer2dStorageVulkan::onSwapChainResize(SwapChainVulkan& swapChain) {
		VkDevice logicDevice = mContext.getLogicDevice();
		
		vkDestroyDescriptorPool(logicDevice, mDescriptorPool, nullptr);

		createDescriptorPool();

		mQuadGraphicsPipeline->recreate(
			logicDevice,
			swapChain.getExtent(),
			mContext.getRenderPass(ERenderPass::APP_SCENE).get()
		);
		mContext.createPipelineUniformObjects(*mQuadGraphicsPipeline, mDescriptorPool);

		TextRenderer::onSwapChainResize(swapChain);
		TextRenderer::createPipelineUniformObjects(mDescriptorPool);
	}

	size_t Renderer2dStorageVulkan::createNewBatchQuad() {
		if (mQuadRenderBatches.size() < BatchSizeLimits::MAX_BATCH_COUNT_QUAD) {

			constexpr size_t batchSizeBytes = BatchSizeLimits::BATCH_QUAD_BYTE_SIZE;

			RenderBatchQuad& batch = mQuadRenderBatches.emplace_back(
				BatchSizeLimits::BATCH_MAX_GEO_COUNT_QUAD,
				BatchSizeLimits::BATCH_MAX_COUNT_TEXTURE
			);

			std::shared_ptr<VertexArrayVulkan> vao = ObjInit::vertexArray();
			std::shared_ptr<VertexBufferVulkan> vbo = ObjInit::vertexBuffer(
				StaticVertexInputLayout::get(RenderBatchQuad::VERTEX_INPUT_TYPE),
				batchSizeBytes,
				VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
			);
			vao->addVertexBuffer(vbo);
			vao->setIndexBuffer(mQuadSharedIndexBuffer, true);
			vao->getVertexBuffers()[0]->map(mContext.getLogicDevice(), batchSizeBytes);

			mRenderableQuadBatches.emplace_back(std::make_shared<SimpleRenderable>(vao));

			return mQuadRenderBatches.size() - 1;
		} else {
			return -1;
		}
	}

	void Renderer2dStorageVulkan::initForQuads() {
		mQuadShaderProgram = ObjInit::shaderProgram(
			ObjInit::shader(EShaderType::VERTEX, Renderer2dStorageVulkan::QUAD_SHADER_PATH_VERT),
			ObjInit::shader(EShaderType::FRAGMENT, Renderer2dStorageVulkan::QUAD_SHADER_PATH_FRAG)
		);

		mQuadBatchTextureArray = std::make_unique<TextureArray>(
			BatchSizeLimits::BATCH_MAX_COUNT_TEXTURE,
			*mWhiteTexture
		);

		//Add blacnk white texture for quads that aren't using a texture
		mQuadBatchTextureArray->addNewTexture(*mWhiteTexture);

		//Commented out after implementation of Texture Atlas,
		// left as comment to show how to use individual textures
		//for (int i = 0; i < 8; i++) {
		//	std::string path = testTexturesPath + "texture" + std::to_string(i) + ".png";
		//	std::shared_ptr<TextureVulkan> testTexture = ObjInit::texture(path);
		//	mTestTextures.emplace_back(ObjInit::texture(path));
		//	
		//	//TODO:: dynamic texture arrays, instead of assigning at initialisation
		//	mQuadBatchTextureArray->addNewTexture(*testTexture);
		//}
		
		mTestMonoSpaceTextureAtlas = ObjInit::monoSpaceTextureAtlas(
			"Dough/res/images/test textures/texturesAtlas.png",
			5,
			5
		);
		mTestIndexedTextureAtlas = ObjInit::indexedTextureAtlas(
			"Dough/res/images/textureAtlasses/indexed_testAtlas.txt",
			"Dough/res/images/textureAtlasses/"
		);

		mQuadBatchTextureArray->addNewTexture(*mTestMonoSpaceTextureAtlas);
		mQuadBatchTextureArray->addNewTexture(*mTestIndexedTextureAtlas);

		ShaderUniformLayout& layout = mQuadShaderProgram->getUniformLayout();
		layout.setValue(0, sizeof(glm::mat4x4));
		layout.setTextureArray(1, *mQuadBatchTextureArray);

		mQuadGraphicsPipelineInstanceInfo = std::make_unique<GraphicsPipelineInstanceInfo>(
			StaticVertexInputLayout::get(RenderBatchQuad::VERTEX_INPUT_TYPE),
			*mQuadShaderProgram,
			ERenderPass::APP_SCENE
		);
		mQuadGraphicsPipelineInstanceInfo->getOptionalFields().setDepthTesting(true, VK_COMPARE_OP_LESS);
		mQuadGraphicsPipelineInstanceInfo->getOptionalFields().setBlending(
			true,
			VK_BLEND_OP_ADD,
			VK_BLEND_FACTOR_SRC_ALPHA,
			VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
			VK_BLEND_OP_ADD,
			VK_BLEND_FACTOR_SRC_ALPHA,
			VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA
		);

		mQuadGraphicsPipeline = mContext.createGraphicsPipeline(
			*mQuadGraphicsPipelineInstanceInfo,
			mContext.getSwapChain().getExtent()
		);

		//Quad Index Buffer
		std::vector<uint32_t> quadIndices;
		quadIndices.resize(BatchSizeLimits::BATCH_QUAD_INDEX_COUNT);
		uint32_t vertexOffset = 0;
		for (uint32_t i = 0; i < BatchSizeLimits::BATCH_QUAD_INDEX_COUNT; i += BatchSizeLimits::SINGLE_QUAD_INDEX_COUNT) {
			quadIndices[i + 0] = vertexOffset + 0;
			quadIndices[i + 1] = vertexOffset + 1;
			quadIndices[i + 2] = vertexOffset + 2;

			quadIndices[i + 3] = vertexOffset + 2;
			quadIndices[i + 4] = vertexOffset + 3;
			quadIndices[i + 5] = vertexOffset + 0;

			vertexOffset += BatchSizeLimits::SINGLE_QUAD_VERTEX_COUNT;
		}
		mQuadSharedIndexBuffer = ObjInit::stagedIndexBuffer(
			quadIndices.data(),
			sizeof(uint32_t) * BatchSizeLimits::BATCH_QUAD_INDEX_COUNT
		);
	}
}
