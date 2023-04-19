#pragma once

#include "dough/Utils.h"
#include "dough/rendering/renderer2d/Renderer2dStorageVulkan.h"
#include "dough/rendering/SwapChainVulkan.h"
#include "dough/scene/geometry/collections/TextString.h"

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
		Renderer2dVulkan operator=(const Renderer2dVulkan& assignment) = delete;

		void init();
		void close();
		void onSwapChainResize(SwapChainVulkan& swapChain);

		void updateRenderer2dUniformData(uint32_t currentImage, glm::mat4x4& sceneProjView);
		void flushScene(uint32_t imageIndex, VkCommandBuffer cmd, CurrentBindingsState& currentBindings);
		//TODO:: separate flush functions for rendering in different render passes
		//void flushUi();

		//Close the dynamic batches that have a geo count of 0
		void closeEmptyQuadBatches();


		//-----Primitive Objects-----
		void drawQuadScene(const Quad& quad);
		void drawQuadTexturedScene(const Quad& quad);
		void drawQuadArrayScene(const std::vector<Quad>& quadArr);
		void drawQuadArrayTexturedScene(const std::vector<Quad>& quadArr);
		void drawQuadArraySameTextureScene(const std::vector<Quad>& quadArr);
		//void drawQuadUi(Quad& quad);

		void drawTextFromQuads(const std::vector<Quad>& quadArr);
		void drawTextSameTextureFromQuads(const std::vector<Quad>& quadArr);


		//-----Collection Objects-----
		void drawTextString(TextString& string);

		

		//-----DEBUG----- Used to show info to ImGui in app
		inline Renderer2dStorageVulkan& getStorage() const { return *mStorage; }
		inline size_t getQuadBatchCount() const { return mStorage->getQuadRenderBatches().size(); }
		inline uint32_t getDrawCount() const { return mDebugInfoDrawCount; }
		inline uint32_t getDrawnQuadCount() const { return mDrawnQuadCount; }
		inline uint32_t getTruncatedQuadCount() const { return mTruncatedQuadCount; }
		inline void resetDrawCount() { mDebugInfoDrawCount = 0; }
	};
}
