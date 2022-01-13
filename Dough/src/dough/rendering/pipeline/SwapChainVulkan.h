#include "dough/rendering/Config.h"

namespace DOH {

	class SwapChainVulkan {

	private:

		SwapChainSupportDetails mSwapChainSupportDetails;

		VkSwapchainKHR mVkSwapChain;
		std::vector<VkImage> mVkSwapChainImages;
		VkFormat mVkSwapChainImageFormat;
		VkExtent2D mVkSwapChainExtent;
		std::vector<VkImageView> mVkSwapChainImageViews;
		std::vector<VkFramebuffer> mVkSwapChainFramebuffers;

		bool mResizable;

	public:
		SwapChainVulkan(
			VkDevice logicDevice,
			SwapChainSupportDetails scsd,
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

		void createFramebuffers(VkDevice logicDevice, VkRenderPass renderPass);
		void destroyFramebuffers(VkDevice logicDevice);

		inline void setResizable(bool resizable) { mResizable = resizable; }
		inline bool isResizable() const { return mResizable; }
		inline float getAspectRatio() const { return (float) mVkSwapChainExtent.width / mVkSwapChainExtent.height; }

		inline VkSwapchainKHR get() const { return mVkSwapChain; }
		VkFramebuffer getFramebufferAt(size_t index) const { return mVkSwapChainFramebuffers.at(index); }
		inline VkExtent2D getExtent() const { return mVkSwapChainExtent; }
		inline VkFormat getImageFormat() const { return mVkSwapChainImageFormat; }

		inline size_t getImageCount() const { return mVkSwapChainImages.size(); }
		inline size_t getImageViewCount() const { return mVkSwapChainImageViews.size(); }
		inline size_t getFramebufferCount() const { return mVkSwapChainFramebuffers.size(); }

		//-----Static Methods-----
		static SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);
		static VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		static VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
		static VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, uint32_t width, uint32_t height);

	private:
		void createImageViews(VkDevice logicDevice);
	};
}
