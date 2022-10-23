#pragma once

#include "dough/Utils.h"
#include "dough/scene/geometry/Quad.h"
#include "dough/rendering/renderer2d/Renderer2dStorageVulkan.h"

#include <vulkan/vulkan_core.h>

namespace DOH {

	class RenderingContextVulkan;

	class Renderer2dVulkan {

	private:
		RenderingContextVulkan& mContext;
		std::unique_ptr<Renderer2dStorageVulkan> mStorage;

		//-----Debug information-----
		uint32_t mDebugInfoDrawCount;
		uint32_t mDrawnQuadCount;
		uint32_t mTruncatedQuadCount;

	public:
		Renderer2dVulkan(RenderingContextVulkan& context);
		Renderer2dVulkan(const Renderer2dVulkan& copy) = delete;
		void operator=(const Renderer2dVulkan& assignment) = delete;


		void init(VkDevice logicDevce);
		void close(VkDevice logicDevice);
		void onSwapChainResize(VkDevice logicDevice, SwapChainVulkan& swapChain);

		void updateRenderer2dUniformData(VkDevice logicDevice, uint32_t currentImage, glm::mat4x4& sceneProjView);
		void flushScene(VkDevice logicDevice, uint32_t imageIndex, VkCommandBuffer cmd);
		//TODO:: separate flush functions for rendering in different render passes
		//void flushUi();

		//Close the dynamic batches that have a geo count of 0
		void closeEmptyQuadBatches();

		void drawQuadScene(Quad& quad);
		void drawQuadTexturedScene(Quad& quad);
		void drawQuadArrayScene(std::vector<Quad>& quadArr);
		void drawQuadArrayTexturedScene(std::vector<Quad>& quadArr);
		void drawQuadArraySameTextureScene(std::vector<Quad>& quadArr);
		//void drawQuadUi(Quad& quad);


		//-----DEBUG----- Used to show info to ImGui in app
		inline Renderer2dStorageVulkan& getStorage() const { return *mStorage; }
		inline size_t getQuadBatchCount() const { return mStorage->getQuadRenderBatches().size(); }
		inline uint32_t getDrawCount() const { return mDebugInfoDrawCount; }
		inline uint32_t getDrawnQuadCount() const { return mDrawnQuadCount; }
		inline uint32_t getTruncatedQuadCount() const { return mTruncatedQuadCount; }
		inline void resetDrawCount() { mDebugInfoDrawCount = 0; }
	};

}
