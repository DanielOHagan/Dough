#pragma once

#include "dough/Utils.h"

#include "dough/rendering/SwapChainVulkan.h"
#include "dough/rendering/RenderPassVulkan.h"
#include "dough/rendering/DescriptorVulkan.h"

#include <vulkan/vulkan_core.h>

namespace DOH {

	class GraphicsPipelineVulkan {

	private:
		
		VkPipeline mGraphicsPipeline;
		VkPipelineLayout mGraphicsPipelineLayout;

		SwapChainVulkan mSwapChain;
		RenderPassVulkan mRenderPass;

		//NOTE:: Command Pool may be used by other Pipeline
		//			instances so its lifetime is not matched to this
		VkCommandPool mCommandPool;
		//NOTE:: Descriptor Pool may be used by other Pipeline
		//			instances so its lifetime is not matched to this
		VkDescriptorPool mDescriptorPool;


		std::vector<VkCommandBuffer> mCommandBuffers;

		DescriptorVulkan mUniformDescriptor;

		std::string mVertShaderPath;
		std::string mFragShaderPath;

	public:
		GraphicsPipelineVulkan();
		GraphicsPipelineVulkan(
			SwapChainVulkan swapChain,
			RenderPassVulkan renderPass,
			std::string& mVertShaderPath,
			std::string& mFragShaderPath
		);

		void createUniformBufferObject(VkDevice logicDevice, VkDeviceSize bufferSize);
		void uploadShaderUBO(VkDevice logicDevice, VkPhysicalDevice physicalDevice);
		void createCommandBuffers(VkDevice logicDevice /*TEMP::*/, VkBuffer vertexBuffer, VkBuffer indexBuffer, uint32_t indexCount/*::TEMP*/);
		void init(VkDevice logicDevice);
		void bind(VkCommandBuffer cmdBuffer);
		void beginRenderPass(size_t framebufferIndex, VkCommandBuffer cmdBuffer);
		void endRenderPass(VkCommandBuffer cmdBuffer);
		void close(VkDevice logicDevice);

		inline const std::string& getVertShaderPath() const { return mVertShaderPath; }
		inline const std::string& getFragShaderPath() const { return mFragShaderPath; }
		inline void setVertShaderPath(std::string& vertShaderPath) { mVertShaderPath = vertShaderPath; }
		inline void setFragShaderPath(std::string& fragShaderPath) { mFragShaderPath = fragShaderPath; }

		inline void setCommandPool(VkCommandPool cmdPool) { mCommandPool = cmdPool; }
		inline const VkCommandPool getCommandPool() const { return mCommandPool; }
		inline void setDescriptorPool(VkDescriptorPool descPool) { mDescriptorPool = descPool; }
		inline const VkDescriptorPool getDescriptorPool() const { return mDescriptorPool; }
		inline DescriptorVulkan& getUniformDescriptor() { return mUniformDescriptor; }

		inline VkPipeline get() const { return mGraphicsPipeline; }
		inline VkPipelineLayout getPipelineLayout() const { return mGraphicsPipelineLayout; }
		inline SwapChainVulkan getSwapChain() const { return mSwapChain; }
		inline RenderPassVulkan getRenderPass() const { return mRenderPass; }
		inline std::vector<VkCommandBuffer>& getCommandBuffers() { return mCommandBuffers; }

	public:
		//TODO:: Uniform info including DescriptorSetLayout creation args, size, and more
		static void create(
			VkDevice logicDevice,
			SwapChainSupportDetails scsd,
			VkSurfaceKHR surface,
			QueueFamilyIndices& indices,
			uint32_t width,
			uint32_t height,
			std::string& vertShaderPath,
			std::string& fragShaderPath,
			VkCommandPool cmdPool,
			VkDeviceSize uniformBufferSize,
			GraphicsPipelineVulkan* dst
		);

		static GraphicsPipelineVulkan createNonInit();
	};
}
