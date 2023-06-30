#include "dough/rendering/renderer2d/Renderer2dStorageVulkan.h"

#include "dough/rendering/RenderingContextVulkan.h"
#include "dough/rendering/ObjInit.h"
#include "dough/Logging.h"

namespace DOH {

	const std::string Renderer2dStorageVulkan::QUAD_SHADER_PATH_VERT = "Dough/res/shaders/spv/QuadBatch.vert.spv";
	const std::string Renderer2dStorageVulkan::QUAD_SHADER_PATH_FRAG = "Dough/res/shaders/spv/QuadBatch.frag.spv";
	const std::string Renderer2dStorageVulkan::TEXT_2D_SHADER_PATH_VERT = "Dough/res/shaders/spv/Text2d.vert.spv";
	const std::string Renderer2dStorageVulkan::TEXT_2D_SHADER_PATH_FRAG = "Dough/res/shaders/spv/Text2d.frag.spv";

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
		initForText();

		createDescriptorPool();

		mContext.createPipelineUniformObjects(*mQuadGraphicsPipeline, mDescriptorPool);
		mContext.createPipelineUniformObjects(*mTextGraphicsPipeline, mDescriptorPool);
	}

	void Renderer2dStorageVulkan::close() {
		VkDevice logicDevice = mContext.getLogicDevice();

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

		for (auto& fontBitmap : mFontBitmaps) {
			for (auto& page : fontBitmap.second->getPageTextures()) {
				page->close(logicDevice);
			}
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

		mTextGraphicsPipeline->recreate(
			logicDevice,
			swapChain.getExtent(),
			mContext.getRenderPass(ERenderPass::APP_SCENE).get()
		);
		mContext.createPipelineUniformObjects(*mTextGraphicsPipeline, mDescriptorPool);
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
		
		mTestTexturesAtlas = ObjInit::monoSpaceTextureAtlas(
			"Dough/res/images/test textures/texturesAtlas.png",
			5,
			5
		);
		mQuadBatchTextureArray->addNewTexture(*mTestTexturesAtlas);

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
			false,
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

	void Renderer2dStorageVulkan::initForText() {
		mTextBatchTextureArray = std::make_unique<TextureArray>(
			BatchSizeLimits::BATCH_MAX_COUNT_TEXTURE,
			*mWhiteTexture
		);

		mFontBitmaps = {};
		const auto& arialFont = mFontBitmaps.emplace(
			Renderer2dStorageVulkan::DEFAULT_FONT_BITMAP_NAME, //Store Arial font as default
			ObjInit::fontBitmap("Dough/res/fonts/arial_latin_32px.fnt", "Dough/res/fonts/")
		);
		if (arialFont.second) {
			addFontBitmapToTextTextureArray(*arialFont.first->second);
		} else {
			LOG_ERR("Failed to store default font: " << Renderer2dStorageVulkan::DEFAULT_FONT_BITMAP_NAME);
			THROW("");
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

		constexpr size_t batchSizeBytes = BatchSizeLimits::BATCH_QUAD_BYTE_SIZE;

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

		mRenderableTextBatch = std::make_shared<SimpleRenderable>(vao);

		mTextGraphicsPipelineInstanceInfo = std::make_unique<GraphicsPipelineInstanceInfo>(
			StaticVertexInputLayout::get(RenderBatchQuad::VERTEX_INPUT_TYPE),
			*mTextShaderProgram,
			ERenderPass::APP_SCENE
		);

		//Text isn't culled by default for viewing from behind.
		//Text depth testing in scene so it doesn't overlap, however, this causes z-fighting during kerning as each glyph is a rendered quad.
		mTextGraphicsPipelineInstanceInfo->getOptionalFields().CullMode = VK_CULL_MODE_NONE;
		mTextGraphicsPipelineInstanceInfo->getOptionalFields().setDepthTesting(true, VK_COMPARE_OP_LESS);
		mTextGraphicsPipelineInstanceInfo->getOptionalFields().setBlending(
			true,
			VK_BLEND_OP_ADD,
			VK_BLEND_FACTOR_SRC_ALPHA,
			VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
			VK_BLEND_OP_ADD,
			VK_BLEND_FACTOR_SRC_ALPHA,
			VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA
		);

		mTextGraphicsPipeline = mContext.createGraphicsPipeline(
			*mTextGraphicsPipelineInstanceInfo,
			mContext.getSwapChain().getExtent()
		);
	}

	void Renderer2dStorageVulkan::addFontBitmapToTextTextureArray(const FontBitmap& fontBitmap) {
		uint32_t bitmapTexturesAdded = 0;
		for (const auto& texture : fontBitmap.getPageTextures()) {
			mTextBatchTextureArray->addNewTexture(*texture);
			bitmapTexturesAdded++;
		}
		if (bitmapTexturesAdded < fontBitmap.getPageCount()) {
			LOG_ERR(
				"Failed to add all arial bitmap textures to quad batch texture array. Missing: " <<
				fontBitmap.getPageCount() - bitmapTexturesAdded
			);
		}
	}

	FontBitmap& Renderer2dStorageVulkan::getFontBitmap(const char* font) const {
		const auto& fontItr = mFontBitmaps.find(font);
		if (fontItr != mFontBitmaps.end()) {
			return *fontItr->second;
		} else {
			const auto& defaultFontItr = mFontBitmaps.find(Renderer2dStorageVulkan::DEFAULT_FONT_BITMAP_NAME);
			return *defaultFontItr->second;
		}
	}
}
