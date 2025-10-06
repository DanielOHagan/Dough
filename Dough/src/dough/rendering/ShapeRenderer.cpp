#include "dough/rendering/ShapeRenderer.h"

#include "dough/rendering/RenderingContextVulkan.h"
#include "dough/Logging.h"
#include "dough/application/Application.h"

#include <tracy/public/tracy/Tracy.hpp>

namespace DOH {

	std::unique_ptr<ShapeRenderer> ShapeRenderer::INSTANCE = nullptr;
	const char* ShapeRenderer::QUAD_SHADER_PATH_VERT = "Dough/Dough/res/shaders/spv/QuadBatch.vert.spv";
	const char* ShapeRenderer::QUAD_SHADER_PATH_FRAG = "Dough/Dough/res/shaders/spv/QuadBatch.frag.spv";
	const char* ShapeRenderer::CIRCLE_SHADER_PATH_VERT = "Dough/Dough/res/shaders/spv/CircleBatch.vert.spv";
	const char* ShapeRenderer::CIRCLE_SHADER_PATH_FRAG = "Dough/Dough/res/shaders/spv/CircleBatch.frag.spv";

	ShapeRenderer::ShapeRenderer(RenderingContextVulkan& context)
	:	mContext(context),
		mDrawnQuadCount(0u),
		mTruncatedQuadCount(0u),
		mDrawnCircleCount(0u),
		mTruncatedCircleCount(0u),
		mTextureArrayDescSet(VK_NULL_HANDLE),
		mWarnOnNullSceneCameraData(true),
		mWarnOnNullUiCameraData(true)
	{}

	void ShapeRenderer::initImpl() {
		ZoneScoped;

		{ // Texture Array
			mTextureArray = std::make_unique<TextureArray>(
				EBatchSizeLimits::BATCH_MAX_COUNT_TEXTURE,
				*mContext.getResourceDefaults().WhiteTexture
			);

			//Add blacnk white texture for quads that aren't using a texture
			mTextureArray->addNewTexture(*mContext.getResourceDefaults().WhiteTexture);

			mTestMonoSpaceTextureAtlas = mContext.createMonoSpaceTextureAtlas(
				"Dough/Dough/res/images/test textures/texturesAtlas.png",
				5,
				5
			);
			mTestIndexedTextureAtlas = mContext.createIndexedTextureAtlas(
				"Dough/Dough/res/images/textureAtlasses/indexed_testAtlas.txt",
				"Dough/Dough/res/images/textureAtlasses/"
			);

			mTextureArray->addNewTexture(*mTestMonoSpaceTextureAtlas);
			mTextureArray->addNewTexture(*mTestIndexedTextureAtlas);
		}

		std::vector<std::reference_wrapper<DescriptorSetLayoutVulkan>> shapeDescSets = {
			mContext.getCommonDescriptorSetLayouts().Ubo,
			mContext.getCommonDescriptorSetLayouts().SingleTextureArray8
		};
		mShapeDescSetLayouts = std::make_shared<ShaderDescriptorSetLayoutsVulkan>(shapeDescSets);

		DescriptorSetLayoutVulkan& texArrSetLayout = mContext.getCommonDescriptorSetLayouts().SingleTextureArray8;
		mTextureArrayDescSet = DescriptorApiVulkan::allocateDescriptorSetFromLayout(
			mContext.getLogicDevice(),
			mContext.getEngineDescriptorPool(),
			texArrSetLayout
		);
		const uint32_t texArrBinding = 0;
		DescriptorSetUpdate texArrUpdate = {
			{{ texArrSetLayout.getDescriptors()[texArrBinding], *mTextureArray }},
			mTextureArrayDescSet
		};
		DescriptorApiVulkan::updateDescriptorSet(mContext.getLogicDevice(), texArrUpdate);

		const uint32_t descSetCount = 2;
		mShapesDescSetsInstanceScene = std::make_shared<DescriptorSetsInstanceVulkan>(descSetCount);
		mShapesDescSetsInstanceScene->setDescriptorSetArray(CAMERA_UBO_SLOT, { VK_NULL_HANDLE , VK_NULL_HANDLE });
		mShapesDescSetsInstanceScene->setDescriptorSetSingle(1, mTextureArrayDescSet);
		mShapesDescSetsInstanceUi = std::make_shared<DescriptorSetsInstanceVulkan>(descSetCount);
		mShapesDescSetsInstanceUi->setDescriptorSetArray(CAMERA_UBO_SLOT, { VK_NULL_HANDLE , VK_NULL_HANDLE });
		mShapesDescSetsInstanceUi->setDescriptorSetSingle(1, mTextureArrayDescSet);

		initQuad();
		initCircle();
		//initTriangle();
	}

	void ShapeRenderer::initQuad() {
		ZoneScoped;

		{ //Scene
			mQuadScene = { EShape::QUAD };

			mQuadScene.VertexShader = mContext.createShader(EShaderStage::VERTEX, ShapeRenderer::QUAD_SHADER_PATH_VERT);
			mQuadScene.FragmentShader = mContext.createShader(EShaderStage::FRAGMENT, ShapeRenderer::QUAD_SHADER_PATH_FRAG);
			mQuadScene.Program = mContext.createShaderProgram(
				mQuadScene.VertexShader,
				mQuadScene.FragmentShader,
				mShapeDescSetLayouts
			);

			mQuadScene.PipelineInstanceInfo = std::make_unique<GraphicsPipelineInstanceInfo>(
				StaticVertexInputLayout::get(QUAD_VERTEX_INPUT_TYPE),
				*mQuadScene.Program,
				ERenderPass::APP_SCENE
			);
			auto& optionalFields = mQuadScene.PipelineInstanceInfo->enableOptionalFields();
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
			optionalFields.ClearRenderablesAfterDraw = false;

			mQuadScene.Pipeline = mContext.createGraphicsPipeline(*mQuadScene.PipelineInstanceInfo);
			mQuadScene.Pipeline->init(
				mContext.getLogicDevice(),
				mContext.getSwapChain().getExtent(),
				mContext.getRenderPass(ERenderPass::APP_SCENE).get()
			);
			mQuadScene.DescriptorSetsInstance = mShapesDescSetsInstanceScene;
		}

		{ //UI
			mQuadUi = { EShape::QUAD };

			mQuadUi.VertexShader = mContext.createShader(EShaderStage::VERTEX, ShapeRenderer::QUAD_SHADER_PATH_VERT);
			mQuadUi.FragmentShader = mContext.createShader(EShaderStage::FRAGMENT, ShapeRenderer::QUAD_SHADER_PATH_FRAG);
			mQuadUi.Program = mContext.createShaderProgram(
				mQuadScene.VertexShader,
				mQuadScene.FragmentShader,
				mShapeDescSetLayouts
			);

			mQuadUi.PipelineInstanceInfo = std::make_unique<GraphicsPipelineInstanceInfo>(
				StaticVertexInputLayout::get(QUAD_VERTEX_INPUT_TYPE),
				*mQuadUi.Program,
				ERenderPass::APP_UI
			);
			auto& optionalFieldsUi = mQuadUi.PipelineInstanceInfo->enableOptionalFields();
			optionalFieldsUi.setBlending(
				true,
				VK_BLEND_OP_ADD,
				VK_BLEND_FACTOR_SRC_ALPHA,
				VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
				VK_BLEND_OP_ADD,
				VK_BLEND_FACTOR_SRC_ALPHA,
				VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA
			);
			optionalFieldsUi.ClearRenderablesAfterDraw = false;

			mQuadUi.Pipeline = mContext.createGraphicsPipeline(*mQuadUi.PipelineInstanceInfo);
			mQuadUi.Pipeline->init(
				mContext.getLogicDevice(),
				mContext.getSwapChain().getExtent(),
				mContext.getRenderPass(ERenderPass::APP_UI).get()
			);
			mQuadUi.DescriptorSetsInstance = mShapesDescSetsInstanceUi;
		}

		//Quad Index Buffer
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
		mQuadSharedIndexBuffer = mContext.createStagedIndexBuffer(
			quadIndices.data(),
			sizeof(uint32_t) * EBatchSizeLimits::QUAD_BATCH_INDEX_COUNT
		);
	}

	void ShapeRenderer::initCircle() {
		{ //Scene
			mCircleScene = { EShape::CIRCLE };
			mCircleScene.VertexShader = mContext.createShader(EShaderStage::VERTEX, ShapeRenderer::CIRCLE_SHADER_PATH_VERT);
			mCircleScene.FragmentShader = mContext.createShader(EShaderStage::FRAGMENT, ShapeRenderer::CIRCLE_SHADER_PATH_FRAG);
			mCircleScene.Program = mContext.createShaderProgram(
				mCircleScene.VertexShader,
				mCircleScene.FragmentShader,
				mShapeDescSetLayouts
			);
			mCircleScene.PipelineInstanceInfo = std::make_unique<GraphicsPipelineInstanceInfo>(
				StaticVertexInputLayout::get(CIRCLE_VERTEX_INPUT_TYPE),
				*mCircleScene.Program,
				ERenderPass::APP_SCENE
			);
			auto& optionalFieldsScene = mCircleScene.PipelineInstanceInfo->enableOptionalFields();
			optionalFieldsScene.setDepthTesting(true, VK_COMPARE_OP_LESS);
			optionalFieldsScene.setBlending(
				true,
				VK_BLEND_OP_ADD,
				VK_BLEND_FACTOR_SRC_ALPHA,
				VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
				VK_BLEND_OP_ADD,
				VK_BLEND_FACTOR_SRC_ALPHA,
				VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA
			);
			optionalFieldsScene.ClearRenderablesAfterDraw = false;

			mCircleScene.Pipeline = mContext.createGraphicsPipeline(*mCircleScene.PipelineInstanceInfo);
			mCircleScene.Pipeline->init(
				mContext.getLogicDevice(),
				mContext.getSwapChain().getExtent(),
				mContext.getRenderPassScene().get()
			);
			mCircleScene.DescriptorSetsInstance = mShapesDescSetsInstanceScene;
		}

		{ //UI
			mCircleUi = { EShape::CIRCLE };
			mCircleUi.VertexShader = mContext.createShader(EShaderStage::VERTEX, ShapeRenderer::CIRCLE_SHADER_PATH_VERT);
			mCircleUi.FragmentShader = mContext.createShader(EShaderStage::FRAGMENT, ShapeRenderer::CIRCLE_SHADER_PATH_FRAG);
			mCircleUi.Program = mContext.createShaderProgram(
				mCircleUi.VertexShader,
				mCircleUi.FragmentShader,
				mShapeDescSetLayouts
			);
			mCircleUi.PipelineInstanceInfo = std::make_unique<GraphicsPipelineInstanceInfo>(
				StaticVertexInputLayout::get(CIRCLE_VERTEX_INPUT_TYPE),
				*mCircleUi.Program,
				ERenderPass::APP_UI
			);
			auto& optionalFieldsUi = mCircleUi.PipelineInstanceInfo->enableOptionalFields();
			optionalFieldsUi.setBlending(
				true,
				VK_BLEND_OP_ADD,
				VK_BLEND_FACTOR_SRC_ALPHA,
				VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
				VK_BLEND_OP_ADD,
				VK_BLEND_FACTOR_SRC_ALPHA,
				VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA
			);
			optionalFieldsUi.ClearRenderablesAfterDraw = false;
			mCircleUi.Pipeline = mContext.createGraphicsPipeline(*mCircleUi.PipelineInstanceInfo);
			mCircleUi.Pipeline->init(
				mContext.getLogicDevice(),
				mContext.getSwapChain().getExtent(),
				mContext.getRenderPassUi().get()
			);
			mCircleUi.DescriptorSetsInstance = mShapesDescSetsInstanceUi;
		}
	}

	void ShapeRenderer::closeImpl() {
		ZoneScoped;

		mQuadScene.addOwnedResourcesToClose(mContext);
		mQuadUi.addOwnedResourcesToClose(mContext);
		mCircleScene.addOwnedResourcesToClose(mContext);
		mCircleUi.addOwnedResourcesToClose(mContext);
		mContext.addGpuResourceToClose(mTestMonoSpaceTextureAtlas);
		mContext.addGpuResourceToClose(mTestIndexedTextureAtlas);
		mContext.addGpuResourceToClose(mQuadSharedIndexBuffer);
	}

	void ShapeRenderer::onSwapChainResizeImpl(SwapChainVulkan& swapChain) {
		ZoneScoped;

		mQuadScene.Pipeline->resize(
			mContext.getLogicDevice(),
			swapChain.getExtent(),
			mContext.getRenderPassScene().get()
		);
		mQuadUi.Pipeline->resize(
			mContext.getLogicDevice(),
			swapChain.getExtent(),
			mContext.getRenderPassUi().get()
		);
		mCircleScene.Pipeline->resize(
			mContext.getLogicDevice(),
			swapChain.getExtent(),
			mContext.getRenderPassScene().get()
		);
		mCircleUi.Pipeline->resize(
			mContext.getLogicDevice(),
			swapChain.getExtent(),
			mContext.getRenderPassUi().get()
		);
	}

	void ShapeRenderer::drawQuad(ShapeRenderingObjects<RenderBatchQuad>& quadGroup, const Quad& quad) {
		ZoneScoped;

		bool added = false;
		for (uint32_t i = 0; i < quadGroup.getBatchCount(); i++) {
			RenderBatchQuad& batch = *quadGroup.GeoBatches[i];
			if (batch.hasSpace(1)) {
				batch.add(quad, 0);
				added = true;
				mDrawnQuadCount++;
				break;
			}
		}

		if (!added) {
			if (quadGroup.getBatchCount() == EBatchSizeLimits::QUAD_MAX_BATCH_COUNT) {
				mTruncatedQuadCount++;
				return;
			} else {
				const size_t batchIndex = createNewBatchQuad(quadGroup);
				if (batchIndex == -1) {
					LOG_ERR("Failed to add new Quad Batch");
					mTruncatedQuadCount++;
					return;
				} else {
					quadGroup.GeoBatches[batchIndex]->add(quad, 0);
					mDrawnQuadCount++;
				}
			}
		}
	}

	void ShapeRenderer::drawQuadTextured(ShapeRenderingObjects<RenderBatchQuad>& quadGroup, const Quad& quad) {
		ZoneScoped;

		bool added = false;

		if (!quad.hasTexture()) {
			LOG_ERR("Quad does not have texture");
			return;
		}

		for (uint32_t i = 0; i < quadGroup.getBatchCount(); i++) {
			RenderBatchQuad& batch = *quadGroup.GeoBatches[i];
			if (batch.hasSpace(1)) {
				if (mTextureArray->hasTextureId(quad.getTexture().getId())) {
					batch.add(quad, mTextureArray->getTextureSlotIndex(quad.getTexture().getId()));
					added = true;
					mDrawnQuadCount++;
					break;
				} else if (mTextureArray->hasTextureSlotAvailable()) {
					const uint32_t textureSlot = mTextureArray->addNewTexture(quad.getTexture());
					batch.add(quad, textureSlot);
					added = true;
					mDrawnQuadCount++;
					break;
				}
			}
		}

		if (!added) {
			if (quadGroup.getBatchCount() == EBatchSizeLimits::QUAD_MAX_BATCH_COUNT) {
				mTruncatedQuadCount++;
				return;
			} else {
				const size_t batchIndex = createNewBatchQuad(quadGroup);
				if (batchIndex != -1) {
					uint32_t textureSlot = 0;
					if (mTextureArray->hasTextureId(quad.getTexture().getId())) {
						textureSlot = mTextureArray->getTextureSlotIndex(quad.getTexture().getId());
					} else if (mTextureArray->hasTextureSlotAvailable()) {
						textureSlot = mTextureArray->addNewTexture(quad.getTexture());
					}
					quadGroup.GeoBatches[batchIndex]->add(quad, textureSlot);
					added = true;
					mDrawnQuadCount++;
				} else {
					mTruncatedQuadCount++;
					return;
				}
			}
		}
	}

	void ShapeRenderer::drawQuadArray(ShapeRenderingObjects<RenderBatchQuad>& quadGroup, const std::vector<Quad>& quadArr) {
		ZoneScoped;

		const size_t arrSize = quadArr.size();

		uint32_t quadBatchStartIndex = 0;

		for (size_t addedCount = 0; addedCount < arrSize; /* addedCount is changed inside loop, breaks loop when no more space or all geo is added */) {
			const Quad& quad = quadArr[addedCount];
			bool added = false;

			for (uint32_t batchIndex = quadBatchStartIndex; batchIndex < quadGroup.getBatchCount(); batchIndex++) {
				RenderBatchQuad& batch = *quadGroup.GeoBatches[batchIndex];
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
				if (quadGroup.getBatchCount() == EBatchSizeLimits::QUAD_MAX_BATCH_COUNT) {
					mTruncatedQuadCount += static_cast<uint32_t>(arrSize - addedCount);
					return;
				} else {
					const size_t batchIndex = createNewBatchQuad(quadGroup);
					if (batchIndex != -1) {
						quadGroup.GeoBatches[batchIndex]->add(quad, 0);
						addedCount++;
						mDrawnQuadCount++;
					} else {
						mTruncatedQuadCount += static_cast<uint32_t>(arrSize - addedCount);
						break;
					}
				}
			}
		}
	}

	void ShapeRenderer::drawQuadArrayTextured(ShapeRenderingObjects<RenderBatchQuad>& quadGroup, const std::vector<Quad>& quadArr) {
		ZoneScoped;

		const size_t arrSize = quadArr.size();
		TextureArray& texArr = *mTextureArray;

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

			for (uint32_t batchIndex = quadBatchStartIndex; batchIndex < quadGroup.getBatchCount(); batchIndex++) {
				RenderBatchQuad& batch = *quadGroup.GeoBatches[batchIndex];
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
				if (quadGroup.getBatchCount() == EBatchSizeLimits::QUAD_MAX_BATCH_COUNT) {
					mTruncatedQuadCount += static_cast<uint32_t>(arrSize - addedCount);
					return;
				} else {
					const size_t batchIndex = createNewBatchQuad(quadGroup);
					if (batchIndex != -1) {
						quadGroup.GeoBatches[batchIndex]->add(quad, textureSlotIndex);
						addedCount++;
						mDrawnQuadCount++;
					} else {
						mTruncatedQuadCount += static_cast<uint32_t>(arrSize - addedCount);
						break;
					}
				}
			}
		}
	}

	void ShapeRenderer::drawQuadArraySameTexture(ShapeRenderingObjects<RenderBatchQuad>& quadGroup, const std::vector<Quad>& quadArr) {
		ZoneScoped;

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
		TextureArray& texArr = *mTextureArray;

		uint32_t textureSlotIndex = 0;
		if (texArr.hasTextureId(textureId)) {
			textureSlotIndex = texArr.getTextureSlotIndex(textureId);
		} else if (texArr.hasTextureSlotAvailable()) {
			textureSlotIndex = texArr.addNewTexture(quadArr[0].getTexture());
		}

		uint32_t quadBatchStartIndex = 0;

		for (size_t addedCount = 0; addedCount < arrSize; /* addedCount is changed inside loop, breaks loop when no more space */) {
			const Quad& quad = quadArr[addedCount];

			for (uint32_t batchIndex = quadBatchStartIndex; batchIndex < quadGroup.getBatchCount(); batchIndex++) {
				RenderBatchQuad& batch = *quadGroup.GeoBatches[batchIndex];
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
				if (quadGroup.getBatchCount() == EBatchSizeLimits::QUAD_MAX_BATCH_COUNT) {
					mTruncatedQuadCount += static_cast<uint32_t>(arrSize - addedCount);
					return;
				} else {
					const size_t batchIndex = createNewBatchQuad(quadGroup);
					if (batchIndex != -1) {
						RenderBatchQuad& batch = *quadGroup.GeoBatches[batchIndex];
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
	}

	void ShapeRenderer::drawCircle(ShapeRenderingObjects<RenderBatchCircle>& circleGroup, const Circle& circle) {
		ZoneScoped;

		bool added = false;
		for (uint32_t i = 0; i < circleGroup.getBatchCount(); i++) {
			RenderBatchCircle& batch = *circleGroup.GeoBatches[i];
			if (batch.hasSpace(1)) {
				batch.add(circle, 0);
				added = true;
				mDrawnCircleCount++;
				break;
			}
		}

		if (!added) {
			if (circleGroup.getBatchCount() == EBatchSizeLimits::CIRCLE_MAX_BATCH_COUNT) {
				mTruncatedCircleCount++;
				return;
			} else {
				const size_t batchIndex = createNewBatchCircle(circleGroup);
				if (batchIndex == -1) {
					LOG_ERR("Failed to add new Circle Batch");
					mTruncatedQuadCount++;
					return;
				} else {
					circleGroup.GeoBatches[batchIndex]->add(circle, 0);
					mDrawnQuadCount++;
				}
			}
		}
	}

	void ShapeRenderer::drawCircleTextured(ShapeRenderingObjects<RenderBatchCircle>& circleGroup, const Circle& circle) {
		ZoneScoped;

		bool added = false;

		if (!circle.hasTexture()) {
			LOG_ERR("Circle does not have texture");
			return;
		}

		for (uint32_t i = 0; i < circleGroup.getBatchCount(); i++) {
			RenderBatchCircle& batch = *circleGroup.GeoBatches[i];
			if (batch.hasSpace(1)) {
				if (mTextureArray->hasTextureId(circle.getTexture().getId())) {
					batch.add(circle, mTextureArray->getTextureSlotIndex(circle.getTexture().getId()));
					added = true;
					mDrawnCircleCount++;
					break;
				} else if (mTextureArray->hasTextureSlotAvailable()) {
					const uint32_t textureSlot = mTextureArray->addNewTexture(circle.getTexture());
					batch.add(circle, textureSlot);
					added = true;
					mDrawnCircleCount++;
					break;
				}
			}
		}

		if (!added) {
			if (circleGroup.getBatchCount() == EBatchSizeLimits::CIRCLE_MAX_BATCH_COUNT) {
				mTruncatedCircleCount++;
				return;
			} else {
				const size_t batchIndex = createNewBatchCircle(circleGroup);
				if (batchIndex != -1) {
					uint32_t textureSlot = 0;
					if (mTextureArray->hasTextureId(circle.getTexture().getId())) {
						textureSlot = mTextureArray->getTextureSlotIndex(circle.getTexture().getId());
					} else if (mTextureArray->hasTextureSlotAvailable()) {
						textureSlot = mTextureArray->addNewTexture(circle.getTexture());
					}
					circleGroup.GeoBatches[batchIndex]->add(circle, textureSlot);
					added = true;
					mDrawnCircleCount++;
				} else {
					mTruncatedCircleCount++;
					return;
				}
			}
		}	
	}

	void ShapeRenderer::drawCircleArray(ShapeRenderingObjects<RenderBatchCircle>& circleGroup, const std::vector<Circle>& circleArr) {
		ZoneScoped;

		const size_t arrSize = circleArr.size();

		uint32_t circleBatchStartIndex = 0;

		for (size_t addedCount = 0; addedCount < arrSize; /* addedCount is changed inside loop, breaks loop when no more space or all geo is added */) {
			const Circle& circle = circleArr[addedCount];
			bool added = false;

			for (uint32_t batchIndex = circleBatchStartIndex; batchIndex < circleGroup.getBatchCount(); batchIndex++) {
				RenderBatchCircle& batch = *circleGroup.GeoBatches[batchIndex];
				if (batch.hasSpace(1)) {
					batch.add(circle, 0);
					added = true;
					addedCount++;
					mDrawnCircleCount++;
					break;
				} else {
					circleBatchStartIndex++;
				}
			}

			if (!added) {
				if (circleGroup.getBatchCount() == EBatchSizeLimits::CIRCLE_MAX_BATCH_COUNT) {
					mTruncatedCircleCount += static_cast<uint32_t>(arrSize - addedCount);
					return;
				} else {
					const size_t batchIndex = createNewBatchCircle(circleGroup);
					if (batchIndex != -1) {
						circleGroup.GeoBatches[batchIndex]->add(circle, 0);
						addedCount++;
						mDrawnCircleCount++;
					} else {
						mTruncatedCircleCount += static_cast<uint32_t>(arrSize - addedCount);
						break;
					}
				}
			}
		}
	}

	void ShapeRenderer::drawCircleArrayTextured(ShapeRenderingObjects<RenderBatchCircle>& circleGroup, const std::vector<Circle>& circleArr) {
		ZoneScoped;

		const size_t arrSize = circleArr.size();

		uint32_t circleBatchStartIndex = 0;

		for (size_t addedCount = 0; addedCount < arrSize; /* addedCount is changed inside loop, breaks loop when no more space or all quads added */) {
			const Circle& circle = circleArr[addedCount];
			bool added = false;

			const uint32_t textureSlotIndex =
				mTextureArray->hasTextureId(circle.getTexture().getId()) ?
				mTextureArray->getTextureSlotIndex(circle.getTexture().getId()) :
				mTextureArray->hasTextureSlotAvailable() ?
				mTextureArray->addNewTexture(circle.getTexture()) :
				0;

			for (uint32_t batchIndex = circleBatchStartIndex; batchIndex < circleGroup.getBatchCount(); batchIndex++) {
				RenderBatchCircle& batch = *circleGroup.GeoBatches[batchIndex];
				if (batch.hasSpace(1)) {
					batch.add(circle, textureSlotIndex);
					addedCount++;
					added = true;
					mDrawnCircleCount++;
					break;
				} else {
					circleBatchStartIndex++;
				}
			}

			if (!added) {
				if (circleGroup.getBatchCount() == EBatchSizeLimits::CIRCLE_MAX_BATCH_COUNT) {
					mTruncatedCircleCount += static_cast<uint32_t>(arrSize - addedCount);
					return;
				} else {
					const size_t batchIndex = createNewBatchCircle(circleGroup);
					if (batchIndex != -1) {
						circleGroup.GeoBatches[batchIndex]->add(circle, textureSlotIndex);
						addedCount++;
						mDrawnCircleCount++;
					} else {
						mTruncatedCircleCount += static_cast<uint32_t>(arrSize - addedCount);
						break;
					}
				}
			}
		}
	}

	void ShapeRenderer::drawCircleArraySameTexture(ShapeRenderingObjects<RenderBatchCircle>& circleGroup, const std::vector<Circle>& circleArr) {
		ZoneScoped;

		size_t addedCount = 0;
		const size_t arrSize = circleArr.size();
		if (arrSize == 0) {
			//TODO:: Is this worth a warning?
			//LOG_WARN("drawQuadArraySameTextureScene() quadArr size = 0");
			return;
		} else if (!circleArr[0].hasTexture()) {
			LOG_ERR("Quad array does not have texture");
			return;
		}
		const uint32_t textureId = circleArr[0].getTexture().getId();
		TextureArray& texArr = *mTextureArray;

		uint32_t textureSlotIndex = 0;
		if (texArr.hasTextureId(textureId)) {
			textureSlotIndex = texArr.getTextureSlotIndex(textureId);
		} else if (texArr.hasTextureSlotAvailable()) {
			textureSlotIndex = texArr.addNewTexture(circleArr[0].getTexture());
		}

		uint32_t circleBatchStartIndex = 0;

		for (size_t addedCount = 0; addedCount < arrSize; /* addedCount is changed inside loop, breaks loop when no more space */) {
			const Circle& quad = circleArr[addedCount];

			for (uint32_t batchIndex = circleBatchStartIndex; batchIndex < circleGroup.getBatchCount(); batchIndex++) {
				RenderBatchCircle& batch = *circleGroup.GeoBatches[batchIndex];
				const size_t remainingSpace = batch.getRemainingGeometrySpace();
				const size_t toAddCount = arrSize - addedCount;
				if (remainingSpace >= toAddCount) {
					batch.addAll(
						circleArr,
						addedCount,
						addedCount + toAddCount,
						texArr.getTextureSlotIndex(textureId)
					);
					addedCount += toAddCount;
					mDrawnCircleCount += static_cast<uint32_t>(toAddCount);
					break;
				} else if (remainingSpace > 0) {
					batch.addAll(
						circleArr,
						addedCount,
						addedCount + remainingSpace,
						texArr.getTextureSlotIndex(textureId)
					);
					addedCount += remainingSpace;
					mDrawnCircleCount += static_cast<uint32_t>(remainingSpace);
				} else {
					circleBatchStartIndex++;
				}
			}

			if (addedCount < arrSize) {
				if (circleGroup.getBatchCount() == EBatchSizeLimits::CIRCLE_MAX_BATCH_COUNT) {
					mTruncatedCircleCount += static_cast<uint32_t>(arrSize - addedCount);
					return;
				} else {
					const size_t batchIndex = createNewBatchCircle(circleGroup);
					if (batchIndex != -1) {
						RenderBatchCircle& batch = *circleGroup.GeoBatches[batchIndex];
						const size_t remainingSpace = batch.getRemainingGeometrySpace();
						const size_t toAddCount = arrSize - addedCount;
						if (remainingSpace >= toAddCount) {
							batch.addAll(circleArr, 0, toAddCount, textureSlotIndex);
							addedCount += toAddCount;
							mDrawnQuadCount += static_cast<uint32_t>(toAddCount);
							break;
						} else {
							batch.addAll(circleArr, addedCount, addedCount + remainingSpace, textureSlotIndex);
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
	}

	void ShapeRenderer::drawSceneImpl(uint32_t imageIndex, VkCommandBuffer cmd, CurrentBindingsState& currentBindings) {
		ZoneScoped;

		if (mSceneCameraData == nullptr) {
			if (mWarnOnNullSceneCameraData) {
				LOG_WARN("ShapeRenderer::drawSceneImpl mSceneCameraData is null");
			}
			return;
		}

		VkDevice logicDevice = mContext.getLogicDevice();
		AppDebugInfo& debugInfo = Application::get().getDebugInfo();

		{ //Draw Quads
			bool hasQuadToDraw = false;

			for (uint32_t i = 0; i < mQuadScene.getBatchCount(); i++) {
				RenderBatchQuad& batch = *mQuadScene.GeoBatches[i];
				const uint32_t quadCount = static_cast<uint32_t>(batch.getGeometryCount());
				
				if (quadCount > 0) {
					SimpleRenderable& batchRenderable = *mQuadScene.Renderables[i];
					VertexArrayVulkan& vao = batchRenderable.getVao();

					vao.getVertexBuffers()[0]->setDataMapped(
						logicDevice,
						batch.getData().data(),
						quadCount * Quad::BYTE_SIZE
					);
					vao.setDrawCount(quadCount * EBatchSizeLimits::QUAD_INDEX_COUNT);

					if (mQuadScene.Pipeline->get() != currentBindings.Pipeline) {
						mQuadScene.Pipeline->bind(cmd);
						currentBindings.Pipeline = mQuadScene.Pipeline->get();
						debugInfo.PipelineBinds++;
					}

					if (mQuadSharedIndexBuffer->getBuffer() != currentBindings.IndexBuffer) {
						mQuadSharedIndexBuffer->bind(cmd);
						currentBindings.IndexBuffer = mQuadSharedIndexBuffer->getBuffer();
						debugInfo.IndexBufferBinds++;
					}

					mQuadScene.Pipeline->recordDrawCommand(imageIndex, cmd, batchRenderable, currentBindings, 0);
					debugInfo.SceneDrawCalls++;

					batch.reset();
				}
			}
		}

		{ //Draw Circles
			for (uint32_t i = 0; i < mCircleScene.getBatchCount(); i++) {
				RenderBatchCircle& batch = *mCircleScene.GeoBatches[i];
				const uint32_t geoCount = static_cast<uint32_t>(batch.getGeometryCount());
				
				if (geoCount > 0) {
					SimpleRenderable& batchRenderable = *mCircleScene.Renderables[i];
					VertexArrayVulkan& vao = batchRenderable.getVao();
		
					vao.getVertexBuffers()[0]->setDataMapped(
						logicDevice,
						batch.getData().data(),
						geoCount * Circle::BYTE_SIZE
					);
					vao.setDrawCount(geoCount * EBatchSizeLimits::CIRCLE_INDEX_COUNT);

					if (mCircleScene.Pipeline->get() != currentBindings.Pipeline) {
						mCircleScene.Pipeline->bind(cmd);
						currentBindings.Pipeline = mCircleScene.Pipeline->get();
						debugInfo.PipelineBinds++;
					}

					if (mQuadSharedIndexBuffer->getBuffer() != currentBindings.IndexBuffer) {
						mQuadSharedIndexBuffer->bind(cmd);
						currentBindings.IndexBuffer = mQuadSharedIndexBuffer->getBuffer();
						debugInfo.IndexBufferBinds++;
					}
		
					mCircleScene.Pipeline->recordDrawCommand(imageIndex, cmd, batchRenderable, currentBindings, 0);
					debugInfo.SceneDrawCalls++;
		
					batch.reset();
				}
			}
		}
	}

	void ShapeRenderer::drawUiImpl(uint32_t imageIndex, VkCommandBuffer cmd, CurrentBindingsState& currentBindings) {
		ZoneScoped;

		VkDevice logicDevice = mContext.getLogicDevice();
		AppDebugInfo& debugInfo = Application::get().getDebugInfo();

		if (mUiCameraData == nullptr) {
			if (mWarnOnNullUiCameraData) {
				LOG_WARN("ShapeRenderer::drawUiImpl mUiCameraData is null");
			}
			return;
		}

		{ //Draw Quads
			for (uint32_t i = 0; i < mQuadUi.getBatchCount(); i++) {
				RenderBatchQuad& batch = *mQuadUi.GeoBatches[i];
				const uint32_t quadCount = static_cast<uint32_t>(batch.getGeometryCount());

				if (quadCount > 0) {
					SimpleRenderable& batchRenderable = *mQuadUi.Renderables[i];
					VertexArrayVulkan& vao = batchRenderable.getVao();

					vao.getVertexBuffers()[0]->setDataMapped(
						logicDevice,
						batch.getData().data(),
						quadCount * Quad::BYTE_SIZE
					);
					vao.setDrawCount(quadCount * EBatchSizeLimits::QUAD_INDEX_COUNT);

					if (mQuadUi.Pipeline->get() != currentBindings.Pipeline) {
						mQuadUi.Pipeline->bind(cmd);
						currentBindings.Pipeline = mQuadUi.Pipeline->get();
						debugInfo.PipelineBinds++;
					}

					//TODO:: Renderable stores shared_ptr to index buffer and there's a check & bind for IB in recordDrawCommand, is this needed here?
					if (mQuadSharedIndexBuffer->getBuffer() != currentBindings.IndexBuffer) {
						mQuadSharedIndexBuffer->bind(cmd);
						currentBindings.IndexBuffer = mQuadSharedIndexBuffer->getBuffer();
						debugInfo.IndexBufferBinds++;
					}

					mQuadUi.Pipeline->recordDrawCommand(imageIndex, cmd, batchRenderable, currentBindings, 0);
					debugInfo.UiDrawCalls++;

					batch.reset();
				}
			}
		}

		{ //Draw Circles
			for (uint32_t i = 0; i < mCircleUi.getBatchCount(); i++) {
				RenderBatchCircle& batch = *mCircleUi.GeoBatches[i];
				const uint32_t geoCount = static_cast<uint32_t>(batch.getGeometryCount());
			
				if (geoCount > 0) {
					SimpleRenderable& batchRenderable = *mCircleUi.Renderables[i];
					VertexArrayVulkan& vao = batchRenderable.getVao();
			
					vao.getVertexBuffers()[0]->setDataMapped(
						logicDevice,
						batch.getData().data(),
						geoCount * Circle::BYTE_SIZE
					);
					vao.setDrawCount(geoCount * EBatchSizeLimits::CIRCLE_INDEX_COUNT);

					if (mCircleUi.Pipeline->get() != currentBindings.Pipeline) {
						mCircleUi.Pipeline->bind(cmd);
						currentBindings.Pipeline = mCircleUi.Pipeline->get();
						debugInfo.PipelineBinds++;
					}

					if (mQuadSharedIndexBuffer->getBuffer() != currentBindings.IndexBuffer) {
						mQuadSharedIndexBuffer->bind(cmd);
						currentBindings.IndexBuffer = mQuadSharedIndexBuffer->getBuffer();
						debugInfo.IndexBufferBinds++;
					}
			
					mCircleUi.Pipeline->recordDrawCommand(imageIndex, cmd, batchRenderable, currentBindings, 0);
					debugInfo.UiDrawCalls++;
			
					batch.reset();
				}
			}
		}
	}

	size_t ShapeRenderer::createNewBatchQuad(ShapeRenderingObjects<RenderBatchQuad>& shapeRendering) {
		ZoneScoped;

		if (shapeRendering.getBatchCount() < EBatchSizeLimits::QUAD_MAX_BATCH_COUNT) {
			const size_t batchSizeBytes = static_cast<size_t>(EBatchSizeLimits::QUAD_BATCH_MAX_GEO_COUNT) * Quad::BYTE_SIZE;

			std::shared_ptr<RenderBatchQuad> batch = std::make_shared<RenderBatchQuad>(
				EBatchSizeLimits::QUAD_BATCH_MAX_GEO_COUNT,
				EBatchSizeLimits::BATCH_MAX_COUNT_TEXTURE
			);

			if (batch == nullptr) {
				return -1;
			}

			shapeRendering.GeoBatches.emplace_back(batch);

			std::shared_ptr<VertexArrayVulkan> vao = mContext.createVertexArray();
			std::shared_ptr<VertexBufferVulkan> vbo = mContext.createVertexBuffer(
				StaticVertexInputLayout::get(QUAD_VERTEX_INPUT_TYPE),
				batchSizeBytes,
				VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
			);
			vao->addVertexBuffer(vbo);
			vao->setIndexBuffer(mQuadSharedIndexBuffer, true);
			vao->getVertexBuffers()[0]->map(mContext.getLogicDevice(), batchSizeBytes);

			shapeRendering.Renderables.emplace_back(std::make_shared<SimpleRenderable>(vao, shapeRendering.DescriptorSetsInstance));

			return shapeRendering.GeoBatches.size() - 1;
		} else {
			return -1;
		}
	}

	size_t ShapeRenderer::createNewBatchCircle(ShapeRenderingObjects<RenderBatchCircle>& shapeRendering) {
		ZoneScoped;

		if (shapeRendering.getBatchCount() < EBatchSizeLimits::CIRCLE_MAX_BATCH_COUNT) {
			const size_t batchSizeBytes = static_cast<size_t>(EBatchSizeLimits::CIRCLE_BATCH_MAX_GEO_COUNT) * Circle::BYTE_SIZE;

			std::shared_ptr<RenderBatchCircle> batch = std::make_shared<RenderBatchCircle>(
				EBatchSizeLimits::CIRCLE_BATCH_MAX_GEO_COUNT,
				EBatchSizeLimits::CIRCLE_MAX_COUNT_TEXTURE
			);

			if (batch == nullptr) {
				return -1;
			}

			shapeRendering.GeoBatches.emplace_back(batch);

			std::shared_ptr<VertexArrayVulkan> vao = mContext.createVertexArray();
			std::shared_ptr<VertexBufferVulkan> vbo = mContext.createVertexBuffer(
				StaticVertexInputLayout::get(CIRCLE_VERTEX_INPUT_TYPE),
				batchSizeBytes,
				VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
			);
			vao->addVertexBuffer(vbo);
			vao->setIndexBuffer(mQuadSharedIndexBuffer, true);
			vao->getVertexBuffers()[0]->map(mContext.getLogicDevice(), batchSizeBytes);

			shapeRendering.Renderables.emplace_back(std::make_shared<SimpleRenderable>(vao, shapeRendering.DescriptorSetsInstance));

			return shapeRendering.GeoBatches.size() - 1;
		} else {
			return -1;
		}
	}

	void ShapeRenderer::closeEmptyQuadBatchesImpl() {
		ZoneScoped;

		//Since RenderBatches and renderables are linked through being the at the same index in their respective vector
		// and batches are filled begining to end, loop through from end to begining closing batch and renderable's vao as it goes,
		// stopping when there is a batch with a geo count of at least one

		//TODO:: A better way of "linking" batches and vaos. pair<batch , vao> ?
		const size_t batchCount = mQuadScene.GeoBatches.size();
		if (batchCount != mQuadScene.Renderables.size()) {
			LOG_WARN("Quad scene batches size doesn't match renderables count");
		}

		for (size_t i = batchCount; i > 0; i--) {
			if (mQuadScene.GeoBatches[i - 1]->getGeometryCount() == 0) {
				mQuadScene.GeoBatches.pop_back();
				mContext.addGpuResourceToClose(mQuadScene.Renderables[i - 1]->getVaoPtr());
				mQuadScene.Renderables.pop_back();
			} else {
				break;
			}
		}

		const size_t uiBatchCount = mQuadUi.GeoBatches.size();
		if (uiBatchCount != mQuadUi.Renderables.size()) {
			LOG_WARN("Quad UI batches size doesn't match renderables count");
		}

		for (size_t i = uiBatchCount; i > 0; i--) {
			if (mQuadUi.GeoBatches[i - 1]->getGeometryCount() == 0) {
				mQuadUi.GeoBatches.pop_back();
				mContext.addGpuResourceToClose(mQuadUi.Renderables[i - 1]->getVaoPtr());
				mQuadUi.Renderables.pop_back();
			} else {
				break;
			}
		}
	}

	void ShapeRenderer::closeEmptyCircleBatchesImpl() {
		ZoneScoped;

		//Since RenderBatches and renderables are linked through being the at the same index in their respective vector
		// and batches are filled begining to end, loop through from end to begining closing batch and renderable's vao as it goes,
		// stopping when there is a batch with a geo count of at least one

		const size_t batchCount = mCircleScene.GeoBatches.size();
		if (batchCount != mCircleScene.Renderables.size()) {
			LOG_WARN("Circle scene batches size doesn't match renderables count");
		}

		for (size_t i = batchCount; i > 0; i--) {
			if (mCircleScene.GeoBatches[i - 1]->getGeometryCount() == 0) {
				mCircleScene.GeoBatches.pop_back();
				mContext.addGpuResourceToClose(mCircleScene.Renderables[i - 1]->getVaoPtr());
				mCircleScene.Renderables.pop_back();
			} else {
				break;
			}
		}

		const size_t uiBatchCount = mCircleUi.GeoBatches.size();
		if (uiBatchCount != mCircleUi.Renderables.size()) {
			LOG_WARN("Circle UI batches size doesn't match renderables count");
		}
		
		for (size_t i = uiBatchCount; i > 0; i--) {
			if (mCircleUi.GeoBatches[i - 1]->getGeometryCount() == 0) {
				mCircleUi.GeoBatches.pop_back();
				mContext.addGpuResourceToClose(mCircleUi.Renderables[i - 1]->getVaoPtr());
				mCircleUi.Renderables.pop_back();
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

	void ShapeRenderer::close() {
		if (INSTANCE != nullptr) {
			INSTANCE->closeImpl();
			INSTANCE.release();
			INSTANCE = nullptr;
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

	//Close the dynamic batches that have a geo count of 0
	void ShapeRenderer::closeEmptyQuadBatches() {
		if (INSTANCE != nullptr) {
			INSTANCE->closeEmptyQuadBatchesImpl();
		} else {
			LOG_WARN("Attempted closeEmptyQuadBatches when ShapeRenderer is un-initialised/closed.");
		}
	}

	void ShapeRenderer::closeEmptyCircleBatches() {
		if (INSTANCE != nullptr) {
			INSTANCE->closeEmptyCircleBatchesImpl();
		} else {
			LOG_WARN("Attempted closeEmptyCircleBatches when ShapeRenderer is un-initialised/closed.");
		}
	}

	void ShapeRenderer::resetLocalDebugInfo() {
		//NOTE:: No nullptr check as this function is expected to be called each frame.
		INSTANCE->mDrawnQuadCount = 0u;
		INSTANCE->mTruncatedQuadCount = 0u;
		INSTANCE->mDrawnCircleCount = 0u;
		INSTANCE->mTruncatedCircleCount = 0u;
	}

	std::vector<DescriptorTypeInfo> ShapeRenderer::getEngineDescriptorTypeInfos() {
		//TEMP:: Currently requires two textures & texture array
		std::vector<DescriptorTypeInfo> descTypeInfos = {};
		descTypeInfos.push_back({ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2u }); //Test textures
		descTypeInfos.push_back({ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 8u }); //Texture Array
		return descTypeInfos;
	
		//NOTE:: Currently there are no Descriptors owned by the ShapeRenderer::INSTANCE so this function is empty.
		//return { };
	}

	void ShapeRenderer::setSceneCameraDataImpl(std::shared_ptr<CameraGpuData> cameraData) {
		mSceneCameraData = cameraData;
		mShapesDescSetsInstanceScene->setDescriptorSetArray(ShapeRenderer::CAMERA_UBO_SLOT, { cameraData->DescriptorSets[0], cameraData->DescriptorSets[1] });
	}
	void ShapeRenderer::setUiCameraDataImpl(std::shared_ptr<CameraGpuData> cameraData) {
		mUiCameraData = cameraData;
		mShapesDescSetsInstanceUi->setDescriptorSetArray(ShapeRenderer::CAMERA_UBO_SLOT, { cameraData->DescriptorSets[0], cameraData->DescriptorSets[1] });
	}
}
