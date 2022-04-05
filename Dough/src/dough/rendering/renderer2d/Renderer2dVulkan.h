#pragma once

#include "dough/Utils.h"
#include "dough/scene/geometry/Quad.h"
#include "dough/rendering/renderer2d/Renderer2dStorageVulkan.h"
//#include "dough/rendering/SwapChainVulkan.h"

#include <vulkan/vulkan_core.h>

namespace DOH {

	class RenderingContextVulkan;

	class Renderer2dVulkan {

	private:
		RenderingContextVulkan& mContext;
		std::unique_ptr<Renderer2dStorageVulkan> mStorage;

	public:
		Renderer2dVulkan(RenderingContextVulkan& context);
		Renderer2dVulkan(const Renderer2dVulkan& copy) = delete;
		void operator=(const Renderer2dVulkan& assignment) = delete;


		void init(VkDevice logicDevce);
		void close(VkDevice logicDevice);
		void closeSwapChainSpecificObjects(VkDevice logicDevice);
		void recreateSwapChainSpecificObjects(SwapChainVulkan& swapChain);

		void updateSceneUniformData(VkDevice logicDevice, uint32_t currentImage, glm::mat4x4& sceneProjView);
		void flush(VkDevice logicDevice, uint32_t imageIndex, VkCommandBuffer cmd);

		//TODO:: separate flush functions for rendering in different render passes
		//void flushScene();
		//void flushUi();

		void drawQuadScene(Quad& quad);
		//void drawQuadUi(Quad& quad);


		//-----DEBUG----- TEMP:: Storage should only be visible to Renderer2d
		Renderer2dStorageVulkan& getStorage() const { return *mStorage; }
	};

}
