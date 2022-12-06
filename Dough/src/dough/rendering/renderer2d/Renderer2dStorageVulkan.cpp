#include "dough/rendering/renderer2d/Renderer2dStorageVulkan.h"

#include "dough/rendering/RenderingContextVulkan.h"
#include "dough/rendering/ObjInit.h"
#include "dough/Logging.h"

namespace DOH {

	const std::string Renderer2dStorageVulkan::QUAD_SHADER_PATH_VERT = "res/shaders/spv/QuadBatch.vert.spv";
	const std::string Renderer2dStorageVulkan::QUAD_SHADER_PATH_FRAG = "res/shaders/spv/QuadBatch.frag.spv";
	const std::string Renderer2dStorageVulkan::TEXT_2D_SHADER_PATH_VERT = "res/shaders/spv/Text2d.vert.spv";
	const std::string Renderer2dStorageVulkan::TEXT_2D_SHADER_PATH_FRAG = "res/shaders/spv/Text2d.frag.spv";

	Renderer2dStorageVulkan::Renderer2dStorageVulkan(RenderingContextVulkan& context)
	:	mContext(context),
		mDescriptorPool(VK_NULL_HANDLE)
	{}

	void Renderer2dStorageVulkan::init(VkDevice logicDevice) {
		mWhiteTexture = ObjInit::texture(255.0f, 255.0f, 255.0f, 255.0f, false);

		initForQuads(logicDevice);
		initForText(logicDevice);

		createDescriptorPool();

		mContext.createPipelineUniformObjects(*mQuadGraphicsPipeline, mDescriptorPool);
		mContext.createPipelineUniformObjects(*mTextGraphicsPipeline, mDescriptorPool);
	}

	void Renderer2dStorageVulkan::close(VkDevice logicDevice) {
		mQuadShaderProgram->close(logicDevice);
		mQuadGraphicsPipeline->close(logicDevice);

		mTextShaderProgram->close(logicDevice);
		mTextGraphicsPipeline->close(logicDevice);
		mRenderableTextBatch->getVao().close(logicDevice);
		mTestTexturesAtlas->close(logicDevice);

		mWhiteTexture->close(logicDevice);

		for (std::shared_ptr<TextureVulkan> texture : mTestTextures) {
			texture->close(logicDevice);
		}

		for (const auto& renderableQuadBatch : mRenderableQuadBatches) {
			renderableQuadBatch->getVao().close(logicDevice);
		}

		for (const auto& texture : mArialBitmap->getPageTextures()) {
			texture->close(logicDevice);
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
			std::vector<DescriptorTypeInfo> descTypes = mTextGraphicsPipeline->getShaderProgram().getShaderDescriptorLayout().asDescriptorTypes();
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

	void Renderer2dStorageVulkan::onSwapChainResize(VkDevice logicDevice, SwapChainVulkan& swapChain) {
		vkDestroyDescriptorPool(logicDevice, mDescriptorPool, nullptr);

		createDescriptorPool();

		mQuadGraphicsPipeline->recreate(
			logicDevice,
			swapChain.getExtent(),
			mContext.getRenderPass(ERenderPass::APP_SCENE).get()
		);
		mContext.createPipelineUniformObjects(*mQuadGraphicsPipeline, mDescriptorPool);

		mTextGraphicsPipeline->recreate(
			logicDevice,
			swapChain.getExtent(),
			mContext.getRenderPass(ERenderPass::APP_SCENE).get()
		);
		mContext.createPipelineUniformObjects(*mTextGraphicsPipeline, mDescriptorPool);
	}

	size_t Renderer2dStorageVulkan::createNewBatchQuad() {
		if (mQuadRenderBatches.size() < BatchSizeLimits::MAX_BATCH_COUNT_QUAD) {
			RenderBatchQuad& batch = mQuadRenderBatches.emplace_back(
				BatchSizeLimits::BATCH_MAX_GEO_COUNT_QUAD,
				BatchSizeLimits::BATCH_MAX_COUNT_TEXTURE
			);

			std::shared_ptr<VertexArrayVulkan> vao = ObjInit::vertexArray();
			std::shared_ptr<VertexBufferVulkan> vbo = ObjInit::vertexBuffer(
				EVertexType::VERTEX_3D_TEXTURED,
				BatchSizeLimits::BATCH_QUAD_BYTE_SIZE,
				VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
			);
			vao->addVertexBuffer(vbo);
			vao->setIndexBuffer(mQuadSharedIndexBuffer, true);

			mRenderableQuadBatches.emplace_back(std::make_shared<SimpleRenderable>(vao));

			return mQuadRenderBatches.size() - 1;
		} else {
			return -1;
		}
	}

	void Renderer2dStorageVulkan::initForQuads(VkDevice logicDevice) {
		mQuadShaderProgram = ObjInit::shaderProgram(
			ObjInit::shader(EShaderType::VERTEX, Renderer2dStorageVulkan::QUAD_SHADER_PATH_VERT),
			ObjInit::shader(EShaderType::FRAGMENT, Renderer2dStorageVulkan::QUAD_SHADER_PATH_FRAG)
		);

		mQuadBatchTextureArray = std::make_unique<TextureArray>(
			BatchSizeLimits::BATCH_MAX_COUNT_TEXTURE,
			*mWhiteTexture
		);

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
		
		mTestTexturesAtlas = ObjInit::monoSpaceTextureAtlas(
			"res/images/test textures/texturesAtlas.png",
			5,
			5
		);
		mQuadBatchTextureArray->addNewTexture(*mTestTexturesAtlas);

		ShaderUniformLayout& layout = mQuadShaderProgram->getUniformLayout();
		layout.setValue(0, sizeof(glm::mat4x4));
		layout.setTextureArray(1, *mQuadBatchTextureArray);

		mQuadGraphicsPipelineInstanceInfo = std::make_unique<GraphicsPipelineInstanceInfo>(
			EVertexType::VERTEX_3D_TEXTURED,
			*mQuadShaderProgram,
			ERenderPass::APP_SCENE
		);
		mQuadGraphicsPipelineInstanceInfo->DepthTestingEnabled = true;
		mQuadGraphicsPipelineInstanceInfo->DepthCompareOp = VK_COMPARE_OP_LESS;
		mQuadGraphicsPipelineInstanceInfo->BlendingEnabled = false;
		mQuadGraphicsPipelineInstanceInfo->ColourBlendOp = VK_BLEND_OP_ADD;
		mQuadGraphicsPipelineInstanceInfo->ColourBlendSrcFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		mQuadGraphicsPipelineInstanceInfo->ColourBlendDstFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		mQuadGraphicsPipelineInstanceInfo->AlphaBlendOp = VK_BLEND_OP_ADD;
		mQuadGraphicsPipelineInstanceInfo->AlphaBlendSrcFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		mQuadGraphicsPipelineInstanceInfo->AlphaBlendDstFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;

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

	void Renderer2dStorageVulkan::initForText(VkDevice logicDevice) {
		mArialBitmap = ObjInit::fontBitmap("res/fonts/arial_latin_32px.fnt", "res/fonts/");

		mTextBatchTextureArray = std::make_unique<TextureArray>(
			BatchSizeLimits::BATCH_MAX_COUNT_TEXTURE,
			*mWhiteTexture
		);

		uint32_t bitmapTexturesAdded = 0;
		for (const auto& texture : mArialBitmap->getPageTextures()) {
			mTextBatchTextureArray->addNewTexture(*texture);
			bitmapTexturesAdded++;
		}
		if (bitmapTexturesAdded < mArialBitmap->getPageCount()) {
			LOG_ERR("Failed to add all arial bitmap textures to quad batch texture array. Missing: " << mArialBitmap->getPageCount() - bitmapTexturesAdded);
		}

		mTextShaderProgram = ObjInit::shaderProgram(
			ObjInit::shader(EShaderType::VERTEX, Renderer2dStorageVulkan::TEXT_2D_SHADER_PATH_VERT),
			ObjInit::shader(EShaderType::FRAGMENT, Renderer2dStorageVulkan::TEXT_2D_SHADER_PATH_FRAG)
		);

		ShaderUniformLayout& layout = mTextShaderProgram->getUniformLayout();
		layout.setValue(0, sizeof(glm::mat4x4));
		layout.setTextureArray(1, *mTextBatchTextureArray);

		mTextRenderBatch = std::make_unique<RenderBatchQuad>(
			BatchSizeLimits::BATCH_MAX_GEO_COUNT_QUAD,
			BatchSizeLimits::BATCH_MAX_COUNT_TEXTURE
		);

		std::shared_ptr<VertexArrayVulkan> vao = ObjInit::vertexArray();
		std::shared_ptr<VertexBufferVulkan> vbo = ObjInit::vertexBuffer(
			EVertexType::VERTEX_3D_TEXTURED,
			BatchSizeLimits::BATCH_QUAD_BYTE_SIZE,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
		);
		vao->addVertexBuffer(vbo);
		vao->setIndexBuffer(mQuadSharedIndexBuffer, true);

		mRenderableTextBatch = std::make_shared<SimpleRenderable>(vao);

		mTextGraphicsPipelineInstanceInfo = std::make_unique<GraphicsPipelineInstanceInfo>(
			EVertexType::VERTEX_3D_TEXTURED,
			*mTextShaderProgram,
			ERenderPass::APP_SCENE
		);
		mTextGraphicsPipelineInstanceInfo->CullMode = VK_CULL_MODE_NONE;
		mTextGraphicsPipelineInstanceInfo->DepthTestingEnabled = false;
		mTextGraphicsPipelineInstanceInfo->BlendingEnabled = true;
		mTextGraphicsPipelineInstanceInfo->ColourBlendOp = VK_BLEND_OP_ADD;
		mTextGraphicsPipelineInstanceInfo->ColourBlendSrcFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		mTextGraphicsPipelineInstanceInfo->ColourBlendDstFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		mTextGraphicsPipelineInstanceInfo->AlphaBlendOp = VK_BLEND_OP_ADD;
		mTextGraphicsPipelineInstanceInfo->AlphaBlendSrcFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		mTextGraphicsPipelineInstanceInfo->AlphaBlendDstFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;

		mTextGraphicsPipeline = mContext.createGraphicsPipeline(
			*mTextGraphicsPipelineInstanceInfo,
			mContext.getSwapChain().getExtent()
		);
	}
}
