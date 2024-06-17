#include "dough/rendering/text/TextRenderer.h"

#include "dough/rendering/RenderingContextVulkan.h"
#include "dough/Logging.h"
#include "dough/application/Application.h"
#include "dough/rendering/ShapeRenderer.h"

#include "tracy/public/tracy/Tracy.hpp"

namespace DOH {

	std::unique_ptr<TextRenderer> TextRenderer::INSTANCE = nullptr;

	const char* TextRenderer::SOFT_MASK_SCENE_SHADER_PATH_VERT = "Dough/res/shaders/spv/TextSoftMask3d.vert.spv";
	const char* TextRenderer::SOFT_MASK_SCENE_SHADER_PATH_FRAG = "Dough/res/shaders/spv/TextSoftMask3d.frag.spv";
	const char* TextRenderer::MSDF_SCENE_SHADER_PATH_VERT = "Dough/res/shaders/spv/TextMsdf3d.vert.spv";
	const char* TextRenderer::MSDF_SCENE_SHADER_PATH_FRAG = "Dough/res/shaders/spv/TextMsdf3d.frag.spv";

	TextRenderer::TextRenderer(RenderingContextVulkan& context)
	:	mContext(context),
		mFontBitmapPagesDescSet(VK_NULL_HANDLE),
		mDrawnQuadCount(0)
	{}

	void TextRenderer::initImpl(TextureVulkan& fallbackTexture, std::shared_ptr<IndexBufferVulkan> quadSharedIndexBuffer) {
		ZoneScoped;

		mFontBitmapPagesTextureArary = std::make_unique<TextureArray>(
			EBatchSizeLimits::BATCH_MAX_COUNT_TEXTURE,
			fallbackTexture
		);

		mFontBitmaps = {};

		if (ShapeRenderer::getQuadSharedIndexBufferPtr() == nullptr) {
			THROW("TextRenderer::init QuadSharedIndexBuffer is null but is required for TextRenderer.");
			return;
		} else {
			mQuadSharedIndexBuffer = quadSharedIndexBuffer;
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

		{ //Load fonts
			bool createdDefaultFont = createFontBitmapImpl(
				TextRenderer::ARIAL_SOFT_MASK_NAME,
				"Dough/res/fonts/arial_latin_32px.fnt",
				"Dough/res/fonts/",
				ETextRenderMethod::SOFT_MASK
			);

			if (!createdDefaultFont) {
				THROW("TextRenderer::init failed to create default font bitmap");
			}

			bool createdMsdfFont = createFontBitmapImpl(
				TextRenderer::ARIAL_MSDF_NAME,
				"Dough/res/fonts/arial_msdf_32px_meta.json",
				"Dough/res/fonts/",
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
			mSoftMaskSceneVertexShader = mContext.createShader(EShaderStage::VERTEX, TextRenderer::SOFT_MASK_SCENE_SHADER_PATH_VERT);
			mSoftMaskSceneFragmentShader = mContext.createShader(EShaderStage::FRAGMENT, TextRenderer::SOFT_MASK_SCENE_SHADER_PATH_FRAG);
			mSoftMaskSceneShaderProgram = mContext.createShaderProgram(
				mSoftMaskSceneVertexShader,
				mSoftMaskSceneFragmentShader,
				textDescSetLayouts
			);

			mSoftMaskSceneBatch = std::make_unique<RenderBatchQuad>(
				EBatchSizeLimits::BATCH_MAX_GEO_COUNT_QUAD,
				EBatchSizeLimits::BATCH_MAX_COUNT_TEXTURE
			);

			constexpr size_t batchSizeBytes = EBatchSizeLimits::BATCH_QUAD_BYTE_SIZE;

			std::shared_ptr<VertexArrayVulkan> vao = mContext.createVertexArray();
			std::shared_ptr<VertexBufferVulkan> vbo = mContext.createVertexBuffer(
				StaticVertexInputLayout::get(QUAD_VERTEX_INPUT_TYPE),
				batchSizeBytes,
				VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
			);
			vao->addVertexBuffer(vbo);
			vao->setIndexBuffer(quadSharedIndexBuffer, true);
			vao->getVertexBuffers()[0]->map(mContext.getLogicDevice(), batchSizeBytes);

			mSoftMaskSceneRenderableBatch = std::make_shared<SimpleRenderable>(vao, mFontRenderingDescSetsInstanceScene);

			mSoftMaskScenePipelineInstanceInfo = std::make_unique<GraphicsPipelineInstanceInfo>(
				StaticVertexInputLayout::get(QUAD_VERTEX_INPUT_TYPE),
				*mSoftMaskSceneShaderProgram,
				ERenderPass::APP_SCENE
			);
			auto& optionalFields = mSoftMaskScenePipelineInstanceInfo->enableOptionalFields();
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

			//TODO:: UI Text, since UI camera is orthographic kerning should work as z-fighting won't happen

			mSoftMaskScenePipeline = mContext.createGraphicsPipeline(*mSoftMaskScenePipelineInstanceInfo);
			mSoftMaskScenePipeline->init(
				mContext.getLogicDevice(),
				mContext.getSwapChain().getExtent(),
				mContext.getRenderPass(ERenderPass::APP_SCENE).get()
			);
		}

		{ //MSDF
			mMsdfSceneVertexShader = mContext.createShader(EShaderStage::VERTEX, TextRenderer::MSDF_SCENE_SHADER_PATH_VERT);
			mMsdfSceneFragmentShader = mContext.createShader(EShaderStage::FRAGMENT, TextRenderer::MSDF_SCENE_SHADER_PATH_FRAG);
			mMsdfSceneShaderProgram = mContext.createShaderProgram(
				mMsdfSceneVertexShader,
				mMsdfSceneFragmentShader,
				textDescSetLayouts
			);

			mMsdfSceneBatch = std::make_unique<RenderBatchQuad>(
				EBatchSizeLimits::BATCH_MAX_GEO_COUNT_QUAD,
				EBatchSizeLimits::BATCH_MAX_COUNT_TEXTURE
			);

			mMsdfScenePipelineInstanceInfo = std::make_unique<GraphicsPipelineInstanceInfo>(
				StaticVertexInputLayout::get(QUAD_VERTEX_INPUT_TYPE),
				*mMsdfSceneShaderProgram,
				ERenderPass::APP_SCENE
			);
			auto& optionalFields = mMsdfScenePipelineInstanceInfo->enableOptionalFields();
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

			constexpr size_t batchSizeBytes = EBatchSizeLimits::BATCH_QUAD_BYTE_SIZE;

			std::shared_ptr<VertexArrayVulkan> vao = mContext.createVertexArray();
			std::shared_ptr<VertexBufferVulkan> vbo = mContext.createVertexBuffer(
				StaticVertexInputLayout::get(QUAD_VERTEX_INPUT_TYPE),
				batchSizeBytes,
				VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
			);
			vao->addVertexBuffer(vbo);
			vao->setIndexBuffer(quadSharedIndexBuffer, true);
			vao->getVertexBuffers()[0]->map(mContext.getLogicDevice(), batchSizeBytes);

			mMsdfSceneRenderableBatch = std::make_shared<SimpleRenderable>(vao, mFontRenderingDescSetsInstanceScene);

			mMsdfScenePipeline = mContext.createGraphicsPipeline(*mMsdfScenePipelineInstanceInfo);
			mMsdfScenePipeline->init(
				mContext.getLogicDevice(),
				mContext.getSwapChain().getExtent(),
				mContext.getRenderPass(ERenderPass::APP_SCENE).get()
			);

			//TODO:: UI Text, since UI camera is orthographic kerning should work as z-fighting won't happen
		}
	}

	void TextRenderer::closeImpl() {
		ZoneScoped;

		VkDevice logicDevice = mContext.getLogicDevice();

		mContext.addGpuResourceToClose(mSoftMaskSceneVertexShader);
		mContext.addGpuResourceToClose(mSoftMaskSceneFragmentShader);
		mContext.addGpuResourceToClose(mSoftMaskScenePipeline);
		mContext.addGpuResourceToClose(mMsdfSceneVertexShader);
		mContext.addGpuResourceToClose(mMsdfSceneFragmentShader);
		mContext.addGpuResourceToClose(mMsdfScenePipeline);
		mContext.addGpuResourceToClose(mSoftMaskSceneRenderableBatch->getVaoPtr());
		mContext.addGpuResourceToClose(mMsdfSceneRenderableBatch->getVaoPtr());

		for (auto& fontBitmap : mFontBitmaps) {
			for (auto& page : fontBitmap.second->getPageTextures()) {
				mContext.addGpuResourceToClose(page);
			}
		}
	}

	void TextRenderer::onSwapChainResizeImpl(SwapChainVulkan& swapChain) {
		ZoneScoped;

		VkDevice logicDevice = mContext.getLogicDevice();
		mSoftMaskScenePipeline->resize(
			logicDevice,
			swapChain.getExtent(),
			mContext.getRenderPass(ERenderPass::APP_SCENE).get()
		);
		mMsdfScenePipeline->resize(
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
		const size_t softMaskQuadCount = mSoftMaskSceneBatch->getGeometryCount();
		const size_t textMsdfQuadCount = mMsdfSceneBatch->getGeometryCount();

		if (softMaskQuadCount > 0) {
			if (currentBindings.Pipeline != mSoftMaskScenePipeline->get()) {
				mSoftMaskScenePipeline->bind(cmd);
				debugInfo.PipelineBinds++;
				currentBindings.Pipeline = mSoftMaskScenePipeline->get();
			}

			if (mQuadSharedIndexBuffer->getBuffer() != currentBindings.IndexBuffer) {
				//NOTE:: Text is currently rendered as a Quad so it can share the Quad batch index buffer
				mQuadSharedIndexBuffer->bind(cmd);
				currentBindings.IndexBuffer = mQuadSharedIndexBuffer->getBuffer();
				debugInfo.IndexBufferBinds++;
			}

			VertexArrayVulkan& vao = mSoftMaskSceneRenderableBatch->getVao();
			vao.getVertexBuffers()[0]->setDataMapped(
				logicDevice,
				mSoftMaskSceneBatch->getData().data(),
				softMaskQuadCount * EBatchSizeLimits::SINGLE_QUAD_BYTE_SIZE
			);
			vao.setDrawCount(static_cast<uint32_t>(softMaskQuadCount * EBatchSizeLimits::SINGLE_QUAD_INDEX_COUNT));

			mSoftMaskScenePipeline->addRenderableToDraw(mSoftMaskSceneRenderableBatch);

			mContext.bindSceneUboToPipeline(
				cmd,
				*mSoftMaskScenePipeline,
				imageIndex,
				currentBindings,
				debugInfo
			);

			const uint32_t drawCount = mSoftMaskScenePipeline->getVaoDrawCount();
			mSoftMaskScenePipeline->recordDrawCommands(cmd, currentBindings, 1);
			debugInfo.SceneDrawCalls += drawCount;

			mSoftMaskSceneBatch->reset();
		}

		//MSDF
		if (textMsdfQuadCount > 0) {
			if (currentBindings.Pipeline != mMsdfScenePipeline->get()) {
				mMsdfScenePipeline->bind(cmd);
				debugInfo.PipelineBinds++;
				currentBindings.Pipeline = mMsdfScenePipeline->get();
			}

			if (mQuadSharedIndexBuffer->getBuffer() != currentBindings.IndexBuffer) {
				//NOTE:: Text is currently rendered as a Quad so it can share the Quad batch index buffer
				mQuadSharedIndexBuffer->bind(cmd);
				currentBindings.IndexBuffer = mQuadSharedIndexBuffer->getBuffer();
				debugInfo.IndexBufferBinds++;
			}

			VertexArrayVulkan& vao = mMsdfSceneRenderableBatch->getVao();
			vao.getVertexBuffers()[0]->setDataMapped(
				logicDevice,
				mMsdfSceneBatch->getData().data(),
				textMsdfQuadCount * EBatchSizeLimits::SINGLE_QUAD_BYTE_SIZE
			);
			vao.setDrawCount(static_cast<uint32_t>(textMsdfQuadCount * EBatchSizeLimits::SINGLE_QUAD_INDEX_COUNT));

			mMsdfScenePipeline->addRenderableToDraw(mMsdfSceneRenderableBatch);

			mContext.bindSceneUboToPipeline(
				cmd,
				*mMsdfScenePipeline,
				imageIndex,
				currentBindings,
				debugInfo
			);

			const uint32_t drawCount = mMsdfScenePipeline->getVaoDrawCount();
			mMsdfScenePipeline->recordDrawCommands(cmd, currentBindings, 1);
			debugInfo.SceneDrawCalls += drawCount;

			mMsdfSceneBatch->reset();
		}

		//TODO:: some kind of function to reduce duplicate code?
	}

	void TextRenderer::drawUiImpl(uint32_t imageIndex, VkCommandBuffer cmd, CurrentBindingsState& currentBindings) {
		//ZoneScoped;

		//TODO:: UI text

		//mContext.bindUiUboToPipeline(cmd, , imageIndex, currentBindings, Application::get().getDebugInfo());
	}

	void TextRenderer::drawTextFromQuadsImpl(const std::vector<Quad>& quadArr, const FontBitmap& bitmap) {
		ZoneScoped;

		size_t quadCount = quadArr.size();

		if (quadCount > 0) {
			RenderBatchQuad& textBatch = bitmap.getTextRenderMethod() == ETextRenderMethod::MSDF ?
				*mMsdfSceneBatch : *mSoftMaskSceneBatch;

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
			uint32_t toAddCount = static_cast<uint32_t>(std::min(quadCount, textBatch.getRemainingGeometrySpace()));
			for (uint32_t i = 0; i < toAddCount; i++) {
				textBatch.add(
					quadArr[i],
					mFontBitmapPagesTextureArary->getTextureSlotIndex(quadArr[i].getTexture().getId())
				);
			}
			mDrawnQuadCount += toAddCount;
			//mTruncatedQuadCount += static_cast<uint32_t>(quadCount) - toAddCount;
		}
	}

	void TextRenderer::drawTextSameTextureFromQuadsImpl(const std::vector<Quad>& quadArr, const FontBitmap& bitmap) {
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

		RenderBatchQuad& textBatch = bitmap.getTextRenderMethod() == ETextRenderMethod::MSDF ?
			*mMsdfSceneBatch : *mSoftMaskSceneBatch;

		if (textBatch.hasSpace(quadCount)) {
			textBatch.addAll(
				quadArr,
				mFontBitmapPagesTextureArary->getTextureSlotIndex(quadArr[0].getTexture().getId())
			);
			mDrawnQuadCount += static_cast<uint32_t>(quadCount);
		}
		//TODO:: truncated system like Renderer2d::drawQuadScene or just leave at not rendering whole array?
	}

	void TextRenderer::drawTextStringImpl(TextString& string) {
		ZoneScoped;

		if (string.getCurrentFontBitmap().getPageCount() > 1) {
			drawTextFromQuadsImpl(string.getQuads(), string.getCurrentFontBitmap());
		} else {
			drawTextSameTextureFromQuadsImpl(string.getQuads(), string.getCurrentFontBitmap());
		}
	}

	void TextRenderer::init(RenderingContextVulkan& context, TextureVulkan& fallbackTexture, std::shared_ptr<IndexBufferVulkan> quadSharedIndexBuffer) {
		if (INSTANCE == nullptr) {
			INSTANCE = std::make_unique<TextRenderer>(context);
			INSTANCE->initImpl(fallbackTexture, quadSharedIndexBuffer);
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

	void TextRenderer::drawScene(uint32_t imageIndex, VkCommandBuffer cmd, CurrentBindingsState& currentBindings) {
		//NOTE:: No nullptr check as this function is expected to be called each frame.
		INSTANCE->drawSceneImpl(imageIndex, cmd, currentBindings);
	}

	void TextRenderer::drawUi(uint32_t imageIndex, VkCommandBuffer cmd, CurrentBindingsState& currentBindings) {
		//NOTE:: No nullptr check as this function is expected to be called each frame.
		INSTANCE->drawUiImpl(imageIndex, cmd, currentBindings);
	}

	uint32_t TextRenderer::getDrawnQuadCount() {
		//NOTE:: No nullptr check as this function is expected to be called each frame.
		return INSTANCE->mDrawnQuadCount;
	}

	void TextRenderer::resetLocalDebugInfo() {
		//NOTE:: No nullptr check as this function is expected to be called each frame.
		INSTANCE->mDrawnQuadCount = 0;
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

	void TextRenderer::drawTextFromQuads(const std::vector<Quad>& quadArr, const FontBitmap& bitmap) {
		//NOTE:: No nullptr check as this function is expected to be called multiple times each frame.
		INSTANCE->drawTextFromQuadsImpl(quadArr, bitmap);
	}

	void TextRenderer::drawTextSameTextureFromQuads(const std::vector<Quad>& quadArr, const FontBitmap& bitmap) {
		//NOTE:: No nullptr check as this function is expected to be called multiple times each frame.
		INSTANCE->drawTextSameTextureFromQuadsImpl(quadArr, bitmap);
	}

	void TextRenderer::drawTextString(TextString& string) {
		//NOTE:: No nullptr check as this function is expected to be called multiple times each frame.
		INSTANCE->drawTextStringImpl(string);
	}
}
