#include "dough/rendering/Config.h"
#include "dough/rendering/pipeline/RenderPassVulkan.h"
#include "dough/rendering/ImageVulkan.h"

namespace DOH {

	class SwapChainVulkan {

	private:

		SwapChainSupportDetails& mSwapChainSupportDetails;

		VkSwapchainKHR mSwapChain;
		std::vector<VkImage> mImages;
		std::vector<VkImageView> mImageViews;
		VkFormat mImageFormat;
		VkExtent2D mExtent;
		
		std::shared_ptr<RenderPassVulkan> mSceneRenderPass;
		std::vector<VkFramebuffer> mSceneFrameBuffers;
		std::vector<ImageVulkan> mSceneDepthImages;

		std::shared_ptr<RenderPassVulkan> mAppUiRenderPass;
		std::vector<VkFramebuffer> mAppUiFrameBuffers;
		
		std::shared_ptr<RenderPassVulkan> mImGuiRenderPass;
		std::vector<VkFramebuffer> mImGuiFrameBuffers;

		bool mResizable;

	public:
		enum class ERenderPassType {
			SCENE,
			APP_UI,
			IMGUI
		};

		SwapChainVulkan(
			VkDevice logicDevice,
			SwapChainSupportDetails& scsd,
			VkSurfaceKHR surface,
			QueueFamilyIndices& indices,
			uint32_t width,
			uint32_t height
		);
		SwapChainVulkan(VkDevice logicDevice, SwapChainCreationInfo& creationInfo);
		SwapChainVulkan(const SwapChainVulkan& copy) = delete;
		SwapChainVulkan operator=(const SwapChainVulkan& assignment) = delete;

		void init(
			VkDevice logicDevice,
			SwapChainSupportDetails& swapChainSupportDetails,
			VkSurfaceKHR surface,
			QueueFamilyIndices& indices,
			uint32_t width,
			uint32_t height
		);
		void close(VkDevice logicDevice);

		void createFrameBuffers(VkDevice logicDevice);
		void destroyFrameBuffers(VkDevice logicDevice);
		uint32_t aquireNextImageIndex(
			VkDevice logicDevice,
			VkFence frameInFlightFence,
			VkSemaphore imageAvailableSemaphore
		);
		void beginRenderPass(ERenderPassType type, size_t frameBufferIndex, VkCommandBuffer cmd);
		void resize();

		inline void setResizable(bool resizable) { mResizable = resizable; }
		inline bool isResizable() const { return mResizable; }
		inline float getAspectRatio() const { return (float) mExtent.width / mExtent.height; }

		inline VkSwapchainKHR get() const { return mSwapChain; }
		inline VkExtent2D getExtent() const { return mExtent; }
		inline VkFormat getImageFormat() const { return mImageFormat; }
		inline uint32_t getImageCount() const { return static_cast<uint32_t>(mImages.size()); }
		inline uint32_t getImageViewCount() const { return static_cast<uint32_t>(mImageViews.size()); }
		//IMPORTANT:: ImGui frame buffers are not included as ImGui handles that itself
		inline uint32_t getFrameBufferCount() const { return static_cast<uint32_t>(mSceneFrameBuffers.size() + mAppUiFrameBuffers.size()); }
		RenderPassVulkan& getRenderPass(ERenderPassType type) const;

		//-----Static Methods-----
		static SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);
		static VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		static VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
		static VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, uint32_t width, uint32_t height);

	private:
		void createImageViews(VkDevice logicDevice);
	};
}
