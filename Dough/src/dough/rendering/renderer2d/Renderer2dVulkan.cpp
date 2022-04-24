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

		for (RenderBatchQuad& batch : mStorage->getQuadRenderBatches()) {
			if (batch.hasSpace(1)) {
				if (batch.hasTextureId(quad.getTexture().getId())) {
					batch.add(quad, batch.getTextureSlotIndex(quad.getTexture().getId()));
					added = true;
					break;
				} else if (batch.hasTextureSlotAvailable()) {
					const uint32_t textureSlot = batch.addNewTexture(quad.getTexture());
					batch.add(quad, textureSlot);
					added = true;
					break;
				}
			}
		}

		if (!added) {
			const size_t batchIndex = mStorage->createNewBatchQuad();
			if (batchIndex == -1) {
				LOG_ERR("Failed to add new Quad Batch");
				return;
			} else {
				RenderBatchQuad& batch = mStorage->getQuadRenderBatches()[batchIndex];
				const uint32_t textureSlot = batch.addNewTexture(quad.getTexture());
				batch.add(quad, textureSlot);
				added = true;
			}
		}
	}

	void Renderer2dVulkan::drawQuadArrayScene(std::vector<Quad>& quadArr) {
		size_t addedCount = 0;
		const size_t arrSize = quadArr.size();

		while (addedCount <= arrSize) {
			for (RenderBatchQuad& batch : mStorage->getQuadRenderBatches()) {
				const size_t remainingSpace = batch.getRemainingGeometrySpace();
				//If has space for all of quadArr then add all, else fill remaining space with 
				const size_t toAddCount = arrSize - addedCount;
				if (remainingSpace >= toAddCount) {
					batch.addAll(quadArr, 0, toAddCount, 0);
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

			//Add a new batch if possible and 
			if (addedCount < arrSize) {
				const size_t batchIndex = mStorage->createNewBatchQuad();
				if (batchIndex != -1) {
					RenderBatchQuad& batch = mStorage->getQuadRenderBatches()[batchIndex];
					const size_t remainingSpace = batch.getRemainingGeometrySpace();
					//If has space for all of quadArr then add all, else fill remaining space with quads
					const size_t toAddCount = arrSize - addedCount;
					if (remainingSpace >= toAddCount) {
						batch.addAll(quadArr, 0, toAddCount, 0);
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

	void Renderer2dVulkan::drawQuadArrayTexturedScene(std::vector<Quad>& quadArr) {
		size_t addedCount = 0;

		for (RenderBatchQuad& batch : mStorage->getQuadRenderBatches()) {
			const uint32_t size = static_cast<uint32_t>(quadArr.size());
			if (size > 0 && batch.hasSpace(size)) {
				for (Quad& quad : quadArr) {
					if (batch.hasTextureId(quad.getTexture().getId())) {
						batch.add(quad, batch.getTextureSlotIndex(quad.getTexture().getId()));
						addedCount += quadArr.size();
						continue;
					} else if (batch.hasTextureSlotAvailable()) {
						const uint32_t textureSlot = batch.addNewTexture(quad.getTexture());
						batch.add(quad, textureSlot);
						addedCount =+ quadArr.size();
						continue;
					}
				}
			}
		}

		//Add excess quads to a new batch
		//TODO:: currently assumes the new batch has size for the excess quads
		if (addedCount < quadArr.size()) {
			const size_t batchIndex = mStorage->createNewBatchQuad();
			if (batchIndex == -1) {
				LOG_ERR("Failed to add new Quad Batch");
				return;
			} else {
				RenderBatchQuad& batch = mStorage->getQuadRenderBatches()[batchIndex];
				for (size_t i = addedCount; i < quadArr.size(); i++) {
					if (batch.hasTextureId(quadArr[i].getTexture().getId())) {
						batch.add(quadArr[i], batch.getTextureSlotIndex(quadArr[i].getTexture().getId()));
						addedCount++;
						continue;
					} else if (batch.hasTextureSlotAvailable()) {
						const uint32_t textureSlot = batch.addNewTexture(quadArr[i].getTexture());
						batch.add(quadArr[i], textureSlot);
						addedCount++;
						continue;
					}
				}
			}
		}
	}

	void Renderer2dVulkan::drawQuadArraySameTextureScene(std::vector<Quad>& quadArr) {
		size_t addedCount = 0;
		const uint32_t textureId = quadArr[0].getTexture().getId();
		const uint32_t size = static_cast<uint32_t>(quadArr.size());

		//TODO::
		//for (RenderBatchQuad& batch : getBatchesUsingTexture(textureId)) {
		//	if (batch.hasSpace(size)) {
		//		const uint32_t textureSlot = batch.getTextureSlotIndex(textureId);
		//		batch.addAll(quadArr, textureSlot);
		//		addedCount += size;
		//		break;
		//	}
		//}
		//
		//if (addedCount < size) {
		//	
		//}
		//
		//
		
		//Search through existing batches for a batch the has 
		for (RenderBatchQuad& batch : mStorage->getQuadRenderBatches()) {
			if (size > 0 && batch.hasSpace(size)) {
				const uint32_t textureSlot = batch.getTextureSlotIndex(textureId);
				if (textureSlot != -1) {
					batch.addAll(quadArr, textureSlot);
					addedCount += quadArr.size();
					break;
				} else if (batch.hasTextureSlotAvailable()) {
					const uint32_t textureSlot = batch.addNewTexture(quadArr[0].getTexture());
					batch.addAll(quadArr, textureSlot);
					addedCount += quadArr.size();
					break;
				}
			}
		}

		//Add excess quads to a new batch
		//TODO:: currently assumes the new batch has size for the excess quads
		if (addedCount < quadArr.size()) {
			const size_t batchIndex = mStorage->createNewBatchQuad();
			if (batchIndex == -1) {
				LOG_ERR("Failed to add new Quad Batch");
				return;
			} else {
				RenderBatchQuad& batch = mStorage->getQuadRenderBatches()[batchIndex];
				const uint32_t textureSlot = batch.addNewTexture(quadArr[0].getTexture());
				batch.addAll(quadArr, addedCount > 0 ? addedCount - 1 : 0, quadArr.size() - 1, textureSlot);
				addedCount += quadArr.size() - addedCount;
			}
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
		//VertexArrayVulkan& vao = mStorage->getQuadVao();

		quadPipeline.bind(cmd);

		//TODO:: each batch needs separate VAOs

		//quadPipeline.addVaoToDraw(vao);

		uint32_t index = 0;
		for (RenderBatchQuad& batch : mStorage->getQuadRenderBatches()) {
			const uint32_t quadCount = static_cast<uint32_t>(batch.getGeometryCount());

			if (quadCount > 0) {
				VertexArrayVulkan& vao = *mStorage->getQuadRenderBatchVaos()[index];
				//batch.uploadData(logicDevice);
				vao.getVertexBuffers()[0]->setData(
					logicDevice,
					batch.getData().data(),
					Renderer2dStorageVulkan::BatchSizeLimits::BATCH_QUAD_BYTE_SIZE
				);
				vao.getIndexBuffer().setCount(
					quadCount * Renderer2dStorageVulkan::BatchSizeLimits::SINGLE_QUAD_INDEX_COUNT
				);

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
