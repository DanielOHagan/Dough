#include "dough/ImGuiWrapper.h"

#include "dough/Utils.h"
#include "dough/rendering/RenderingContextVulkan.h"
#include "dough/Window.h"
#include "dough/Logging.h"

#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_vulkan.h>
#include <vulkan/vulkan.hpp>

#include "tracy/public/tracy/Tracy.hpp"

#define DOH_IMGUI_UI_MAX_TEXTURE_COUNT 1000

namespace DOH {

	const char* ImGuiWrapper::EMPTY_LABEL = "##";
	const char* ImGuiWrapper::TEXTURE_LABEL = "##tex";

	ImGuiWrapper::ImGuiWrapper()
	:	mDescriptorPool(VK_NULL_HANDLE),
		mTextureCount(0),
		mUsingGpuResources(false)
	{}

	void ImGuiWrapper::init(Window& window, ImGuiInitInfo& imGuiInit) {
		ZoneScoped;

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGui::StyleColorsDark();

		createRenderPass(imGuiInit.LogicDevice, imGuiInit.ImageFormat);

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
		setEnabledConfigFlag(EImGuiConfigFlag::DOCKING, true);
		setEnabledConfigFlag(EImGuiConfigFlag::VIEWPORTS, true);
		io.ConfigDockingWithShift = true;
		
		ImGui_ImplGlfw_InitForVulkan(window.getNativeWindow(), true);
		ImGui_ImplVulkan_Init(&initInfo, mRenderPass->get());

		mUsingGpuResources = true;

		mLoadedTextures = {};
	}

	void ImGuiWrapper::close(VkDevice logicDevice) {
		ZoneScoped;

		if (mUsingGpuResources) {
			ImGui_ImplVulkan_Shutdown();
			ImGui_ImplGlfw_Shutdown();
			ImGui::DestroyContext();

			vkDestroyDescriptorPool(logicDevice, mDescriptorPool, nullptr);

			mUsingGpuResources = false;
		}

		closeRenderPass(logicDevice);
		closeFrameBuffers(logicDevice);
	}

	void ImGuiWrapper::uploadFonts(RenderingContextVulkan& context) {
		ZoneScoped;

		VkCommandBuffer cmd = context.beginSingleTimeCommands();
		ImGui_ImplVulkan_CreateFontsTexture(cmd);
		context.endSingleTimeCommands(cmd);

		ImGui_ImplVulkan_DestroyFontUploadObjects();
	}

	void ImGuiWrapper::newFrame() {
		ZoneScoped;

		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	}

	void ImGuiWrapper::endFrame() {
		ZoneScoped;

		ImGui::EndFrame();
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}

	void ImGuiWrapper::beginRenderPass(uint32_t imageIndex, VkExtent2D extent, VkCommandBuffer cmd) {
		mRenderPass->begin(mFrameBuffers[imageIndex], extent, cmd);
	}

	void ImGuiWrapper::render(VkCommandBuffer cmd) {
		ZoneScoped;

		ImGui::Render();
		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);
	}

	void ImGuiWrapper::onWindowResize(int width, int height) const {
		ImGui::GetIO().DisplaySize = ImVec2(static_cast<float>(width), static_cast<float>(height));
	}

	void ImGuiWrapper::setEnabledConfigFlag(const EImGuiConfigFlag configFlag, const bool enabled) {
		enabled ?
			ImGui::GetIO().ConfigFlags |= (ImGuiConfigFlags) configFlag :
			ImGui::GetIO().ConfigFlags &= ~(ImGuiConfigFlags) configFlag;
	}

	void ImGuiWrapper::drawTexture(const TextureVulkan& texture, glm::vec2 size, glm::vec2 uv0, glm::vec2 uv1) {
		ZoneScoped;

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
		ZoneScoped;

		VkDescriptorSet descSet = VK_NULL_HANDLE;
		
		if (mTextureCount < DOH_IMGUI_UI_MAX_TEXTURE_COUNT) {
			descSet = ImGui_ImplVulkan_AddTexture(sampler, imageView, imageLayout);
			mTextureCount++;
		} else {
			LOG_WARN("Failed to add texture to ImGUI, mTextureCount reached the limit");
		}

		return descSet;
	}

	void ImGuiWrapper::createRenderPass(VkDevice logicDevice, VkFormat imageFormat) {
		ZoneScoped;

		SubPassVulkan imGuiSubPass = {
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			{
				{
					ERenderPassAttachmentType::COLOUR,
					imageFormat,
					VK_ATTACHMENT_LOAD_OP_LOAD,
					VK_ATTACHMENT_STORE_OP_STORE,
					VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
					VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
				}
			}
		};

		mRenderPass = std::make_shared<RenderPassVulkan>(logicDevice, imGuiSubPass);
	}

	void ImGuiWrapper::createFrameBuffers(VkDevice logicDevice, const std::vector<VkImageView>& imageViews, VkExtent2D extent) {
		ZoneScoped;

		mFrameBuffers.resize(imageViews.size());

		uint32_t i = 0;
		for (const VkImageView imageView : imageViews) {
			VkImageView imGuiAttachments[] = { imageView };
			VkFramebufferCreateInfo imGuiFrameBufferInfo = {};
			imGuiFrameBufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			imGuiFrameBufferInfo.renderPass = mRenderPass->get();
			imGuiFrameBufferInfo.attachmentCount = 1;
			imGuiFrameBufferInfo.pAttachments = imGuiAttachments;
			imGuiFrameBufferInfo.width = extent.width;
			imGuiFrameBufferInfo.height = extent.height;
			imGuiFrameBufferInfo.layers = 1;

			VK_TRY(
				vkCreateFramebuffer(logicDevice, &imGuiFrameBufferInfo, nullptr, &mFrameBuffers[i]),
				"Failed to create ImGui FrameBuffer."
			);

			i++;
		}
	}

	void ImGuiWrapper::closeFrameBuffers(VkDevice logicDevice) {
		ZoneScoped;

		for (const VkFramebuffer& frameBuffer : mFrameBuffers) {
			vkDestroyFramebuffer(logicDevice, frameBuffer, nullptr);
		}
	}

	void ImGuiWrapper::closeRenderPass(VkDevice logicDevice) {
		mRenderPass->close(logicDevice);
	}
}
