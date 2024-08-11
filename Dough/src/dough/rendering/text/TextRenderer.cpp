#include "dough/rendering/text/TextRenderer.h"

#include "dough/rendering/RenderingContextVulkan.h"
#include "dough/Logging.h"
#include "dough/application/Application.h"
#include "dough/rendering/ShapeRenderer.h"

#include <tracy/public/tracy/Tracy.hpp>

namespace DOH {

	std::unique_ptr<TextRenderer> TextRenderer::INSTANCE = nullptr;

	const char* TextRenderer::SOFT_MASK_SHADER_PATH_VERT = "Dough/Dough/res/shaders/spv/TextSoftMask3d.vert.spv";
	const char* TextRenderer::SOFT_MASK_SHADER_PATH_FRAG = "Dough/Dough/res/shaders/spv/TextSoftMask3d.frag.spv";
	const char* TextRenderer::MSDF_SHADER_PATH_VERT = "Dough/Dough/res/shaders/spv/TextMsdf3d.vert.spv";
	const char* TextRenderer::MSDF_SHADER_PATH_FRAG = "Dough/Dough/res/shaders/spv/TextMsdf3d.frag.spv";

	TextRenderer::TextRenderer(RenderingContextVulkan& context)
	:	mContext(context),
		mFontBitmapPagesDescSet(VK_NULL_HANDLE),
		mQuadIndexBufferShared(false),
		mDrawnQuadCount(0)
	{}

	void TextRenderer::initImpl() {
		ZoneScoped;

		mFontBitmapPagesTextureArary = std::make_unique<TextureArray>(
			EBatchSizeLimits::BATCH_MAX_COUNT_TEXTURE,
			*mContext.getResourceDefaults().WhiteTexture
		);

		mFontBitmaps = {};
		mSoftMaskRendering = {};
		mMsdfRendering = {};

		mQuadIndexBuffer = ShapeRenderer::getQuadSharedIndexBufferPtr();
		if (mQuadIndexBuffer == nullptr) {
			std::vector<uint32_t> quadIndices;
			quadIndices.resize(EBatchSizeLimits::QUAD_BATCH_INDEX_COUNT);
			uint32_t vertexOffset = 0;
			for (uint32_t i = 0; i < EBatchSizeLimits::QUAD_BATCH_INDEX_COUNT; i += EBatchSizeLimits::QUAD_INDEX_COUNT) {
				quadIndices[i + 0] = vertexOffset + 0;
				quadIndices[i + 1] = vertexOffset + 1;
				quadIndices[i + 2] = vertexOffset + 2;

				quadIndices[i + 3] = vertexOffset + 2;
				quadIndices[i + 4] = vertexOffset + 3;
				quadIndices[i + 5] = vertexOffset + 0;

				vertexOffset += EBatchSizeLimits::QUAD_VERTEX_COUNT;
			}
			mQuadIndexBuffer = mContext.createStagedIndexBuffer(
				quadIndices.data(),
				sizeof(uint32_t) * EBatchSizeLimits::QUAD_BATCH_INDEX_COUNT
			);
		} else {
			mQuadIndexBufferShared = true;
		}

		DescriptorSetLayoutVulkan& texArrSetLayout = mContext.getCommonDescriptorSetLayouts().SingleTextureArray8;
		std::vector<std::reference_wrapper<DescriptorSetLayoutVulkan>> textDescSets = {
			mContext.getCommonDescriptorSetLayouts().Ubo,
			texArrSetLayout
		};
		std::shared_ptr<ShaderDescriptorSetLayoutsVulkan> textDescSetLayouts = std::make_shared<ShaderDescriptorSetLayoutsVulkan>(textDescSets);

		mFontBitmapPagesDescSet = DescriptorApiVulkan::allocateDescriptorSetFromLayout(
			mContext.getLogicDevice(),
			mContext.getEngineDescriptorPool(),
			texArrSetLayout
		);
		std::initializer_list<VkDescriptorSet> fontDescSets = { VK_NULL_HANDLE, mFontBitmapPagesDescSet };
		mFontRenderingDescSetsInstanceScene = std::make_shared<DescriptorSetsInstanceVulkan>(fontDescSets);
		mFontRenderingDescSetsInstanceUi = std::make_shared<DescriptorSetsInstanceVulkan>(fontDescSets);

		const StaticVertexInputLayout& textVertexLayout = StaticVertexInputLayout::get(EVertexType::VERTEX_3D_TEXTURED_INDEXED);
		const size_t batchSizeBytes = static_cast<size_t>(EBatchSizeLimits::QUAD_BATCH_MAX_GEO_COUNT) * Quad::BYTE_SIZE;

		{ //Load fonts
			bool createdDefaultFont = createFontBitmapImpl(
				TextRenderer::ARIAL_SOFT_MASK_NAME,
				"Dough/Dough/res/fonts/arial_latin_32px.fnt",
				"Dough/Dough/res/fonts/",
				ETextRenderMethod::SOFT_MASK
			);

			if (!createdDefaultFont) {
				THROW("TextRenderer::init failed to create default font bitmap");
			}

			bool createdMsdfFont = createFontBitmapImpl(
				TextRenderer::ARIAL_MSDF_NAME,
				"Dough/Dough/res/fonts/arial_msdf_32px_meta.json",
				"Dough/Dough/res/fonts/",
				ETextRenderMethod::MSDF
			);

			if (!createdMsdfFont) {
				LOG_WARN("TextRenderer::init failed to load MSDF font");
			}
		}

		//Update descriptors after fonts have been loaded
		const uint32_t textureArrBinding = 0;
		DescriptorSetUpdate texArrUpdate = {
			{{ texArrSetLayout.getDescriptors()[textureArrBinding], *mFontBitmapPagesTextureArary }},
			mFontBitmapPagesDescSet
		};
		DescriptorApiVulkan::updateDescriptorSet(mContext.getLogicDevice(), texArrUpdate);

		{ //Soft Mask
			mSoftMaskRendering.SceneVertexShader = mContext.createShader(EShaderStage::VERTEX, TextRenderer::SOFT_MASK_SHADER_PATH_VERT);
			mSoftMaskRendering.SceneFragmentShader = mContext.createShader(EShaderStage::FRAGMENT, TextRenderer::SOFT_MASK_SHADER_PATH_FRAG);
			mSoftMaskRendering.SceneShaderProgram = mContext.createShaderProgram(
				mSoftMaskRendering.SceneVertexShader,
				mSoftMaskRendering.SceneFragmentShader,
				textDescSetLayouts
			);

			//Scene
			mSoftMaskRendering.SceneBatch = std::make_unique<RenderBatchQuad>(
				EBatchSizeLimits::QUAD_BATCH_MAX_GEO_COUNT,
				EBatchSizeLimits::BATCH_MAX_COUNT_TEXTURE
			);

			std::shared_ptr<VertexArrayVulkan> vao = mContext.createVertexArray();
			std::shared_ptr<VertexBufferVulkan> vbo = mContext.createVertexBuffer(
				textVertexLayout,
				batchSizeBytes,
				VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
			);
			vao->addVertexBuffer(vbo);
			vao->setIndexBuffer(mQuadIndexBuffer, true);
			vao->getVertexBuffers()[0]->map(mContext.getLogicDevice(), batchSizeBytes);

			mSoftMaskRendering.SceneRenderableBatch = std::make_shared<SimpleRenderable>(vao, mFontRenderingDescSetsInstanceScene);

			mSoftMaskRendering.ScenePipelineInstanceInfo = std::make_unique<GraphicsPipelineInstanceInfo>(
				textVertexLayout,
				*mSoftMaskRendering.SceneShaderProgram,
				ERenderPass::APP_SCENE
			);
			auto& optionalFields = mSoftMaskRendering.ScenePipelineInstanceInfo->enableOptionalFields();
			//Text isn't culled by default for viewing from behind.
			//Text depth testing in scene so it doesn't overlap, however, this causes z-fighting during kerning as each glyph is a rendered quad.
			optionalFields.CullMode = VK_CULL_MODE_NONE;
			optionalFields.setDepthTesting(true, VK_COMPARE_OP_LESS);
			optionalFields.setBlending(
				true,
				VK_BLEND_OP_ADD,
				VK_BLEND_FACTOR_SRC_ALPHA,
				VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
				VK_BLEND_OP_ADD,
				VK_BLEND_FACTOR_SRC_ALPHA,
				VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA
			);

			mSoftMaskRendering.ScenePipeline = mContext.createGraphicsPipeline(*mSoftMaskRendering.ScenePipelineInstanceInfo);
			mSoftMaskRendering.ScenePipeline->init(
				mContext.getLogicDevice(),
				mContext.getSwapChain().getExtent(),
				mContext.getRenderPass(ERenderPass::APP_SCENE).get()
			);

			//UI
			mSoftMaskRendering.UiBatch = std::make_unique<RenderBatchQuad>(
				EBatchSizeLimits::QUAD_BATCH_MAX_GEO_COUNT,
				EBatchSizeLimits::BATCH_MAX_COUNT_TEXTURE
			);
			
			std::shared_ptr<VertexArrayVulkan> uiVao = mContext.createVertexArray();
			std::shared_ptr<VertexBufferVulkan> uiVbo = mContext.createVertexBuffer(
				textVertexLayout,
				batchSizeBytes,
				VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
			);
			uiVao->addVertexBuffer(uiVbo);
			uiVao->setIndexBuffer(mQuadIndexBuffer, true);
			uiVao->getVertexBuffers()[0]->map(mContext.getLogicDevice(), batchSizeBytes);
			
			mSoftMaskRendering.UiRenderableBatch = std::make_shared<SimpleRenderable>(uiVao, mFontRenderingDescSetsInstanceUi);

			mSoftMaskRendering.UiPipelineInstanceInfo = std::make_unique<GraphicsPipelineInstanceInfo>(
				textVertexLayout,
				*mSoftMaskRendering.SceneShaderProgram,
				ERenderPass::APP_UI
			);
			auto& uiOptionalFields = mSoftMaskRendering.UiPipelineInstanceInfo->enableOptionalFields();
			//Text isn't culled by default for viewing from behind.
			//Text depth testing in scene so it doesn't overlap, however, this causes z-fighting during kerning as each glyph is a rendered quad.
			uiOptionalFields.CullMode = VK_CULL_MODE_NONE;
			//uiOptionalFields.setDepthTesting(true, VK_COMPARE_OP_LESS);
			uiOptionalFields.setBlending(
				true,
				VK_BLEND_OP_ADD,
				VK_BLEND_FACTOR_SRC_ALPHA,
				VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
				VK_BLEND_OP_ADD,
				VK_BLEND_FACTOR_SRC_ALPHA,
				VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA
			);

			mSoftMaskRendering.UiPipeline = mContext.createGraphicsPipeline(*mSoftMaskRendering.UiPipelineInstanceInfo);
			mSoftMaskRendering.UiPipeline->init(
				mContext.getLogicDevice(),
				mContext.getSwapChain().getExtent(),
				mContext.getRenderPass(ERenderPass::APP_UI).get()
			);
		}

		{ //MSDF
			mMsdfRendering.SceneVertexShader = mContext.createShader(EShaderStage::VERTEX, TextRenderer::MSDF_SHADER_PATH_VERT);
			mMsdfRendering.SceneFragmentShader = mContext.createShader(EShaderStage::FRAGMENT, TextRenderer::MSDF_SHADER_PATH_FRAG);
			mMsdfRendering.SceneShaderProgram = mContext.createShaderProgram(
				mMsdfRendering.SceneVertexShader,
				mMsdfRendering.SceneFragmentShader,
				textDescSetLayouts
			);

			//Scene
			mMsdfRendering.SceneBatch = std::make_unique<RenderBatchQuad>(
				EBatchSizeLimits::QUAD_BATCH_MAX_GEO_COUNT,
				EBatchSizeLimits::BATCH_MAX_COUNT_TEXTURE
			);

			mMsdfRendering.ScenePipelineInstanceInfo = std::make_unique<GraphicsPipelineInstanceInfo>(
				StaticVertexInputLayout::get(QUAD_VERTEX_INPUT_TYPE),
				*mMsdfRendering.SceneShaderProgram,
				ERenderPass::APP_SCENE
			);
			auto& optionalFields = mMsdfRendering.ScenePipelineInstanceInfo->enableOptionalFields();
			//Text isn't culled by default for viewing from behind.
			//Text depth testing in scene so it doesn't overlap, however, this causes z-fighting during kerning as each glyph is a rendered quad.
			optionalFields.CullMode = VK_CULL_MODE_NONE;
			optionalFields.setDepthTesting(true, VK_COMPARE_OP_LESS);
			optionalFields.setBlending(
				true,
				VK_BLEND_OP_ADD,
				VK_BLEND_FACTOR_SRC_ALPHA,
				VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
				VK_BLEND_OP_ADD,
				VK_BLEND_FACTOR_SRC_ALPHA,
				VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA
			);

			std::shared_ptr<VertexArrayVulkan> vao = mContext.createVertexArray();
			std::shared_ptr<VertexBufferVulkan> vbo = mContext.createVertexBuffer(
				textVertexLayout,
				batchSizeBytes,
				VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
			);
			vao->addVertexBuffer(vbo);
			vao->setIndexBuffer(mQuadIndexBuffer, true);
			vao->getVertexBuffers()[0]->map(mContext.getLogicDevice(), batchSizeBytes);

			mMsdfRendering.SceneRenderableBatch = std::make_shared<SimpleRenderable>(vao, mFontRenderingDescSetsInstanceScene);

			mMsdfRendering.ScenePipeline = mContext.createGraphicsPipeline(*mMsdfRendering.ScenePipelineInstanceInfo);
			mMsdfRendering.ScenePipeline->init(
				mContext.getLogicDevice(),
				mContext.getSwapChain().getExtent(),
				mContext.getRenderPass(ERenderPass::APP_SCENE).get()
			);

			//Ui
			mMsdfRendering.UiBatch = std::make_unique<RenderBatchQuad>(
				EBatchSizeLimits::QUAD_BATCH_MAX_GEO_COUNT,
				EBatchSizeLimits::BATCH_MAX_COUNT_TEXTURE
			);

			mMsdfRendering.UiPipelineInstanceInfo = std::make_unique<GraphicsPipelineInstanceInfo>(
				textVertexLayout,
				*mMsdfRendering.SceneShaderProgram,
				ERenderPass::APP_UI
			);
			auto& uiOptionalFields = mMsdfRendering.UiPipelineInstanceInfo->enableOptionalFields();
			//Text isn't culled by default for viewing from behind.
			//Text depth testing in scene so it doesn't overlap, however, this causes z-fighting during kerning as each glyph is a rendered quad.
			uiOptionalFields.CullMode = VK_CULL_MODE_NONE;
			//optionalFields.setDepthTesting(true, VK_COMPARE_OP_LESS);
			uiOptionalFields.setBlending(
				true,
				VK_BLEND_OP_ADD,
				VK_BLEND_FACTOR_SRC_ALPHA,
				VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
				VK_BLEND_OP_ADD,
				VK_BLEND_FACTOR_SRC_ALPHA,
				VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA
			);

			std::shared_ptr<VertexArrayVulkan> uiVao = mContext.createVertexArray();
			std::shared_ptr<VertexBufferVulkan> uiVbo = mContext.createVertexBuffer(
				textVertexLayout,
				batchSizeBytes,
				VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
			);
			uiVao->addVertexBuffer(uiVbo);
			uiVao->setIndexBuffer(mQuadIndexBuffer, true);
			uiVao->getVertexBuffers()[0]->map(mContext.getLogicDevice(), batchSizeBytes);

			mMsdfRendering.UiRenderableBatch = std::make_shared<SimpleRenderable>(uiVao, mFontRenderingDescSetsInstanceUi);

			mMsdfRendering.UiPipeline = mContext.createGraphicsPipeline(*mMsdfRendering.UiPipelineInstanceInfo);
			mMsdfRendering.UiPipeline->init(
				mContext.getLogicDevice(),
				mContext.getSwapChain().getExtent(),
				mContext.getRenderPass(ERenderPass::APP_UI).get()
			);
		}
	}

	void TextRenderer::closeImpl() {
		ZoneScoped;

		mContext.addGpuResourceToClose(mSoftMaskRendering.SceneVertexShader);
		mContext.addGpuResourceToClose(mSoftMaskRendering.SceneFragmentShader);
		mContext.addGpuResourceToClose(mSoftMaskRendering.ScenePipeline);
		mContext.addGpuResourceToClose(mSoftMaskRendering.UiPipeline);
		mContext.addGpuResourceToClose(mSoftMaskRendering.SceneRenderableBatch->getVaoPtr());
		mContext.addGpuResourceToClose(mSoftMaskRendering.UiRenderableBatch->getVaoPtr());

		mContext.addGpuResourceToClose(mMsdfRendering.SceneVertexShader);
		mContext.addGpuResourceToClose(mMsdfRendering.SceneFragmentShader);
		mContext.addGpuResourceToClose(mMsdfRendering.ScenePipeline);
		mContext.addGpuResourceToClose(mMsdfRendering.UiPipeline);
		mContext.addGpuResourceToClose(mMsdfRendering.SceneRenderableBatch->getVaoPtr());
		mContext.addGpuResourceToClose(mMsdfRendering.UiRenderableBatch->getVaoPtr());

		mSoftMaskRendering.SceneBatch.release();
		mSoftMaskRendering.UiBatch.release();
		mMsdfRendering.SceneBatch.release();
		mMsdfRendering.UiBatch.release();

		if (!mQuadIndexBufferShared) {
			mContext.addGpuResourceToClose(mQuadIndexBuffer);
		}

		for (auto& fontBitmap : mFontBitmaps) {
			for (auto& page : fontBitmap.second->getPageTextures()) {
				mContext.addGpuResourceToClose(page);
			}
		}
	}

	void TextRenderer::onSwapChainResizeImpl(SwapChainVulkan& swapChain) {
		ZoneScoped;

		VkDevice logicDevice = mContext.getLogicDevice();
		mSoftMaskRendering.ScenePipeline->resize(
			logicDevice,
			swapChain.getExtent(),
			mContext.getRenderPass(ERenderPass::APP_SCENE).get()
		);
		mMsdfRendering.ScenePipeline->resize(
			logicDevice,
			swapChain.getExtent(),
			mContext.getRenderPass(ERenderPass::APP_SCENE).get()
		);
	}

	bool TextRenderer::createFontBitmapImpl(const char* fontName, const char* filePath, const char* imageDir, ETextRenderMethod textRenderMethod) {
		ZoneScoped;

		const auto& font = mFontBitmaps.emplace(
			fontName,
			mContext.createFontBitmap(filePath, imageDir, textRenderMethod)
		);
		if (font.second) {
			addFontBitmapToTextTextureArray(*font.first->second);
			return true;
		} else {
			LOG_ERR("Failed to store font: " << fontName);
			return false;
		}
	}

	void TextRenderer::addFontBitmapToTextTextureArrayImpl(const FontBitmap& fontBitmap) {
		ZoneScoped;

		if (mFontBitmapPagesTextureArary->hasTextureSlotsAvailable(fontBitmap.getPageCount())) {
			for (const auto& texture : fontBitmap.getPageTextures()) {
				mFontBitmapPagesTextureArary->addNewTexture(*texture);
			}
		} else {
			LOG_ERR(
				"addFontBitmapToTextTextureArray failed, not enough texture slots available in batch. Required slots: " << fontBitmap.getPageCount()
			);
		}
	}

	void TextRenderer::drawSceneImpl(uint32_t imageIndex, VkCommandBuffer cmd, CurrentBindingsState& currentBindings) {
		ZoneScoped;

		VkDevice logicDevice = mContext.getLogicDevice();
		AppDebugInfo& debugInfo = Application::get().getDebugInfo();
		const size_t softMaskQuadCount = mSoftMaskRendering.SceneBatch->getGeometryCount();
		const size_t textMsdfQuadCount = mMsdfRendering.SceneBatch->getGeometryCount();
		const StaticVertexInputLayout& textVertexLayout = StaticVertexInputLayout::get(EVertexType::VERTEX_3D_TEXTURED_INDEXED);

		if (softMaskQuadCount > 0) {
			if (currentBindings.Pipeline != mSoftMaskRendering.ScenePipeline->get()) {
				mSoftMaskRendering.ScenePipeline->bind(cmd);
				debugInfo.PipelineBinds++;
				currentBindings.Pipeline = mSoftMaskRendering.ScenePipeline->get();
			}

			if (mQuadIndexBuffer->getBuffer() != currentBindings.IndexBuffer) {
				//NOTE:: Text is currently rendered as a Quad so it can share the Quad batch index buffer
				mQuadIndexBuffer->bind(cmd);
				currentBindings.IndexBuffer = mQuadIndexBuffer->getBuffer();
				debugInfo.IndexBufferBinds++;
			}

			VertexArrayVulkan& vao = mSoftMaskRendering.SceneRenderableBatch->getVao();
			vao.getVertexBuffers()[0]->setDataMapped(
				logicDevice,
				mSoftMaskRendering.SceneBatch->getData().data(),
				softMaskQuadCount * Quad::BYTE_SIZE
			);
			vao.setDrawCount(static_cast<uint32_t>(softMaskQuadCount * EBatchSizeLimits::QUAD_INDEX_COUNT));

			mSoftMaskRendering.ScenePipeline->addRenderableToDraw(mSoftMaskRendering.SceneRenderableBatch);

			mContext.bindSceneUboToPipeline(
				cmd,
				*mSoftMaskRendering.ScenePipeline,
				imageIndex,
				currentBindings,
				debugInfo
			);

			const uint32_t drawCount = mSoftMaskRendering.ScenePipeline->getVaoDrawCount();
			mSoftMaskRendering.ScenePipeline->recordDrawCommands(cmd, currentBindings, 1);
			debugInfo.SceneDrawCalls += drawCount;

			mSoftMaskRendering.SceneBatch->reset();
		}

		//MSDF
		if (textMsdfQuadCount > 0) {
			if (currentBindings.Pipeline != mMsdfRendering.ScenePipeline->get()) {
				mMsdfRendering.ScenePipeline->bind(cmd);
				debugInfo.PipelineBinds++;
				currentBindings.Pipeline = mMsdfRendering.ScenePipeline->get();
			}

			if (mQuadIndexBuffer->getBuffer() != currentBindings.IndexBuffer) {
				//NOTE:: Text is currently rendered as a Quad so it can share the Quad batch index buffer
				mQuadIndexBuffer->bind(cmd);
				currentBindings.IndexBuffer = mQuadIndexBuffer->getBuffer();
				debugInfo.IndexBufferBinds++;
			}

			VertexArrayVulkan& vao = mMsdfRendering.SceneRenderableBatch->getVao();
			vao.getVertexBuffers()[0]->setDataMapped(
				logicDevice,
				mMsdfRendering.SceneBatch->getData().data(),
				textMsdfQuadCount * Quad::BYTE_SIZE
			);
			vao.setDrawCount(static_cast<uint32_t>(textMsdfQuadCount * EBatchSizeLimits::QUAD_INDEX_COUNT));

			mMsdfRendering.ScenePipeline->addRenderableToDraw(mMsdfRendering.SceneRenderableBatch);

			mContext.bindSceneUboToPipeline(
				cmd,
				*mMsdfRendering.ScenePipeline,
				imageIndex,
				currentBindings,
				debugInfo
			);

			const uint32_t drawCount = mMsdfRendering.ScenePipeline->getVaoDrawCount();
			mMsdfRendering.ScenePipeline->recordDrawCommands(cmd, currentBindings, 1);
			debugInfo.SceneDrawCalls += drawCount;

			mMsdfRendering.SceneBatch->reset();
		}

		//TODO:: some kind of function to reduce duplicate code?
	}

	void TextRenderer::drawUiImpl(uint32_t imageIndex, VkCommandBuffer cmd, CurrentBindingsState& currentBindings) {
		ZoneScoped;

		VkDevice logicDevice = mContext.getLogicDevice();
		AppDebugInfo& debugInfo = Application::get().getDebugInfo();
		const size_t softMaskQuadCount = mSoftMaskRendering.UiBatch->getGeometryCount();
		const size_t textMsdfQuadCount = mMsdfRendering.UiBatch->getGeometryCount();
		const StaticVertexInputLayout& textVertexLayout = StaticVertexInputLayout::get(EVertexType::VERTEX_3D_TEXTURED_INDEXED);

		if (softMaskQuadCount > 0) {
			if (currentBindings.Pipeline != mSoftMaskRendering.UiPipeline->get()) {
				mSoftMaskRendering.UiPipeline->bind(cmd);
				debugInfo.PipelineBinds++;
				currentBindings.Pipeline = mSoftMaskRendering.UiPipeline->get();
			}

			if (mQuadIndexBuffer->getBuffer() != currentBindings.IndexBuffer) {
				//NOTE:: Text is currently rendered as a Quad so it can share the Quad batch index buffer
				mQuadIndexBuffer->bind(cmd);
				currentBindings.IndexBuffer = mQuadIndexBuffer->getBuffer();
				debugInfo.IndexBufferBinds++;
			}

			VertexArrayVulkan& vao = mSoftMaskRendering.UiRenderableBatch->getVao();
			vao.getVertexBuffers()[0]->setDataMapped(
				logicDevice,
				mSoftMaskRendering.UiBatch->getData().data(),
				softMaskQuadCount * Quad::BYTE_SIZE
			);
			vao.setDrawCount(static_cast<uint32_t>(softMaskQuadCount * EBatchSizeLimits::QUAD_INDEX_COUNT));

			mSoftMaskRendering.UiPipeline->addRenderableToDraw(mSoftMaskRendering.UiRenderableBatch);

			mContext.bindUiUboToPipeline(
				cmd,
				*mSoftMaskRendering.UiPipeline,
				imageIndex,
				currentBindings,
				debugInfo
			);

			const uint32_t drawCount = mSoftMaskRendering.UiPipeline->getVaoDrawCount();
			mSoftMaskRendering.UiPipeline->recordDrawCommands(cmd, currentBindings, 1);
			debugInfo.UiDrawCalls += drawCount;

			mSoftMaskRendering.UiBatch->reset();
		}

		//MSDF
		if (textMsdfQuadCount > 0) {
			if (currentBindings.Pipeline != mMsdfRendering.UiPipeline->get()) {
				mMsdfRendering.UiPipeline->bind(cmd);
				debugInfo.PipelineBinds++;
				currentBindings.Pipeline = mMsdfRendering.UiPipeline->get();
			}

			if (mQuadIndexBuffer->getBuffer() != currentBindings.IndexBuffer) {
				//NOTE:: Text is currently rendered as a Quad so it can share the Quad batch index buffer
				mQuadIndexBuffer->bind(cmd);
				currentBindings.IndexBuffer = mQuadIndexBuffer->getBuffer();
				debugInfo.IndexBufferBinds++;
			}

			VertexArrayVulkan& vao = mMsdfRendering.UiRenderableBatch->getVao();
			vao.getVertexBuffers()[0]->setDataMapped(
				logicDevice,
				mMsdfRendering.UiBatch->getData().data(),
				textMsdfQuadCount * Quad::BYTE_SIZE
			);
			vao.setDrawCount(static_cast<uint32_t>(textMsdfQuadCount * EBatchSizeLimits::QUAD_INDEX_COUNT));

			mMsdfRendering.UiPipeline->addRenderableToDraw(mMsdfRendering.UiRenderableBatch);

			mContext.bindUiUboToPipeline(
				cmd,
				*mMsdfRendering.UiPipeline,
				imageIndex,
				currentBindings,
				debugInfo
			);

			const uint32_t drawCount = mMsdfRendering.UiPipeline->getVaoDrawCount();
			mMsdfRendering.UiPipeline->recordDrawCommands(cmd, currentBindings, 1);
			debugInfo.UiDrawCalls += drawCount;

			mMsdfRendering.UiBatch->reset();
		}

		//TODO:: some kind of function to reduce duplicate code?
	}

	void TextRenderer::drawTextFromQuadsImpl(const std::vector<Quad>& quadArr, const FontBitmap& bitmap, RenderBatchQuad& renderBatch) {
		ZoneScoped;

		size_t quadCount = quadArr.size();

		if (quadCount > 0) {
			//Only render if there is space for whole string
			//if (textBatch.hasSpace(quadCount)) {
			//	for (const Quad& quad : quadArr) {
			//		textBatch.add(
			//			quad,
			//			mFontBitmapPagesTextureArary->getTextureSlotIndex(quad.getTexture().getId())
			//		);
			//	}
			//	mDrawnQuadCount += static_cast<uint32_t>(quadCount);
			//}

			//Or

			//Add as many quads as possible (even if size limitations prevent a whole string from being added) and track additions/truncations.
			uint32_t toAddCount = static_cast<uint32_t>(std::min(quadCount, renderBatch.getRemainingGeometrySpace()));
			for (uint32_t i = 0; i < toAddCount; i++) {
				renderBatch.add(
					quadArr[i],
					mFontBitmapPagesTextureArary->getTextureSlotIndex(quadArr[i].getTexture().getId())
				);
			}
			mDrawnQuadCount += toAddCount;
			//mTruncatedQuadCount += static_cast<uint32_t>(quadCount) - toAddCount;
		}
	}

	void TextRenderer::drawTextSameTextureFromQuadsImpl(const std::vector<Quad>& quadArr, const FontBitmap& bitmap, RenderBatchQuad& renderBatch) {
		ZoneScoped;

		size_t quadCount = quadArr.size();
		if (quadCount == 0) {
			//TODO:: Is this worth a warning?
			//LOG_WARN("drawTextSameTextureFromQuads() quadArr size = 0");
			return;
		} else if (!quadArr[0].hasTexture()) {
			LOG_ERR("Quad array does not have texture");
			return;
		}

		if (renderBatch.hasSpace(quadCount)) {
			renderBatch.addAll(
				quadArr,
				mFontBitmapPagesTextureArary->getTextureSlotIndex(quadArr[0].getTexture().getId())
			);
			mDrawnQuadCount += static_cast<uint32_t>(quadCount);
		}
		//TODO:: truncated system like Renderer2d::drawQuadScene or just leave at not rendering whole array?
	}

	void TextRenderer::drawTextStringImpl(TextString& string, RenderBatchQuad& renderBatch) {
		ZoneScoped;

		if (string.getCurrentFontBitmap().getPageCount() > 1) {
			drawTextFromQuadsImpl(string.getQuads(), string.getCurrentFontBitmap(), renderBatch);
		} else {
			drawTextSameTextureFromQuadsImpl(string.getQuads(), string.getCurrentFontBitmap(), renderBatch);
		}
	}

	RenderBatchQuad& TextRenderer::getSuitableTextBatchScene(const FontBitmap& bitmap) {
		return bitmap.getTextRenderMethod() == ETextRenderMethod::SOFT_MASK ? *mSoftMaskRendering.SceneBatch : *mMsdfRendering.SceneBatch;
	}

	RenderBatchQuad& TextRenderer::getSuitableTextBatchUi(const FontBitmap& bitmap) {
		return bitmap.getTextRenderMethod() == ETextRenderMethod::SOFT_MASK ? *mSoftMaskRendering.UiBatch : *mMsdfRendering.UiBatch;
	}

	void TextRenderer::init(RenderingContextVulkan& context) {
		if (INSTANCE == nullptr) {
			INSTANCE = std::make_unique<TextRenderer>(context);
			INSTANCE->initImpl();
		} else {
			LOG_WARN("Tried to initialise alreaday initialised text renderer.");
		}
	}

	void TextRenderer::close() {
		if (INSTANCE != nullptr) {
			INSTANCE->closeImpl();
			INSTANCE.release();
			INSTANCE = nullptr;
		} else {
			LOG_WARN("Tried to close already closed text renderer.");
		}
	}

	void TextRenderer::onSwapChainResize(SwapChainVulkan& swapChain) {
		if (INSTANCE != nullptr) {
			INSTANCE->onSwapChainResizeImpl(swapChain);
		} else {
			LOG_ERR("onSwapChainResize called when text renderer is NOT initialised.");
		}
	}

	void TextRenderer::addFontBitmapToTextTextureArray(const FontBitmap& fontBitmap) {
		if (INSTANCE != nullptr) {
			INSTANCE->addFontBitmapToTextTextureArrayImpl(fontBitmap);
		} else {
			LOG_ERR("addFontBitmapToTextTextureArray called when text renderer is NOT initialised.");
		}
	}

	bool TextRenderer::hasFont(const char* fontName) {
		if (INSTANCE != nullptr) {
			return INSTANCE->mFontBitmaps.find(fontName) != INSTANCE->mFontBitmaps.end();
		} else {
			LOG_ERR("hasFont called when text renderer is NOT initialised.");
		}

		return false;
	}

	FontBitmap& TextRenderer::getFontBitmap(const char* fontName) {
		ZoneScoped;

		const auto& font = INSTANCE->mFontBitmaps.find(fontName);
		if (font != INSTANCE->mFontBitmaps.end()) {
			return *font->second;
		} else {
			LOG_WARN("Default font bitmap returned from getFontBitmap. Requested font was not found: " << fontName);
			return *INSTANCE->mFontBitmaps.find(TextRenderer::ARIAL_SOFT_MASK_NAME)->second;
		}
	}

	std::vector<DescriptorTypeInfo> TextRenderer::getEngineDescriptorTypeInfos() {
		ZoneScoped;

		std::vector<DescriptorTypeInfo> descInfoTypes;
		descInfoTypes.reserve(3);

		descInfoTypes.push_back({ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1u }); //Soft Mask Font
		descInfoTypes.push_back({ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1u }); //MSDF Font
		descInfoTypes.push_back({ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 8u }); //Font Pages Texture Array

		return descInfoTypes;
	}
}
