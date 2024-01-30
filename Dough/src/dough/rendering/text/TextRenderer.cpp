#include "dough/rendering/text/TextRenderer.h"

#include "dough/rendering/RenderingContextVulkan.h"
#include "dough/Logging.h"
#include "dough/application/Application.h"

#include "tracy/public/tracy/Tracy.hpp"

namespace DOH {

	std::unique_ptr<TextRenderer> TextRenderer::INSTANCE = nullptr;

	const std::string TextRenderer::SOFT_MASK_SCENE_SHADER_PATH_VERT = "Dough/res/shaders/spv/TextSoftMask3d.vert.spv";
	const std::string TextRenderer::SOFT_MASK_SCENE_SHADER_PATH_FRAG = "Dough/res/shaders/spv/TextSoftMask3d.frag.spv";
	const std::string TextRenderer::MSDF_SCENE_SHADER_PATH_VERT = "Dough/res/shaders/spv/TextMsdf3d.vert.spv";
	const std::string TextRenderer::MSDF_SCENE_SHADER_PATH_FRAG = "Dough/res/shaders/spv/TextMsdf3d.frag.spv";

	TextRenderer::TextRenderer(RenderingContextVulkan& context)
	:	mContext(context),
		mDrawnQuadCount(0)
	{}

	void TextRenderer::initImpl(TextureVulkan& fallbackTexture, std::shared_ptr<IndexBufferVulkan> quadSharedIndexBuffer) {
		ZoneScoped;

		mFontBitmapPagesTextureArary = std::make_unique<TextureArray>(
			EBatchSizeLimits::BATCH_MAX_COUNT_TEXTURE,
			fallbackTexture
		);

		mFontBitmaps = {};

		if (quadSharedIndexBuffer == nullptr) {
			THROW("TextRenderer::init quadSharedIndexBuffer is null");
			return;
		} else {
			mQuadSharedIndexBuffer = quadSharedIndexBuffer;
		}

		{ //Soft Mask
			bool createdDefaultFont = createFontBitmapImpl(
				TextRenderer::ARIAL_SOFT_MASK_NAME,
				"Dough/res/fonts/arial_latin_32px.fnt",
				"Dough/res/fonts/",
				ETextRenderMethod::SOFT_MASK
			);

			if (!createdDefaultFont) {
				THROW("TextRenderer::init failed to create default font bitmap");
			}

			mSoftMaskSceneShaderProgram = mContext.createShaderProgram(
				mContext.createShader(EShaderType::VERTEX, TextRenderer::SOFT_MASK_SCENE_SHADER_PATH_VERT),
				mContext.createShader(EShaderType::FRAGMENT, TextRenderer::SOFT_MASK_SCENE_SHADER_PATH_FRAG)
			);

			ShaderUniformLayout& layout = mSoftMaskSceneShaderProgram->getUniformLayout();
			layout.setValue(0, sizeof(glm::mat4x4));
			layout.setTextureArray(1, *mFontBitmapPagesTextureArary);

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

			mSoftMaskSceneRenderableBatch = std::make_shared<SimpleRenderable>(vao);

			mSoftMaskScenePipelineInstanceInfo = std::make_unique<GraphicsPipelineInstanceInfo>(
				StaticVertexInputLayout::get(QUAD_VERTEX_INPUT_TYPE),
				*mSoftMaskSceneShaderProgram,
				ERenderPass::APP_SCENE
			);

			//Text isn't culled by default for viewing from behind.
			//Text depth testing in scene so it doesn't overlap, however, this causes z-fighting during kerning as each glyph is a rendered quad.
			mSoftMaskScenePipelineInstanceInfo->getOptionalFields().CullMode = VK_CULL_MODE_NONE;
			mSoftMaskScenePipelineInstanceInfo->getOptionalFields().setDepthTesting(true, VK_COMPARE_OP_LESS);
			mSoftMaskScenePipelineInstanceInfo->getOptionalFields().setBlending(
				true,
				VK_BLEND_OP_ADD,
				VK_BLEND_FACTOR_SRC_ALPHA,
				VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
				VK_BLEND_OP_ADD,
				VK_BLEND_FACTOR_SRC_ALPHA,
				VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA
			);

			//TODO:: UI Text, since UI camera is orthographic kerning should work as z-fighting won't happen

			mSoftMaskScenePipeline = mContext.createGraphicsPipeline(
				*mSoftMaskScenePipelineInstanceInfo,
				mContext.getSwapChain().getExtent()
			);
		}

		{ //MSDF
			createFontBitmapImpl(
				TextRenderer::ARIAL_MSDF_NAME,
				"Dough/res/fonts/arial_msdf_32px_meta.json",
				"Dough/res/fonts/",
				ETextRenderMethod::MSDF
			);

			mMsdfSceneShaderProgram = mContext.createShaderProgram(
				mContext.createShader(EShaderType::VERTEX, TextRenderer::MSDF_SCENE_SHADER_PATH_VERT),
				mContext.createShader(EShaderType::FRAGMENT, TextRenderer::MSDF_SCENE_SHADER_PATH_FRAG)
			);

			ShaderUniformLayout& layout = mMsdfSceneShaderProgram->getUniformLayout();
			layout.setValue(0, sizeof(glm::mat4x4));
			layout.setTextureArray(1, *mFontBitmapPagesTextureArary);

			mMsdfSceneBatch = std::make_unique<RenderBatchQuad>(
				EBatchSizeLimits::BATCH_MAX_GEO_COUNT_QUAD,
				EBatchSizeLimits::BATCH_MAX_COUNT_TEXTURE
			);

			mMsdfScenePipelineInstanceInfo = std::make_unique<GraphicsPipelineInstanceInfo>(
				StaticVertexInputLayout::get(QUAD_VERTEX_INPUT_TYPE),
				*mMsdfSceneShaderProgram,
				ERenderPass::APP_SCENE
			);

			//Text isn't culled by default for viewing from behind.
			//Text depth testing in scene so it doesn't overlap, however, this causes z-fighting during kerning as each glyph is a rendered quad.
			mMsdfScenePipelineInstanceInfo->getOptionalFields().CullMode = VK_CULL_MODE_NONE;
			mMsdfScenePipelineInstanceInfo->getOptionalFields().setDepthTesting(true, VK_COMPARE_OP_LESS);
			mMsdfScenePipelineInstanceInfo->getOptionalFields().setBlending(
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

			mMsdfSceneRenderableBatch = std::make_shared<SimpleRenderable>(vao);

			//TODO:: UI Text, since UI camera is orthographic kerning should work as z-fighting won't happen

			mMsdfScenePipeline = mContext.createGraphicsPipeline(
				*mMsdfScenePipelineInstanceInfo,
				mContext.getSwapChain().getExtent()
			);
		}
	}

	void TextRenderer::closeImpl() {
		ZoneScoped;

		VkDevice logicDevice = mContext.getLogicDevice();

		mSoftMaskSceneShaderProgram->close(logicDevice);
		mSoftMaskScenePipeline->close(logicDevice);
		mMsdfSceneShaderProgram->close(logicDevice);
		mMsdfScenePipeline->close(logicDevice);
		mSoftMaskSceneRenderableBatch->getVao().close(logicDevice);
		mMsdfSceneRenderableBatch->getVao().close(logicDevice);

		for (auto& fontBitmap : mFontBitmaps) {
			for (auto& page : fontBitmap.second->getPageTextures()) {
				page->close(logicDevice);
			}
		}
	}

	void TextRenderer::onSwapChainResizeImpl(SwapChainVulkan& swapChain) {
		ZoneScoped;

		VkDevice logicDevice = mContext.getLogicDevice();
		mSoftMaskScenePipeline->recreate(
			logicDevice,
			swapChain.getExtent(),
			mContext.getRenderPass(ERenderPass::APP_SCENE).get()
		);
		mMsdfScenePipeline->recreate(
			logicDevice,
			swapChain.getExtent(),
			mContext.getRenderPass(ERenderPass::APP_SCENE).get()
		);
	}

	void TextRenderer::createPipelineUniformObjectsImpl(VkDescriptorPool descPool) {
		ZoneScoped;

		mContext.createPipelineUniformObjects(*mSoftMaskScenePipeline, descPool);
		mContext.createPipelineUniformObjects(*mMsdfScenePipeline, descPool);
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

	void TextRenderer::setUniformDataImpl(
		uint32_t currentImage,
		uint32_t uboBinding,
		glm::mat4x4& sceneProjView,
		glm::mat4x4& uiProjView
	) {
		ZoneScoped;

		VkDevice logicDevice = mContext.getLogicDevice();

		mSoftMaskScenePipeline->setFrameUniformData(
			logicDevice,
			currentImage,
			uboBinding,
			&sceneProjView,
			sizeof(glm::mat4x4)
		);

		mMsdfScenePipeline->setFrameUniformData(
			logicDevice,
			currentImage,
			uboBinding,
			&sceneProjView,
			sizeof(glm::mat4x4)
		);

		//TODO:: UI
	}

	void TextRenderer::drawSceneImpl(const uint32_t imageIndex, VkCommandBuffer cmd, CurrentBindingsState& currentBindings) {
		ZoneScoped;

		VkDevice logicDevice = mContext.getLogicDevice();
		const size_t softMaskQuadCount = mSoftMaskSceneBatch->getGeometryCount();
		AppDebugInfo& debugInfo = Application::get().getDebugInfo();

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

			const uint32_t drawCount = mSoftMaskScenePipeline->getVaoDrawCount();
			mSoftMaskScenePipeline->recordDrawCommands(imageIndex, cmd, currentBindings);
			debugInfo.SceneDrawCalls += drawCount;

			mSoftMaskSceneBatch->reset();
		}

		//MSDF
		const size_t textMsdfQuadCount = mMsdfSceneBatch->getGeometryCount();

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

			const uint32_t drawCount = mMsdfScenePipeline->getVaoDrawCount();
			mMsdfScenePipeline->recordDrawCommands(imageIndex, cmd, currentBindings);
			debugInfo.SceneDrawCalls += drawCount;

			mMsdfSceneBatch->reset();
		}

		//TODO:: some kind of function to reduce duplicate code?
	}

	void TextRenderer::drawUiImpl(const uint32_t imageIndex, VkCommandBuffer cmd, CurrentBindingsState& currentBindings) {
		//ZoneScoped;

		//TODO:: UI text
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

	void TextRenderer::createPipelineUniformObjects(VkDescriptorPool descPool) {
		if (INSTANCE != nullptr) {
			INSTANCE->createPipelineUniformObjectsImpl(descPool);
		} else {
			LOG_ERR("createPipelineUniform objects called when text renderer isn NOT initialised.");
		}
	}

	void TextRenderer::addFontBitmapToTextTextureArray(const FontBitmap& fontBitmap) {
		if (INSTANCE != nullptr) {
			INSTANCE->addFontBitmapToTextTextureArrayImpl(fontBitmap);
		} else {
			LOG_ERR("addFontBitmapToTextTextureArray called when text renderer is NOT initialised.");
		}
	}

	void TextRenderer::setUniformData(
		uint32_t currentImage,
		uint32_t uboBinding,
		glm::mat4x4& sceneProjView,
		glm::mat4x4& uiProjView
	) {
		//NOTE:: No nullptr check as this function is expected to be called each frame.
		INSTANCE->setUniformDataImpl(currentImage, uboBinding, sceneProjView, uiProjView);
	}

	void TextRenderer::drawScene(const uint32_t imageIndex, VkCommandBuffer cmd, CurrentBindingsState& currentBindings) {
		//NOTE:: No nullptr check as this function is expected to be called each frame.
		INSTANCE->drawSceneImpl(imageIndex, cmd, currentBindings);
	}

	void TextRenderer::drawUi(const uint32_t imageIndex, VkCommandBuffer cmd, CurrentBindingsState& currentBindings) {
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

	std::vector<DescriptorTypeInfo> TextRenderer::getDescriptorTypeInfos() {
		ZoneScoped;

		if (INSTANCE != nullptr) {
			std::vector<DescriptorTypeInfo> totalDescTypes = {};
			{
				std::vector<DescriptorTypeInfo> descTypes = INSTANCE->mSoftMaskScenePipeline->getShaderProgram().getShaderDescriptorLayout().asDescriptorTypes();
				totalDescTypes.reserve(totalDescTypes.size() + descTypes.size());	

				for (const DescriptorTypeInfo& descType : descTypes) {
					totalDescTypes.emplace_back(descType);
				}
			}
			{
				std::vector<DescriptorTypeInfo> descTypes = INSTANCE->mMsdfScenePipeline->getShaderProgram().getShaderDescriptorLayout().asDescriptorTypes();
				totalDescTypes.reserve(totalDescTypes.size() + descTypes.size());	

				for (const DescriptorTypeInfo& descType : descTypes) {
					totalDescTypes.emplace_back(descType);
				}
			}

			return totalDescTypes;
		} else {
			LOG_ERR("getDescriptorTypes called when text renderer is NOT initialised.");
		}

		return {};
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
