#include "dough/rendering/renderer2d/Renderer2dVulkan.h"

#include "dough/rendering/RenderingContextVulkan.h"
#include "dough/Logging.h"
#include "dough/rendering/ObjInit.h"

namespace DOH {

	Renderer2dVulkan::Renderer2dVulkan(RenderingContextVulkan& context)
	:	mContext(context),
		mDebugInfoDrawCount(0),
		mDrawnQuadCount(0),
		mTruncatedQuadCount(0)
	{}

	void Renderer2dVulkan::init(VkDevice logicDevice) {
		mStorage = std::make_unique<Renderer2dStorageVulkan>(mContext);
		mStorage->init(logicDevice);
	}

	void Renderer2dVulkan::close(VkDevice logicDevice) {
		mStorage->close(logicDevice);
	}

	void Renderer2dVulkan::onSwapChainResize(VkDevice logicDevice, SwapChainVulkan& swapChain) {
		mStorage->onSwapChainResize(logicDevice, swapChain);
	}

	void Renderer2dVulkan::drawQuadScene(Quad& quad) {
		bool added = false;
		for (RenderBatchQuad& batch : mStorage->getQuadRenderBatches()) {
			if (batch.hasSpace(1)) {
				batch.add(quad, 0);
				added = true;
				mDrawnQuadCount++;
				break;
			}
		}

		if (!added) {
			const size_t batchIndex = mStorage->createNewBatchQuad();
			if (batchIndex == -1) {
				LOG_ERR("Failed to add new Quad Batch");
				mTruncatedQuadCount++;
				return;
			} else {
				RenderBatchQuad& batch = mStorage->getQuadRenderBatches()[batchIndex];
				batch.add(quad, 0);
				mDrawnQuadCount++;
			}
		} else {
			mTruncatedQuadCount++;
		}
	}

	void Renderer2dVulkan::drawQuadTexturedScene(Quad& quad) {
		bool added = false;
		TextureArray& texArr = mStorage->getQuadBatchTextureArray();

		if (!quad.hasTexture()) {
			LOG_ERR("Quad does not have texture");
			return;
		}

		for (RenderBatchQuad& batch : mStorage->getQuadRenderBatches()) {
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
			const size_t batchIndex = mStorage->createNewBatchQuad();
			if (batchIndex != -1) {
				RenderBatchQuad& batch = mStorage->getQuadRenderBatches()[batchIndex];
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

	void Renderer2dVulkan::drawQuadArrayScene(std::vector<Quad>& quadArr) {
		const size_t arrSize = quadArr.size();
		TextureArray& texArr = mStorage->getQuadBatchTextureArray();

		uint32_t quadBatchStartIndex = 0;

		for (size_t addedCount = 0; addedCount < arrSize; addedCount++) {
			Quad& quad = quadArr[addedCount];
			bool added = false;

			for (uint32_t batchIndex = quadBatchStartIndex; batchIndex < mStorage->getQuadRenderBatches().size(); batchIndex++) {
				RenderBatchQuad& batch = mStorage->getQuadRenderBatches()[batchIndex];
				if (batch.hasSpace(1)) {
					batch.add(quad, 0);
					added = true;
					mDrawnQuadCount += static_cast<uint32_t>(addedCount);
					break;
				} else {
					quadBatchStartIndex++;
				}
			}

			if (!added) {
				const size_t batchIndex = mStorage->createNewBatchQuad();
				if (batchIndex != -1) {
					mStorage->getQuadRenderBatches()[batchIndex].add(quad, 0);
					mDrawnQuadCount += static_cast<uint32_t>(addedCount);
				} else {
					mTruncatedQuadCount += static_cast<uint32_t>(arrSize - addedCount);
					break;
				}
			}
		}
	}

	void Renderer2dVulkan::drawQuadArrayTexturedScene(std::vector<Quad>& quadArr) {
		const size_t arrSize = quadArr.size();
		TextureArray& texArr = mStorage->getQuadBatchTextureArray();

		uint32_t quadBatchStartIndex = 0;

		for (size_t addedCount = 0; addedCount < arrSize; addedCount++) {
			Quad& quad = quadArr[addedCount];
			bool added = false;

			uint32_t textureSlotIndex = 0;
			if (texArr.hasTextureId(quad.getTexture().getId())) {
				textureSlotIndex = texArr.getTextureSlotIndex(quad.getTexture().getId());
			} else if (texArr.hasTextureSlotAvailable()) {
				textureSlotIndex = texArr.addNewTexture(quad.getTexture());
			}

			for (uint32_t batchIndex = quadBatchStartIndex; batchIndex < mStorage->getQuadRenderBatches().size(); batchIndex++) {
				RenderBatchQuad& batch = mStorage->getQuadRenderBatches()[batchIndex];
				if (batch.hasSpace(1)) {
					batch.add(quad, textureSlotIndex);
					added = true;
					mDrawnQuadCount += static_cast<uint32_t>(addedCount);
					break;
				} else {
					quadBatchStartIndex++;
				}
			}

			if (!added) {
				const size_t batchIndex = mStorage->createNewBatchQuad();
				if (batchIndex != -1) {
					mStorage->getQuadRenderBatches()[batchIndex].add(quad, textureSlotIndex);
					mDrawnQuadCount += static_cast<uint32_t>(addedCount);
				} else {
					mTruncatedQuadCount += static_cast<uint32_t>(arrSize - addedCount);
					break;
				}
			}
		}
	}

	void Renderer2dVulkan::drawQuadArraySameTextureScene(std::vector<Quad>& quadArr) {
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
		TextureArray& texArr = mStorage->getQuadBatchTextureArray();

		uint32_t textureSlotIndex = 0;
		if (texArr.hasTextureId(textureId)) {
			textureSlotIndex = texArr.getTextureSlotIndex(textureId);
		} else if (texArr.hasTextureSlotAvailable()) {
			textureSlotIndex = texArr.addNewTexture(quadArr[0].getTexture());
		}

		uint32_t quadBatchStartIndex = 0;

		for (size_t addedCount = 0; addedCount < arrSize;) {
			Quad& quad = quadArr[addedCount];

			for (uint32_t batchIndex = quadBatchStartIndex; batchIndex < mStorage->getQuadRenderBatches().size(); batchIndex++) {
				RenderBatchQuad& batch = mStorage->getQuadRenderBatches()[batchIndex];
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
				const size_t batchIndex = mStorage->createNewBatchQuad();
				if (batchIndex != -1) {
					RenderBatchQuad& batch = mStorage->getQuadRenderBatches()[batchIndex];
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

	void Renderer2dVulkan::updateRenderer2dUniformData(
		VkDevice logicDevice,
		uint32_t currentImage,
		glm::mat4x4& sceneProjView
	) {
		const uint32_t uboBinding = 0;
		mStorage->getQuadGraphicsPipeline().getShaderDescriptor().getBuffersFromBinding(uboBinding)[currentImage]->
			setData(logicDevice, &sceneProjView, sizeof(glm::mat4x4));

		//TODO:: UI
	}

	void Renderer2dVulkan::flushScene(VkDevice logicDevice, uint32_t imageIndex, VkCommandBuffer cmd) {
		GraphicsPipelineVulkan& quadPipeline = mStorage->getQuadGraphicsPipeline();

		quadPipeline.bind(cmd);
		mStorage->getQuadBatchIndexBuffer().bind(cmd);

		uint32_t index = 0;
		for (RenderBatchQuad& batch : mStorage->getQuadRenderBatches()) {
			const uint32_t quadCount = static_cast<uint32_t>(batch.getGeometryCount());

			if (quadCount > 0) {
				const auto& renderableBatch = mStorage->getRenderableQuadBatches()[index];
				VertexArrayVulkan& vao = renderableBatch->getVao();

				vao.getVertexBuffers()[0]->setData(
					logicDevice,
					batch.getData().data(),
					Renderer2dStorageVulkan::BatchSizeLimits::BATCH_QUAD_BYTE_SIZE
				);
				vao.setDrawCount(quadCount * Renderer2dStorageVulkan::BatchSizeLimits::SINGLE_QUAD_INDEX_COUNT);

				quadPipeline.addRenderableToDraw(renderableBatch);
				mDebugInfoDrawCount++;

				batch.reset();
			}

			index++;
		}

		quadPipeline.recordDrawCommands(imageIndex, cmd);

		quadPipeline.clearRenderableToDraw();

		mDrawnQuadCount = 0;
		mTruncatedQuadCount = 0;
	}

	void Renderer2dVulkan::closeEmptyQuadBatches() {
		//Since RenderBatches and renderables are linked through being the at the same index in their respective vector
		// and batches are filled begining to end, loop through from end to begining closing batch and renderable's vao as it goes,
		// stopping when there is a batch with a geo count of at least one

		//TODO:: A better way of "linking" batches and vaos
		std::vector<RenderBatchQuad>& batches = mStorage->getQuadRenderBatches();
		//std::vector<std::shared_ptr<VertexArrayVulkan>>& vaos = mStorage->getQuadRenderBatchVaos();
		std::vector<std::shared_ptr<SimpleRenderable>>& renderableQuadBatches = mStorage->getRenderableQuadBatches();

		if (batches.size() != renderableQuadBatches.size()) {
			LOG_WARN("Quad batches size doesn't match vaos");
		}

		for (size_t i = batches.size(); i > 0; i--) {
			if (batches[i - 1].getGeometryCount() == 0) {
				batches.pop_back();
				mContext.closeGpuResourceImmediately(renderableQuadBatches[i - 1]->getVao());
				renderableQuadBatches.pop_back();
			} else {
				break;
			}
		}
	}
}
