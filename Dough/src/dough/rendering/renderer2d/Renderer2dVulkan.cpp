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

		for (size_t addedCount = 0; addedCount < arrSize; /* addedCount is changed inside loop, breaks loop when no more space or all quads added */) {
			Quad& quad = quadArr[addedCount];
			bool added = false;

			for (uint32_t batchIndex = quadBatchStartIndex; batchIndex < mStorage->getQuadRenderBatches().size(); batchIndex++) {
				RenderBatchQuad& batch = mStorage->getQuadRenderBatches()[batchIndex];
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
				const size_t batchIndex = mStorage->createNewBatchQuad();
				if (batchIndex != -1) {
					mStorage->getQuadRenderBatches()[batchIndex].add(quad, 0);
					addedCount++;
					mDrawnQuadCount++;
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

		for (size_t addedCount = 0; addedCount < arrSize; /* addedCount is changed inside loop, breaks loop when no more space or all quads added */) {
			Quad& quad = quadArr[addedCount];
			bool added = false;

			const uint32_t textureSlotIndex =
				texArr.hasTextureId(quad.getTexture().getId()) ?
					texArr.getTextureSlotIndex(quad.getTexture().getId()) :
					texArr.hasTextureSlotAvailable() ?
						texArr.addNewTexture(quad.getTexture()) :
						0;

			for (uint32_t batchIndex = quadBatchStartIndex; batchIndex < mStorage->getQuadRenderBatches().size(); batchIndex++) {
				RenderBatchQuad& batch = mStorage->getQuadRenderBatches()[batchIndex];
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
				const size_t batchIndex = mStorage->createNewBatchQuad();
				if (batchIndex != -1) {
					mStorage->getQuadRenderBatches()[batchIndex].add(quad, textureSlotIndex);
					addedCount++;
					mDrawnQuadCount++;
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

		for (size_t addedCount = 0; addedCount < arrSize; /* addedCount is changed inside loop, breaks loop when no more space */) {
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

	void Renderer2dVulkan::drawTextFromQuads(std::vector<Quad>& quadArr) {
		RenderBatchQuad& textBatch = mStorage->getTextRenderBatch();
		TextureArray& textTextureArr = mStorage->getTextTextureArray();

		for (const Quad& quad : quadArr) {
			textBatch.add(
				quad,
				textTextureArr.getTextureSlotIndex(quad.getTexture().getId())
			);
		}
	}

	void Renderer2dVulkan::drawTextSameTextureFromQuads(std::vector<Quad>& quadArr) {
		if (quadArr.size() == 0) {
			//TODO:: Is this worth a warning?
			//LOG_WARN("drawTextSameTextureFromQuads() quadArr size = 0");
			return;
		} else if (!quadArr[0].hasTexture()) {
			LOG_ERR("Quad array does not have texture");
			return;
		}

		mStorage->getTextRenderBatch().addAll(
			quadArr,
			mStorage->getTextTextureArray().getTextureSlotIndex(quadArr[0].getTexture().getId())
		);
	}

	std::vector<Quad> Renderer2dVulkan::getStringAsQuads(
		const char* string,
		const glm::vec3 rootPos,
		const FontBitmap& bitmap,
		const ETextFlags2d flags
	) const {
		const size_t stringLength = strlen(string);

		if (stringLength == 0) {
			return {};
		}

		std::vector<Quad> quads;
		quads.reserve(stringLength);

		glm::vec3 currentPos = rootPos;

		uint32_t lastCharId = 0;

		for (size_t i = 0; i < stringLength; i++) {
			//TODO:: currently doesn't support a UTF-8 conversion so a cast to uint produces ASCII decimal values
			const uint32_t charId = static_cast<uint32_t>(string[i]);

			//Handle special characters
			if (charId == 32) { //space
				currentPos.x += bitmap.getSpaceWidthNorm();
				lastCharId = 0;
				continue;
			} else if (charId == 10) { //new line
				currentPos.y -=  bitmap.getLineHeightNorm();
				currentPos.x = rootPos.x;
				lastCharId = 0;
				continue;
			} else if (charId == 9) { //tab
				currentPos.x += bitmap.getTabWidthNorm(); //Tab size is equal to 4 glyph sizes
				lastCharId = 0;
				continue;
			}

			const auto& g = bitmap.getGlyphMap().find(charId);
			if (g != bitmap.getGlyphMap().end()) {

				Quad quad = {};
				quad.Position = {
					currentPos.x + g->second.Offset.x,
					currentPos.y - g->second.Size.y + bitmap.getBaseNorm() + g->second.Offset.y,
					currentPos.z
				};
				quad.Size = {
					g->second.Size.x,
					g->second.Size.y
				};

				quad.TextureCoords = {
					g->second.TexCoordTopLeft.x,
					g->second.TexCoordBotRight.y,

					g->second.TexCoordBotRight.x,
					g->second.TexCoordBotRight.y,

					g->second.TexCoordBotRight.x,
					g->second.TexCoordTopLeft.y,
					
					g->second.TexCoordTopLeft.x,
					g->second.TexCoordTopLeft.y
				};

				quad.Colour = { 1.0f, 1.0f, 1.0f, 1.0f };

				//TODO:: convert to using an unordered_map instead of vector
				for (const KerningData& k : bitmap.getKernings()) {
					if (k.FirstGlyphId == lastCharId && k.SecondGlyphId == charId) {
						quad.Position.x += k.Amount;
					}
				}

				//TODO:: Turn 
				//KerningMapKey kerningKey = { lastCharId, charId };
				//const auto& kerning = bitmap.getKerningMap().find(kerningKey);
				//if (kerning != bitmap.getKerningMap().end()) {
				//	quad.Position.x += kerning->second;
				//}
				
				
				lastCharId = charId;

				quad.setTexture(*bitmap.getPageTexture(g->second.PageId));
				quads.emplace_back(quad);

				currentPos.x += g->second.AdvanceX;

			} else {
				LOG_WARN("Failed to find charId: " << charId << " in bitmap");
			}
		}

		return quads;
	}

	void Renderer2dVulkan::updateRenderer2dUniformData(
		VkDevice logicDevice,
		uint32_t currentImage,
		glm::mat4x4& sceneProjView
	) {
		const uint32_t uboBinding = 0;
		mStorage->getQuadGraphicsPipeline().getShaderDescriptor().getBuffersFromBinding(uboBinding)[currentImage]->
			setData(logicDevice, &sceneProjView, sizeof(glm::mat4x4));

		mStorage->getTextGraphicsPipeline().getShaderDescriptor().getBuffersFromBinding(uboBinding)[currentImage]->
			setData(logicDevice, &sceneProjView, sizeof(glm::mat4x4));

		//TODO:: UI
	}

	void Renderer2dVulkan::flushScene(VkDevice logicDevice, uint32_t imageIndex, VkCommandBuffer cmd) {

		bool quadIboBound = false;

		{ //Draw Quads
			GraphicsPipelineVulkan& quadPipeline = mStorage->getQuadGraphicsPipeline();

			bool quadPipelineBound = false;

			uint32_t index = 0;
			for (RenderBatchQuad& batch : mStorage->getQuadRenderBatches()) {
				const uint32_t quadCount = static_cast<uint32_t>(batch.getGeometryCount());

				if (quadCount > 0) {
					if (!quadPipelineBound) {
						quadPipeline.bind(cmd);
					}

					if (!quadIboBound) {
						mStorage->getQuadBatchIndexBuffer().bind(cmd);
					}

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

		{ //Draw Text
			GraphicsPipelineVulkan& textPipeline = mStorage->getTextGraphicsPipeline();
			RenderBatchQuad& textBatch = mStorage->getTextRenderBatch();
			const size_t textQuadCount = textBatch.getGeometryCount();

			if (textQuadCount > 0) {
				textPipeline.bind(cmd);

				if (!quadIboBound) {
					//NOTE:: Text is currently rendered as a Quad so it can share the Quad batch index buffer
					mStorage->getQuadBatchIndexBuffer().bind(cmd);
				}

				const auto& renderableBatch = mStorage->getRenderableTextBatch();
				VertexArrayVulkan& vao = renderableBatch->getVao();

				vao.getVertexBuffers()[0]->setData(
					logicDevice,
					textBatch.getData().data(),
					textQuadCount * Renderer2dStorageVulkan::BatchSizeLimits::SINGLE_QUAD_BYTE_SIZE
				);
				vao.setDrawCount(static_cast<uint32_t>(textQuadCount * Renderer2dStorageVulkan::BatchSizeLimits::SINGLE_QUAD_INDEX_COUNT));

				textPipeline.addRenderableToDraw(renderableBatch);

				textBatch.reset();
			}

			textPipeline.recordDrawCommands(imageIndex, cmd);

			textPipeline.clearRenderableToDraw();
		}
	}

	void Renderer2dVulkan::closeEmptyQuadBatches() {
		//Since RenderBatches and renderables are linked through being the at the same index in their respective vector
		// and batches are filled begining to end, loop through from end to begining closing batch and renderable's vao as it goes,
		// stopping when there is a batch with a geo count of at least one

		//TODO:: A better way of "linking" batches and vaos
		std::vector<RenderBatchQuad>& batches = mStorage->getQuadRenderBatches();
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
