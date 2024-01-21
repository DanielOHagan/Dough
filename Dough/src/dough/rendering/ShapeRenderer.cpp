#include "dough/rendering/ShapeRenderer.h"

#include "dough/rendering/RenderingContextVulkan.h"
#include "dough/Logging.h"
#include "dough/rendering/ObjInit.h"
#include "dough/application/Application.h"

namespace DOH {

	std::unique_ptr<ShapeRenderer> ShapeRenderer::INSTANCE = nullptr;
	const std::string ShapeRenderer::QUAD_SHADER_PATH_VERT = "Dough/res/shaders/spv/QuadBatch.vert.spv";
	const std::string ShapeRenderer::QUAD_SHADER_PATH_FRAG = "Dough/res/shaders/spv/QuadBatch.frag.spv";

	ShapeRenderer::ShapeRenderer(RenderingContextVulkan& context)
	:	mContext(context),
		mDrawnQuadCount(0),
		mTruncatedQuadCount(0)
	{}

	void ShapeRenderer::initImpl() {
		mWhiteTexture = ObjInit::texture(
			255.0f,
			255.0f,
			255.0f,
			255.0f,
			false,
			"White Texture"
		);

		initQuadImpl();
	}

	void ShapeRenderer::initQuadImpl() {
		mQuadShaderProgram = ObjInit::shaderProgram(
			ObjInit::shader(EShaderType::VERTEX, ShapeRenderer::QUAD_SHADER_PATH_VERT),
			ObjInit::shader(EShaderType::FRAGMENT, ShapeRenderer::QUAD_SHADER_PATH_FRAG)
		);

		mQuadBatchTextureArray = std::make_unique<TextureArray>(
			EBatchSizeLimits::BATCH_MAX_COUNT_TEXTURE,
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
			StaticVertexInputLayout::get(QUAD_VERTEX_INPUT_TYPE),
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
		quadIndices.resize(EBatchSizeLimits::BATCH_QUAD_INDEX_COUNT);
		uint32_t vertexOffset = 0;
		for (uint32_t i = 0; i < EBatchSizeLimits::BATCH_QUAD_INDEX_COUNT; i += EBatchSizeLimits::SINGLE_QUAD_INDEX_COUNT) {
			quadIndices[i + 0] = vertexOffset + 0;
			quadIndices[i + 1] = vertexOffset + 1;
			quadIndices[i + 2] = vertexOffset + 2;

			quadIndices[i + 3] = vertexOffset + 2;
			quadIndices[i + 4] = vertexOffset + 3;
			quadIndices[i + 5] = vertexOffset + 0;

			vertexOffset += EBatchSizeLimits::SINGLE_QUAD_VERTEX_COUNT;
		}
		mQuadSharedIndexBuffer = ObjInit::stagedIndexBuffer(
			quadIndices.data(),
			sizeof(uint32_t) * EBatchSizeLimits::BATCH_QUAD_INDEX_COUNT
		);
	}

	void ShapeRenderer::closeImpl() {
		VkDevice logicDevice = mContext.getLogicDevice();

		mQuadShaderProgram->close(logicDevice);
		mQuadGraphicsPipeline->close(logicDevice);

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
	}

	void ShapeRenderer::onSwapChainResizeImpl(SwapChainVulkan& swapChain) {
		VkDevice logicDevice = mContext.getLogicDevice();

		//vkDestroyDescriptorPool(logicDevice, mDescriptorPool, nullptr);

		//createDescriptorPool();

		mQuadGraphicsPipeline->recreate(
			logicDevice,
			swapChain.getExtent(),
			mContext.getRenderPass(ERenderPass::APP_SCENE).get()
		);
		//mContext.createPipelineUniformObjects(*mQuadGraphicsPipeline, mDescriptorPool);
	}

	void ShapeRenderer::createPipelineUniformObjectsImpl(VkDescriptorPool descPool) {
		mContext.createPipelineUniformObjects(*mQuadGraphicsPipeline, descPool);
	}

	void ShapeRenderer::drawQuadSceneImpl(const Quad& quad) {
		bool added = false;
		for (RenderBatchQuad& batch : mQuadRenderBatches) {
			if (batch.hasSpace(1)) {
				batch.add(quad, 0);
				added = true;
				mDrawnQuadCount++;
				break;
			}
		}

		if (!added) {
			const size_t batchIndex = createNewBatchQuadImpl();
			if (batchIndex == -1) {
				LOG_ERR("Failed to add new Quad Batch");
				mTruncatedQuadCount++;
				return;
			} else {
				RenderBatchQuad& batch = mQuadRenderBatches[batchIndex];
				batch.add(quad, 0);
				mDrawnQuadCount++;
			}
		}
	}

	void ShapeRenderer::drawQuadTexturedSceneImpl(const Quad& quad) {
		bool added = false;
		TextureArray& texArr = *mQuadBatchTextureArray;

		if (!quad.hasTexture()) {
			LOG_ERR("Quad does not have texture");
			return;
		}

		for (RenderBatchQuad& batch : mQuadRenderBatches) {
			if (batch.hasSpace(1)) {
				if (texArr.hasTextureId(quad.getTexture().getId())) {
					batch.add(quad, texArr.getTextureSlotIndex(quad.getTexture().getId()));
					added = true;
					mDrawnQuadCount++;
					break;
				} else if (texArr.hasTextureSlotAvailable()) {
					const uint32_t textureSlot = texArr.addNewTexture(quad.getTexture());
					batch.add(quad, textureSlot);
					added = true;
					mDrawnQuadCount++;
					break;
				}
			}
		}

		if (!added) {
			const size_t batchIndex = createNewBatchQuadImpl();
			if (batchIndex != -1) {
				RenderBatchQuad& batch = mQuadRenderBatches[batchIndex];
				uint32_t textureSlot = 0;
				if (texArr.hasTextureId(quad.getTexture().getId())) {
					textureSlot = texArr.getTextureSlotIndex(quad.getTexture().getId());
				} else if (texArr.hasTextureSlotAvailable()) {
					textureSlot = texArr.addNewTexture(quad.getTexture());
				}
				batch.add(quad, textureSlot);
				added = true;
				mDrawnQuadCount++;
			} else {
				mTruncatedQuadCount++;
				return;
			}
		}
	}

	void ShapeRenderer::drawQuadArraySceneImpl(const std::vector<Quad>& quadArr) {
		const size_t arrSize = quadArr.size();

		uint32_t quadBatchStartIndex = 0;

		for (size_t addedCount = 0; addedCount < arrSize; /* addedCount is changed inside loop, breaks loop when no more space or all quads added */) {
			const Quad& quad = quadArr[addedCount];
			bool added = false;

			for (uint32_t batchIndex = quadBatchStartIndex; batchIndex < mQuadRenderBatches.size(); batchIndex++) {
				RenderBatchQuad& batch = mQuadRenderBatches[batchIndex];
				if (batch.hasSpace(1)) {
					batch.add(quad, 0);
					added = true;
					addedCount++;
					mDrawnQuadCount++;
					break;
				} else {
					quadBatchStartIndex++;
				}
			}

			if (!added) {
				const size_t batchIndex = createNewBatchQuadImpl();
				if (batchIndex != -1) {
					mQuadRenderBatches[batchIndex].add(quad, 0);
					addedCount++;
					mDrawnQuadCount++;
				} else {
					mTruncatedQuadCount += static_cast<uint32_t>(arrSize - addedCount);
					break;
				}
			}
		}
	}

	void ShapeRenderer::drawQuadArrayTexturedSceneImpl(const std::vector<Quad>& quadArr) {
		const size_t arrSize = quadArr.size();
		TextureArray& texArr = *mQuadBatchTextureArray;

		uint32_t quadBatchStartIndex = 0;

		for (size_t addedCount = 0; addedCount < arrSize; /* addedCount is changed inside loop, breaks loop when no more space or all quads added */) {
			const Quad& quad = quadArr[addedCount];
			bool added = false;

			const uint32_t textureSlotIndex =
				texArr.hasTextureId(quad.getTexture().getId()) ?
				texArr.getTextureSlotIndex(quad.getTexture().getId()) :
				texArr.hasTextureSlotAvailable() ?
				texArr.addNewTexture(quad.getTexture()) :
				0;

			for (uint32_t batchIndex = quadBatchStartIndex; batchIndex < mQuadRenderBatches.size(); batchIndex++) {
				RenderBatchQuad& batch = mQuadRenderBatches[batchIndex];
				if (batch.hasSpace(1)) {
					batch.add(quad, textureSlotIndex);
					addedCount++;
					added = true;
					mDrawnQuadCount++;
					break;
				} else {
					quadBatchStartIndex++;
				}
			}

			if (!added) {
				const size_t batchIndex = createNewBatchQuadImpl();
				if (batchIndex != -1) {
					mQuadRenderBatches[batchIndex].add(quad, textureSlotIndex);
					addedCount++;
					mDrawnQuadCount++;
				} else {
					mTruncatedQuadCount += static_cast<uint32_t>(arrSize - addedCount);
					break;
				}
			}
		}
	}

	void ShapeRenderer::drawQuadArraySameTextureSceneImpl(const std::vector<Quad>& quadArr) {
		size_t addedCount = 0;
		const size_t arrSize = quadArr.size();
		if (arrSize == 0) {
			//TODO:: Is this worth a warning?
			//LOG_WARN("drawQuadArraySameTextureScene() quadArr size = 0");
			return;
		} else if (!quadArr[0].hasTexture()) {
			LOG_ERR("Quad array does not have texture");
			return;
		}
		const uint32_t textureId = quadArr[0].getTexture().getId();
		TextureArray& texArr = *mQuadBatchTextureArray;

		uint32_t textureSlotIndex = 0;
		if (texArr.hasTextureId(textureId)) {
			textureSlotIndex = texArr.getTextureSlotIndex(textureId);
		} else if (texArr.hasTextureSlotAvailable()) {
			textureSlotIndex = texArr.addNewTexture(quadArr[0].getTexture());
		}

		uint32_t quadBatchStartIndex = 0;

		for (size_t addedCount = 0; addedCount < arrSize; /* addedCount is changed inside loop, breaks loop when no more space */) {
			const Quad& quad = quadArr[addedCount];

			for (uint32_t batchIndex = quadBatchStartIndex; batchIndex < mQuadRenderBatches.size(); batchIndex++) {
				RenderBatchQuad& batch = mQuadRenderBatches[batchIndex];
				const size_t remainingSpace = batch.getRemainingGeometrySpace();
				const size_t toAddCount = arrSize - addedCount;
				if (remainingSpace >= toAddCount) {
					batch.addAll(
						quadArr,
						addedCount,
						addedCount + toAddCount,
						texArr.getTextureSlotIndex(textureId)
					);
					addedCount += toAddCount;
					mDrawnQuadCount += static_cast<uint32_t>(toAddCount);
					break;
				} else if (remainingSpace > 0) {
					batch.addAll(
						quadArr,
						addedCount,
						addedCount + remainingSpace,
						texArr.getTextureSlotIndex(textureId)
					);
					addedCount += remainingSpace;
					mDrawnQuadCount += static_cast<uint32_t>(remainingSpace);
				} else {
					quadBatchStartIndex++;
				}
			}

			if (addedCount < arrSize) {
				const size_t batchIndex = createNewBatchQuadImpl();
				if (batchIndex != -1) {
					RenderBatchQuad& batch = mQuadRenderBatches[batchIndex];
					const size_t remainingSpace = batch.getRemainingGeometrySpace();
					const size_t toAddCount = arrSize - addedCount;
					if (remainingSpace >= toAddCount) {
						batch.addAll(quadArr, 0, toAddCount, textureSlotIndex);
						addedCount += toAddCount;
						mDrawnQuadCount += static_cast<uint32_t>(toAddCount);
						break;
					} else {
						batch.addAll(quadArr, addedCount, addedCount + remainingSpace, textureSlotIndex);
						addedCount += remainingSpace;
						mDrawnQuadCount += static_cast<uint32_t>(remainingSpace);
					}
				} else {
					mTruncatedQuadCount += static_cast<uint32_t>(arrSize - addedCount);
					break;
				}
			}
		}
	}

	void ShapeRenderer::setUniformDataImpl(uint32_t currentImage, uint32_t uboBinding, glm::mat4x4& sceneProjView, glm::mat4x4& uiProjView) {
		VkDevice logicDevice = mContext.getLogicDevice();

		mQuadGraphicsPipeline->setFrameUniformData(
			logicDevice,
			currentImage,
			uboBinding,
			&sceneProjView,
			sizeof(glm::mat4x4)
		);

		//TODO:: UI
	}

	void ShapeRenderer::drawSceneImpl(uint32_t imageIndex, VkCommandBuffer cmd, CurrentBindingsState& currentBindings) {
		VkDevice logicDevice = mContext.getLogicDevice();
		AppDebugInfo& debugInfo = Application::get().getDebugInfo();

		{ //Draw Quads
			bool hasQuadToDraw = false;

			uint32_t index = 0;
			for (RenderBatchQuad& batch : mQuadRenderBatches) {
				const uint32_t quadCount = static_cast<uint32_t>(batch.getGeometryCount());

				if (quadCount > 0) {
					if (mQuadGraphicsPipeline->get() != currentBindings.Pipeline) {
						mQuadGraphicsPipeline->bind(cmd);
						currentBindings.Pipeline = mQuadGraphicsPipeline->get();
						debugInfo.PipelineBinds++;
					}

					if (mQuadSharedIndexBuffer->getBuffer() != currentBindings.IndexBuffer) {
						mQuadSharedIndexBuffer->bind(cmd);
						currentBindings.IndexBuffer = mQuadSharedIndexBuffer->getBuffer();
						debugInfo.IndexBufferBinds++;
					}

					const auto& renderableBatch = mRenderableQuadBatches[index];
					VertexArrayVulkan& vao = renderableBatch->getVao();

					vao.getVertexBuffers()[0]->setDataMapped(
						logicDevice,
						batch.getData().data(),
						quadCount * EBatchSizeLimits::SINGLE_QUAD_BYTE_SIZE
					);
					vao.setDrawCount(quadCount * EBatchSizeLimits::SINGLE_QUAD_INDEX_COUNT);

					mQuadGraphicsPipeline->addRenderableToDraw(renderableBatch);
					debugInfo.SceneDrawCalls++;

					batch.reset();

					hasQuadToDraw = true;
				}

				index++;
			}

			if (hasQuadToDraw) {
				mQuadGraphicsPipeline->recordDrawCommands(imageIndex, cmd, currentBindings);
			}
		}
	}

	void ShapeRenderer::drawUiImpl(uint32_t imageIndex, VkCommandBuffer cmd, CurrentBindingsState& currentBindings) {
		//TODO:: Draw UI
	}

	size_t ShapeRenderer::createNewBatchQuadImpl() {
		if (mQuadRenderBatches.size() < EBatchSizeLimits::MAX_BATCH_COUNT_QUAD) {

			constexpr size_t batchSizeBytes = EBatchSizeLimits::BATCH_QUAD_BYTE_SIZE;

			RenderBatchQuad& batch = mQuadRenderBatches.emplace_back(
				EBatchSizeLimits::BATCH_MAX_GEO_COUNT_QUAD,
				EBatchSizeLimits::BATCH_MAX_COUNT_TEXTURE
			);

			std::shared_ptr<VertexArrayVulkan> vao = ObjInit::vertexArray();
			std::shared_ptr<VertexBufferVulkan> vbo = ObjInit::vertexBuffer(
				StaticVertexInputLayout::get(QUAD_VERTEX_INPUT_TYPE),
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

	void ShapeRenderer::closeEmptyQuadBatchesImpl() {
		//Since RenderBatches and renderables are linked through being the at the same index in their respective vector
		// and batches are filled begining to end, loop through from end to begining closing batch and renderable's vao as it goes,
		// stopping when there is a batch with a geo count of at least one

		//TODO:: A better way of "linking" batches and vaos. pair<batch , vao> ?
		const size_t batchCount = mQuadRenderBatches.size();
		if (batchCount != mRenderableQuadBatches.size()) {
			LOG_WARN("Quad batches size doesn't match vaos");
		}

		for (size_t i = batchCount; i > 0; i--) {
			if (mQuadRenderBatches[i - 1].getGeometryCount() == 0) {
				mQuadRenderBatches.pop_back();
				mContext.closeGpuResourceImmediately(mRenderableQuadBatches[i - 1]->getVao());
				mRenderableQuadBatches.pop_back();
			} else {
				break;
			}
		}
	}

	void ShapeRenderer::init(RenderingContextVulkan& context) {
		if (INSTANCE != nullptr) {
			LOG_WARN("Attempted init when ShapeRenderer is already initialised.");
		} else {
			INSTANCE = std::make_unique<ShapeRenderer>(context);
			INSTANCE->initImpl();
		}
	}

	void ShapeRenderer::initQuad() {
		if (INSTANCE != nullptr) {
			INSTANCE->initQuadImpl();
		} else {
			LOG_WARN("Attempted initQuad when ShapeRenderer is un-initialised/closed.");
		}
	}

	//void ShapeRenderer::initCircle() {}
	//void ShapeRenderer::initTriangle() {}

	void ShapeRenderer::close() {
		if (INSTANCE != nullptr) {
			INSTANCE->closeImpl();
		} else {
			LOG_WARN("Attempted close when ShapeRenderer is un-initialised/closed.");
		}
	}

	void ShapeRenderer::onSwapChainResize(SwapChainVulkan& swapChain) {
		if (INSTANCE != nullptr) {
			INSTANCE->onSwapChainResizeImpl(swapChain);
		} else {
			LOG_WARN("Attempted onSwapChainResize when ShapeRenderer is un-initialised/closed.");
		}
	}

	void ShapeRenderer::createPipelineUniformObjects(VkDescriptorPool descPool) {
		if (INSTANCE != nullptr) {
			INSTANCE->createPipelineUniformObjectsImpl(descPool);
		} else {
			LOG_WARN("Attempted createPipelineUniformObjects when ShapeRenderer is un-initialised/closed.");
		}
	}

	void ShapeRenderer::setUniformData(uint32_t currentImage, uint32_t uboBinding, glm::mat4x4& sceneProjView, glm::mat4x4& uiProjView) {
		//NOTE:: No nullptr check as this function is expected to be called each frame.
		INSTANCE->setUniformDataImpl(currentImage, uboBinding, sceneProjView, uiProjView);
	}

	void ShapeRenderer::drawScene(uint32_t imageIndex, VkCommandBuffer cmd, CurrentBindingsState& currentBindings) {
		//NOTE:: No nullptr check as this function is expected to be called each frame.
		INSTANCE->drawSceneImpl(imageIndex, cmd, currentBindings);
	}

	void ShapeRenderer::drawUi(uint32_t imageIndex, VkCommandBuffer cmd, CurrentBindingsState& currentBindings) {
		//NOTE:: No nullptr check as this function is expected to be called each frame.
		INSTANCE->drawUiImpl(imageIndex, cmd, currentBindings);
	}

	size_t ShapeRenderer::createNewBatchQuad() {
		if (INSTANCE != nullptr) {
			return INSTANCE->createNewBatchQuadImpl();
		} else {
			LOG_WARN("Attempted createNewBatchQuad when ShapeRenderer is un-initialised/closed.");
		}

		return -1;
	}

	//Close the dynamic batches that have a geo count of 0
	void ShapeRenderer::closeEmptyQuadBatches() {
		if (INSTANCE != nullptr) {
			INSTANCE->closeEmptyQuadBatchesImpl();
		} else {
			LOG_WARN("Attempted closeEmptyQuadBatches when ShapeRenderer is un-initialised/closed.");
		}
	}

	size_t ShapeRenderer::getQuadBatchCount() {
		//NOTE:: No nullptr check as this function is expected to be called each frame.
		return INSTANCE->mQuadRenderBatches.size();
	}

	uint32_t ShapeRenderer::getDrawnQuadCount() {
		//NOTE:: No nullptr check as this function is expected to be called each frame.
		return INSTANCE->mDrawnQuadCount;
	}
	uint32_t ShapeRenderer::getTruncatedQuadCount() {
		//NOTE:: No nullptr check as this function is expected to be called each frame.
		return INSTANCE->mTruncatedQuadCount;
	}
	void ShapeRenderer::resetLocalDebugInfo() {
		//NOTE:: No nullptr check as this function is expected to be called each frame.
		INSTANCE->mDrawnQuadCount = 0;
		INSTANCE->mTruncatedQuadCount = 0;
	}

	std::vector<DescriptorTypeInfo> ShapeRenderer::getDescriptorTypeInfos() {
		if (INSTANCE != nullptr) {
			std::vector<DescriptorTypeInfo> totalDescTypes = {};
			{
				std::vector<DescriptorTypeInfo> descTypes = INSTANCE->mQuadGraphicsPipeline->getShaderProgram().getShaderDescriptorLayout().asDescriptorTypes();
				totalDescTypes.reserve(totalDescTypes.size() + descTypes.size());

				for (const DescriptorTypeInfo& descType : descTypes) {
					totalDescTypes.emplace_back(descType);
				}
			}

			return totalDescTypes;
		} else {
			LOG_ERR("Attempted getDescriptorTypeInfos when ShapeRenderer is un-initialised/closed.");
		}

		return {};
	}

	void ShapeRenderer::drawQuadScene(const Quad& quad) {
		//NOTE:: No nullptr check as this function is expected to be called multiple times each frame.
		INSTANCE->drawQuadSceneImpl(quad);
	}

	void ShapeRenderer::drawQuadTexturedScene(const Quad& quad) {
		//NOTE:: No nullptr check as this function is expected to be called multiple times each frame.
		INSTANCE->drawQuadTexturedSceneImpl(quad);
	}

	void ShapeRenderer::drawQuadArrayScene(const std::vector<Quad>& quadArr) {
		//NOTE:: No nullptr check as this function is expected to be called multiple times each frame.
		INSTANCE->drawQuadArraySceneImpl(quadArr);
	}

	void ShapeRenderer::drawQuadArrayTexturedScene(const std::vector<Quad>& quadArr) {
		//NOTE:: No nullptr check as this function is expected to be called multiple times each frame.
		INSTANCE->drawQuadArrayTexturedSceneImpl(quadArr);
	}

	void ShapeRenderer::drawQuadArraySameTextureScene(const std::vector<Quad>& quadArr) {
		//NOTE:: No nullptr check as this function is expected to be called multiple times each frame.
		INSTANCE->drawQuadArraySameTextureSceneImpl(quadArr);
	}

	//void ShapeRenderer::drawQuadUi(Quad& quad) {}
	//void ShapeRenderer::drawQuadTexturedUi(const Quad& quad) {}
	//void ShapeRenderer::drawQuadArrayUi(const std::vector<Quad>& quadArr) {}
	//void ShapeRenderer::drawQuadArrayTexturedUi(const std::vector<Quad>& quadArr) {}
	//void ShapeRenderer::drawQuadArraySameTextureUi(const std::vector<Quad>& quadArr) {}
}
