#pragma once

#include "dough/Maths.h"
#include "dough/Core.h"
#include "dough/Window.h"
#include "dough/rendering/IGPUResourceVulkan.h"

namespace DOH {

	class RenderingContextVulkan;

	struct ImGuiInitInfo {
		VkInstance VulkanInstance;
		VkPhysicalDevice PhysicalDevice;
		VkDevice LogicDevice;
		uint32_t QueueFamily; //graphic queue index
		VkQueue Queue;//graphics queue
		uint32_t MinImageCount;
		uint32_t ImageCount;
		VkRenderPass UiRenderPass;
	};

	class ImGuiWrapper : public IGPUResourceVulkan {
	
	private:
		VkDescriptorPool mDescriptorPool = VK_NULL_HANDLE;

	public:
		ImGuiWrapper() = default;
		ImGuiWrapper(const ImGuiWrapper& copy) = delete;
		ImGuiWrapper operator=(const ImGuiWrapper& assignment) = delete;

		void init(Window& window, ImGuiInitInfo& imGuiInit);
		void close(VkDevice logicDevice);
		void uploadFonts(RenderingContextVulkan& context);
		void newFrame();
		void endFrame();
		void render(VkCommandBuffer cmd);
		void onWindowResize(int width, int height) const;
	};
}
