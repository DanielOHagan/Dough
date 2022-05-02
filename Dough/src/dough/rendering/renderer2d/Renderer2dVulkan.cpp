#include "dough/rendering/renderer2d/Renderer2dVulkan.h"

#include "dough/rendering/RenderingContextVulkan.h"
#include "dough/Logging.h"
#include "dough/rendering/ObjInit.h"

namespace DOH {

	Renderer2dVulkan::Renderer2dVulkan(RenderingContextVulkan& context)
	:	mContext(context)
	{}

	void Renderer2dVulkan::init(VkDevice logicDevice) {
		mStorage = std::make_unique<Renderer2dStorageVulkan>(mContext);
		mStorage->init(logicDevice);
	}

	void Renderer2dVulkan::close(VkDevice logicDevice) {
		mStorage->close(logicDevice);
	}

	void Renderer2dVulkan::closeSwapChainSpecificObjects(VkDevice logicDevice) {
		mStorage->closeSwapChainSpecificObjects(logicDevice);
	}

	void Renderer2dVulkan::recreateSwapChainSpecificObjects(SwapChainVulkan& swapChain) {
		mStorage->recreateSwapChainSpecificObjects();
	}

	void Renderer2dVulkan::drawQuadScene(Quad& quad) {
		bool added = false;
		for (RenderBatchQuad& batch : mStorage->getQuadRenderBatches()) {
			if (batch.hasSpace(1)) {
				batch.add(quad, 0);
				added = true;
				break;
			}
		}

		if (!added) {
			const size_t batchIndex = mStorage->createNewBatchQuad();
			if (batchIndex == -1) {
				LOG_ERR("Failed to add new Quad Batch");
				return;
			} else {
				RenderBatchQuad& batch = mStorage->getQuadRenderBatches()[batchIndex];
				batch.add(quad, 0);
				added = true;
			}
		}
	}

	void Renderer2dVulkan::drawQuadTexturedScene(Quad& quad) {
		bool added = false;
		TextureArray& texArr = mStorage->getTextureArray();

		for (RenderBatchQuad& batch : mStorage->getQuadRenderBatches()) {
			if (batch.hasSpace(1)) {
				if (texArr.hasTextureId(quad.getTexture().getId())) {
					batch.add(quad, texArr.getTextureSlotIndex(quad.getTexture().getId()));
					added = true;
					break;
				} else if (texArr.hasTextureSlotAvailable()) {
					const uint32_t textureSlot = texArr.addNewTexture(quad.getTexture());
					batch.add(quad, textureSlot);
					added = true;
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
			} else {
				return;
			}
		}
	}

	void Renderer2dVulkan::drawQuadArrayScene(std::vector<Quad>& quadArr) {
		size_t addedCount = 0;
		const size_t arrSize = quadArr.size();

		while (addedCount <= arrSize) {
			for (RenderBatchQuad& batch : mStorage->getQuadRenderBatches()) {
				const size_t remainingSpace = batch.getRemainingGeometrySpace();
				//If has space for all of quadArr then add all, else fill remaining space with geo
				const size_t toAddCount = arrSize - addedCount;
				if (remainingSpace >= toAddCount) {
					batch.addAll(quadArr, addedCount, addedCount + toAddCount, 0);
					addedCount += toAddCount;
					break;
				} else if (remainingSpace > 0) {
					batch.addAll(
						quadArr,
						addedCount,
						addedCount + remainingSpace,
						0
					);
					addedCount += remainingSpace;
				}
			}

			//Add a new batch if possible and add geo
			if (addedCount < arrSize) {
				const size_t batchIndex = mStorage->createNewBatchQuad();
				if (batchIndex != -1) {
					RenderBatchQuad& batch = mStorage->getQuadRenderBatches()[batchIndex];
					const size_t remainingSpace = batch.getRemainingGeometrySpace();
					//If has space for all of quadArr then add all, else fill remaining space with quads
					const size_t toAddCount = arrSize - addedCount;
					if (remainingSpace >= toAddCount) {
						batch.addAll(quadArr, addedCount, addedCount + toAddCount, 0);
						addedCount += toAddCount;
						break;
					} else if (remainingSpace > 0) {
						batch.addAll(
							quadArr,
							addedCount,
							addedCount + remainingSpace,
							0
						);
						addedCount += remainingSpace;
					}
				} else {
					break;
				}
			} else {
				break;
			}
		}

		//DEBUG:: Might be worth keeping as a "debug option"
		if (addedCount < quadArr.size()) {
			LOG_WARN(quadArr.size() - addedCount << " Quads unable to be drawn");
		}
	}

	//TODO:: Make this work more like drawQuadArraySameTextureScene,
	// use the while loop so there isn't a need to check for at least one batch
	void Renderer2dVulkan::drawQuadArrayTexturedScene(std::vector<Quad>& quadArr) {
		size_t addedCount = 0;
		const size_t arrSize = quadArr.size();
		TextureArray& texArr = mStorage->getQuadBatchTextureArray();

		if (mStorage->getQuadRenderBatches().size() == 0) {
			if (mStorage->createNewBatchQuad() == -1) {
				LOG_ERR("Failed to create new quad batch");
			}
		}

		size_t quadBatchStartIndex = 0;
		for (Quad& quad : quadArr) {
			bool added = false;

			//TODO:: If the quad is unable to be added to the texArr then addNewTexture may add a texture that is never used
			uint32_t textureSlot = 0;
			if (texArr.hasTextureId(quad.getTexture().getId())) {
				textureSlot = texArr.getTextureSlotIndex(quad.getTexture().getId());
			} else if (texArr.hasTextureSlotAvailable()) {
				textureSlot = texArr.addNewTexture(quad.getTexture());
			}
		
			for (size_t i = quadBatchStartIndex; i < mStorage->getQuadRenderBatches().size(); i++) {
				RenderBatchQuad& batch = mStorage->getQuadRenderBatches()[i];
				if (batch.hasSpace(1)) {
					batch.add(quad, textureSlot);
					addedCount++;
					added = true;
					break;
				} else {
					//Signal for next quad to exclude this batch 
					quadBatchStartIndex++;
				}
			}
		
			if (!added) {
				const size_t batchIndex = mStorage->createNewBatchQuad();
				mStorage->getQuadRenderBatches()[batchIndex].add(quad, textureSlot);
				addedCount++;
				break;
			}
		}

		//DEBUG:: Might be worth keeping as a "debug option"
		if (addedCount < quadArr.size()) {
			LOG_WARN(quadArr.size() - addedCount << " Quads unable to be drawn");
		}
	}

	void Renderer2dVulkan::drawQuadArraySameTextureScene(std::vector<Quad>& quadArr) {
		size_t addedCount = 0;
		const size_t arrSize = quadArr.size();
		const uint32_t textureId = quadArr[0].getTexture().getId();
		TextureArray& texArr = mStorage->getQuadBatchTextureArray();

		uint32_t textureSlotIndex = 0;
		if (texArr.hasTextureId(textureId)) {
			textureSlotIndex = texArr.getTextureSlotIndex(textureId);
		} else if (texArr.hasTextureSlotAvailable()) {
			textureSlotIndex = texArr.addNewTexture(quadArr[0].getTexture());
		}

		while (addedCount <= arrSize) {
			for (RenderBatchQuad& batch : mStorage->getQuadRenderBatches()) {
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
					break;
				} else if (remainingSpace > 0) {
					batch.addAll(
						quadArr,
						addedCount,
						addedCount + remainingSpace,
						texArr.getTextureSlotIndex(textureId)
					);
					addedCount += remainingSpace;
				}
			}

			//Either the existing batches don't have the space for the geo array or space for a new texture
			if (addedCount < arrSize) {
				const size_t batchIndex = mStorage->createNewBatchQuad();
				if (batchIndex != -1) {
					RenderBatchQuad& batch = mStorage->getQuadRenderBatches()[batchIndex];
					const size_t remainingSpace = batch.getRemainingGeometrySpace();
					const size_t toAddCount = arrSize - addedCount;
					if (remainingSpace >= toAddCount) {
						batch.addAll(quadArr, 0, toAddCount, textureSlotIndex);
						addedCount += toAddCount;
						break;
					} else {
						batch.addAll(quadArr, addedCount, addedCount + remainingSpace, textureSlotIndex);
						addedCount += remainingSpace;
					}
				} else {
					break;
				}
			} else {
				break;
			}
		}

		//DEBUG:: Might be worth keeping as a "debug option"
		if (addedCount < quadArr.size()) {
			LOG_WARN(quadArr.size() - addedCount << " Quads unable to be drawn");
		}
	}

	void Renderer2dVulkan::updateSceneUniformData(
		VkDevice logicDevice,
		uint32_t currentImage,
		glm::mat4x4& sceneProjView
	) {
		const uint32_t uboBinding = 0;
		mStorage->getQuadGraphicsPipeline().getShaderDescriptor().getBuffersFromBinding(uboBinding)[currentImage]->
			setData(logicDevice, &sceneProjView, sizeof(glm::mat4x4));
	}

	void Renderer2dVulkan::flushScene(VkDevice logicDevice, uint32_t imageIndex, VkCommandBuffer cmd) {
		GraphicsPipelineVulkan& quadPipeline = mStorage->getQuadGraphicsPipeline();

		quadPipeline.bind(cmd);
		mStorage->getQuadBatchIndexBuffer().bind(cmd);

		uint32_t index = 0;
		for (RenderBatchQuad& batch : mStorage->getQuadRenderBatches()) {
			const uint32_t quadCount = static_cast<uint32_t>(batch.getGeometryCount());

			if (quadCount > 0) {
				VertexArrayVulkan& vao = *mStorage->getQuadRenderBatchVaos()[index];

				vao.getVertexBuffers()[0]->setData(
					logicDevice,
					batch.getData().data(),
					Renderer2dStorageVulkan::BatchSizeLimits::BATCH_QUAD_BYTE_SIZE
				);
				vao.setDrawCount(quadCount * Renderer2dStorageVulkan::BatchSizeLimits::SINGLE_QUAD_INDEX_COUNT);

				quadPipeline.addVaoToDraw(vao);

				batch.reset();
			}

			index++;
		}

		quadPipeline.recordDrawCommands(imageIndex, cmd);

		quadPipeline.clearVaoToDraw();
	}

	void Renderer2dVulkan::closeEmptyQuadBatches() {
		//Since RenderBatches and VAOs are linked through being the at the same index in their respective vector
		// and batches are filled begining to end, loop through from end to begining closing batch and vao as it goes,
		// stopping when there is a batch with a geo count of at least one

		//TODO:: A better way of "linking" batches and vaos
		std::vector<RenderBatchQuad>& batches = mStorage->getQuadRenderBatches();
		std::vector<std::shared_ptr<VertexArrayVulkan>>& vaos = mStorage->getQuadRenderBatchVaos();

		if (batches.size() != vaos.size()) {
			LOG_WARN("Quad batches size doesn't match vaos");
		}


		for (size_t i = batches.size() - 1; i > 0; i--) {
			if (batches[i].getGeometryCount() == 0) {
				batches.pop_back();
				mContext.addResourceToCloseAfterUse(vaos[i]);
				vaos.pop_back();
			} else {
				break;
			}
		}
	}
}
