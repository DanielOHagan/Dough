#pragma once

#include "dough/Maths.h"
#include "dough/Core.h"
#include "dough/Window.h"
#include "dough/rendering/IGPUResourceVulkan.h"
#include "dough/rendering/textures/TextureVulkan.h"

#include <imgui/imgui.h>

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
		VkRenderPass RenderPass;
	};

	enum class EImGuiConfigFlag {
		NONE = 0,

		DOCKING = ImGuiConfigFlags_DockingEnable,
		VIEWPORTS = ImGuiConfigFlags_ViewportsEnable
	};

	class ImGuiWrapper : public IGPUResourceVulkan {

		//TODO:: 
		// Interacting with "detatched" ImGui windows does NOT keep the main window focused
		//
		// Handle events that happen when ImGui is focused (sometimes it might be preferable
		//	that imgui is focused and allow for certain functions in app e.g. camera movement)
		//
		// If Viewports are enabled and one is dragged out of the main window, therefore, creating a new window,
		//	and then the main window display mode is changed to fullscreen, the program crashes.

	private:
		VkDescriptorPool mDescriptorPool = VK_NULL_HANDLE;
		std::unordered_map<uint32_t, VkDescriptorSet> mLoadedTextures;

		//IMPORTANT:: ImGui has not confirmed exactly how this works in vulkan, currently images are drawn from
		// a descriptor set created by ImGui
		// Nor is it obvious how it handles removing of said textures as there is no ...removeTexture() or ...destroyDescPool
		VkDescriptorSet addTextureVulkan(VkSampler sampler, VkImageView imageView, VkImageLayout imageLayout);

	public:

		static const char* EMPTY_LABEL;

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
		void setEnabledConfigFlag(const EImGuiConfigFlag configFlag, const bool enabled);

		void drawTexture(const TextureVulkan& texture, glm::vec2 size, glm::vec2 uv0 = { 0.0f, 0.0f }, glm::vec2 uv1 = { 1.0f, 1.0f });
	};
}
