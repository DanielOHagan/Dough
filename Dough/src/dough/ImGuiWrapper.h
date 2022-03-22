#pragma once

#include "dough/Maths.h"
#include "dough/Core.h"
#include "dough/Window.h"

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

	class ImGuiWrapper {
	
	private:
		struct PushConstantBlock {
			glm::vec2 Scale;
			glm::vec2 Translate;
		} mPushConstantBlock;

		std::unique_ptr<std::string> mImGuiShaderVertPath = std::make_unique<std::string>("res/shaders/spv/ImGui.vert.spv");
		std::unique_ptr<std::string> mImGuiShaderFragPath = std::make_unique<std::string>("res/shaders/spv/ImGui.frag.spv");

		//VkCommandPool mCommandPool = VK_NULL_HANDLE;
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

		inline const PushConstantBlock& getPushConstantBlock() const { return mPushConstantBlock; }
	};
}
