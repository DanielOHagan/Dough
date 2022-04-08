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
		if (mStorage->sceneQuadBatchHasSpace(1)) {
			mStorage->addSceneQuad(quad);
		} else {
			LOG_WARN("Failed to add scene quad");
		}
	}

	void Renderer2dVulkan::drawQuadArrayScene(std::vector<Quad>& quadArr) {
		if (mStorage->sceneQuadBatchHasSpace(quadArr.size())) {
			mStorage->addSceneQuadArray(quadArr);
		} else {
			LOG_WARN("Failed to add scene quad array of size: " << quadArr.size());
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
		const uint32_t quadCount = mStorage->getRenderBatchQuad().getGeometryCount();
		if (quadCount > 0) {
			GraphicsPipelineVulkan& quadPipeline = mStorage->getQuadGraphicsPipeline();

			//foreach (batch : mStorage->getQuadBatches()) {

			//TODO:: create uniform objects with each batch.textures

			RenderBatchQuad& batch = mStorage->getRenderBatchQuad();

			//TODO::VertexArrayVulkan& vao = batch.getVao();
			VertexArrayVulkan& vao = mStorage->getQuadVao();

			quadPipeline.bind(cmd);

			std::vector<float> batchData = batch.getData();

			//IMPORTANT NOTE:: Since only 1 vbo is allowed, I can assume here that index 0 must be where the correct vbo is
			vao.getVertexBuffers()[0]->setData(
				logicDevice,
				batchData.data(),
				Renderer2dStorageVulkan::BatchSizeLimits::BATCH_QUAD_BYTE_SIZE
			);
			vao.getIndexBuffer().setCount(
				quadCount * Renderer2dStorageVulkan::BatchSizeLimits::SINGLE_QUAD_INDEX_COUNT
			);

			quadPipeline.addVaoToDraw(vao);
			quadPipeline.recordDrawCommands(imageIndex, cmd);
			quadPipeline.clearVaoToDraw();

			batch.reset();
			
			//}
		}
	}
}
