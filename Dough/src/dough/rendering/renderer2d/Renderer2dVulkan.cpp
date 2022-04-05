#include "dough/rendering/renderer2d/Renderer2dVulkan.h"

#include "dough/rendering/renderer2d/Renderer2dStorageVulkan.h"
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
		if (mStorage->sceneQuadBatchHasSpace(1)) {
			mStorage->addSceneQuad(quad);
		} else {
			LOG_WARN("Failed to add scene quad");
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

	void Renderer2dVulkan::flush/*Scene*/(VkDevice logicDevice, uint32_t imageIndex, VkCommandBuffer cmd) {
		const size_t quadCount = mStorage->getSceneQuadCount();
		if (quadCount > 0) {
			GraphicsPipelineVulkan& quadPipeline = mStorage->getQuadGraphicsPipeline();
			quadPipeline.bind(cmd);
		
			//foreach (batch : mStorage->getQuadBatches()) {
			VertexArrayVulkan& sceneQuadVao = mStorage->getQuadVao();
			sceneQuadVao.getIndexBuffer().setCount(mStorage->getSceneQuadCount() * Renderer2dStorageVulkan::BatchSizeLimits::SINGLE_QUAD_INDEX_COUNT);

			quadPipeline.addVaoToDraw(sceneQuadVao);
			quadPipeline.recordDrawCommands(imageIndex, cmd);
		
			quadPipeline.clearVaoToDraw();
			//}
		
			//TODO:: Enable per-frame updates to quad batches
			//mStorage->resetBatches();
		}
	}
}
