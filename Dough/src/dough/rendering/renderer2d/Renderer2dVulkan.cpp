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
		for (RenderBatchQuad& batch : mStorage->getQuadRenderBatches()) {
			if (batch.hasSpace(1)) {
				batch.add(quad, 0);
				break;
			}
		}
	}

	void Renderer2dVulkan::drawQuadTexturedScene(Quad& quad) {
		for (RenderBatchQuad& batch : mStorage->getQuadRenderBatches()) {
			if (batch.hasSpace(1)) {
				if (batch.hasTextureId(quad.getTexture().getId())) {
					batch.add(quad, batch.getTextureSlotIndex(quad.getTexture().getId()));
					break;
				} else if (batch.hasTextureSlotAvailable()) {
					uint32_t textureSlot = batch.addNewTexture(quad.getTexture());
					batch.add(quad, textureSlot);
					break;
				}
			}
		}
	}

	void Renderer2dVulkan::drawQuadArrayScene(std::vector<Quad>& quadArr) {
		for (RenderBatchQuad& batch : mStorage->getQuadRenderBatches()) {
			if (batch.hasSpace(static_cast<uint32_t>(quadArr.size()))) {
				batch.addAll(quadArr, 0);
				break;
			}
		}
	}

	void Renderer2dVulkan::drawQuadArrayTexturedScene(std::vector<Quad>& quadArr) {
		for (RenderBatchQuad& batch : mStorage->getQuadRenderBatches()) {
			const uint32_t size = static_cast<uint32_t>(quadArr.size());
			if (size > 0 && batch.hasSpace(size)) {
				for (Quad& quad : quadArr) {
					if (batch.hasTextureId(quad.getTexture().getId())) {
						batch.add(quad, batch.getTextureSlotIndex(quad.getTexture().getId()));
						break;
					} else if (batch.hasTextureSlotAvailable()) {
						const uint32_t textureSlot = batch.addNewTexture(quad.getTexture());
						batch.add(quad, textureSlot);
						break;
					}
				}
			}
		}
	}

	void Renderer2dVulkan::drawQuadArraySameTextureScene(std::vector<Quad>& quadArr) {
		for (RenderBatchQuad& batch : mStorage->getQuadRenderBatches()) {
			const uint32_t size = static_cast<uint32_t>(quadArr.size());
			if (size > 0 && batch.hasSpace(size)) {
				const uint32_t textureSlot = batch.getTextureSlotIndex(quadArr[0].getTexture().getId());
				if (textureSlot != -1) {
					batch.addAll(quadArr, textureSlot);
					break;
				} else if (batch.hasTextureSlotAvailable()) {
					const uint32_t textureSlot = batch.addNewTexture(quadArr[0].getTexture());
					batch.addAll(quadArr, textureSlot);
					break;
				}
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
		VertexArrayVulkan& vao = mStorage->getQuadVao();

		quadPipeline.bind(cmd);
		quadPipeline.addVaoToDraw(vao);

		for (RenderBatchQuad& batch : mStorage->getQuadRenderBatches()) {
			const uint32_t quadCount = batch.getGeometryCount();

			if (quadCount > 0) {
				vao.getVertexBuffers()[0]->setData(
					logicDevice,
					batch.getData().data(),
					Renderer2dStorageVulkan::BatchSizeLimits::BATCH_QUAD_BYTE_SIZE
				);
				vao.getIndexBuffer().setCount(
					quadCount * Renderer2dStorageVulkan::BatchSizeLimits::SINGLE_QUAD_INDEX_COUNT
				);

				quadPipeline.recordDrawCommands(imageIndex, cmd);

				batch.reset();
			}
		}

		quadPipeline.clearVaoToDraw();
	}
}
