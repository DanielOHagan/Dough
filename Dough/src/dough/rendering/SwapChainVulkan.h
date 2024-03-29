#pragma once

#include "dough/rendering/Config.h"
#include "dough/rendering/RenderPassVulkan.h"
#include "dough/rendering/ImageVulkan.h"

namespace DOH {

	class SwapChainVulkan : public IGPUResourceVulkan {

	private:
		VkSwapchainKHR mSwapChain;
		std::vector<VkImage> mImages;
		std::vector<VkImageView> mImageViews;
		VkFormat mImageFormat;
		VkExtent2D mExtent;
		bool mResizable;

	public:
		SwapChainVulkan(VkDevice logicDevice, SwapChainCreationInfo& creationInfo);
		SwapChainVulkan(const SwapChainVulkan& copy) = delete;
		SwapChainVulkan operator=(const SwapChainVulkan& assignment) = delete;

		virtual ~SwapChainVulkan() override;
		virtual void close(VkDevice logicDevice) override;

		void init(VkDevice logicDevice, const SwapChainCreationInfo& scCreate);
		void resize(VkDevice logicDevice, const SwapChainCreationInfo& scCreate);

		uint32_t aquireNextImageIndex(
			VkDevice logicDevice,
			VkFence frameInFlightFence,
			VkSemaphore imageAvailableSemaphore
		);

		inline void setResizable(bool resizable) { mResizable = resizable; }
		inline bool isResizable() const { return mResizable; }
		inline float getAspectRatio() const { return (float) mExtent.width / mExtent.height; }

		inline const VkSwapchainKHR get() const { return mSwapChain; }
		inline const VkExtent2D getExtent() const { return mExtent; }
		inline const VkFormat getImageFormat() const { return mImageFormat; }
		inline const uint32_t getImageCount() const { return static_cast<uint32_t>(mImages.size()); }
		inline const uint32_t getImageViewCount() const { return static_cast<uint32_t>(mImageViews.size()); }
		inline const std::vector<VkImageView>& getImageViews() const { return mImageViews; }

		//-----Static Methods-----
		static SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);
		static VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		static VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes, VkPresentModeKHR desiredPresentMode, bool fallbackToImmediatePresentMode);
		static VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, uint32_t width, uint32_t height);

	private:
		void createSwapChainKHR(VkDevice logicDevice, const SwapChainCreationInfo& scCreate);
	};
}
