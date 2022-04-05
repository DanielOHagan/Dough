#include "dough/ImGuiWrapper.h"
#include "dough/Utils.h"
#include "dough/rendering/RenderingContextVulkan.h"
#include "dough/Window.h"

#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_vulkan.h>

namespace DOH {

	void ImGuiWrapper::init(Window& window, ImGuiInitInfo& imGuiInit) {
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGui::StyleColorsDark();

		ImGui_ImplVulkan_InitInfo initInfo = {};
		initInfo.Instance = imGuiInit.VulkanInstance;
		initInfo.PhysicalDevice = imGuiInit.PhysicalDevice;
		initInfo.Device = imGuiInit.LogicDevice;
		initInfo.PipelineCache = VK_NULL_HANDLE;
		initInfo.Allocator = nullptr;
		initInfo.QueueFamily = imGuiInit.QueueFamily;
		initInfo.Queue = imGuiInit.Queue;
		initInfo.MinImageCount = imGuiInit.MinImageCount;
		initInfo.ImageCount = imGuiInit.ImageCount;
		initInfo.CheckVkResultFn = nullptr;

		VkDescriptorPoolSize pool_sizes[] =
		{
			{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
		};
		VkDescriptorPoolCreateInfo pool_info = {};
		pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		pool_info.maxSets = 1000 * IM_ARRAYSIZE(pool_sizes);
		pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
		pool_info.pPoolSizes = pool_sizes;
		VK_TRY(
			vkCreateDescriptorPool(imGuiInit.LogicDevice, &pool_info, nullptr, &mDescriptorPool),
			"Failed to create ImGui descriptor pool"
		);

		initInfo.DescriptorPool = mDescriptorPool;

		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		
		ImGui_ImplGlfw_InitForVulkan(window.getNativeWindow(), true);
		ImGui_ImplVulkan_Init(&initInfo, imGuiInit.UiRenderPass);

		mUsingGpuResource = true;
	}

	void ImGuiWrapper::close(VkDevice logicDevice) {
		if (isUsingGpuResource()) {
			ImGui_ImplVulkan_Shutdown();
			ImGui_ImplGlfw_Shutdown();
			ImGui::DestroyContext();

			vkDestroyDescriptorPool(logicDevice, mDescriptorPool, nullptr);

			mUsingGpuResource = false;
		}
	}

	void ImGuiWrapper::uploadFonts(RenderingContextVulkan& context) {
		VkCommandBuffer cmd = context.beginSingleTimeCommands();
		ImGui_ImplVulkan_CreateFontsTexture(cmd);
		context.endSingleTimeCommands(cmd);

		ImGui_ImplVulkan_DestroyFontUploadObjects();
	}

	void ImGuiWrapper::newFrame() {
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	}

	void ImGuiWrapper::endFrame() {
		ImGui::EndFrame();
	}

	void ImGuiWrapper::render(VkCommandBuffer cmd) {
		ImGuiIO& io = ImGui::GetIO();
		ImGui::Render();
		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);
	}

	void ImGuiWrapper::onWindowResize(int width, int height) const {
		ImGui::GetIO().DisplaySize = ImVec2((float) width, (float)height);
	}
}
