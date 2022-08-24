#include "dough/ImGuiWrapper.h"
#include "dough/Utils.h"
#include "dough/rendering/RenderingContextVulkan.h"
#include "dough/Window.h"
#include "dough/Logging.h"

#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_vulkan.h>

namespace DOH {

	const char* ImGuiWrapper::EMPTY_LABEL = "##";

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
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
		io.ConfigDockingWithShift = true;
		
		ImGui_ImplGlfw_InitForVulkan(window.getNativeWindow(), true);
		ImGui_ImplVulkan_Init(&initInfo, imGuiInit.RenderPass);

		mUsingGpuResource = true;

		mLoadedTextures = {};
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
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}

	void ImGuiWrapper::render(VkCommandBuffer cmd) {
		ImGuiIO& io = ImGui::GetIO();
		ImGui::Render();
		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);
	}

	void ImGuiWrapper::onWindowResize(int width, int height) const {
		ImGui::GetIO().DisplaySize = ImVec2((float) width, (float)height);
	}

	void ImGuiWrapper::drawTexture(const TextureVulkan& texture, glm::vec2 size, glm::vec2 uv0, glm::vec2 uv1) {
		const auto& begin = mLoadedTextures.find(texture.getId());
		VkDescriptorSet descSet = VK_NULL_HANDLE;

		if (begin != mLoadedTextures.end()) {
			descSet = begin->second;
		} else {
			descSet = addTextureVulkan(
				texture.getSampler(),
				texture.getImageView(),
				VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
			);

			if (descSet == VK_NULL_HANDLE) {
				LOG_ERR("Failed to add texture to ImGui");
				return;
			}

			mLoadedTextures.emplace(texture.getId(), descSet);
		}

		//TODO:: add a default texture to display in the event the desired texture cannot be displayed?
		// Or log/display text

		if (descSet == VK_NULL_HANDLE) {
			LOG_ERR("Unable to draw texture in ImGui");
			return;
		}

		ImGui::Image(
			descSet,
			ImVec2(size.x, size.y),
			ImVec2(uv0.x, uv0.y),
			ImVec2(uv1.x, uv1.y)
		);
	}

	VkDescriptorSet ImGuiWrapper::addTextureVulkan(VkSampler sampler, VkImageView imageView, VkImageLayout imageLayout) {
		return ImGui_ImplVulkan_AddTexture(sampler, imageView, imageLayout);
	}
}
