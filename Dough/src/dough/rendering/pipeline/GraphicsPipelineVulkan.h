#pragma once

#include "dough/Utils.h"

#include "dough/rendering/pipeline/SwapChainVulkan.h"
#include "dough/rendering/pipeline/RenderPassVulkan.h"
#include "dough/rendering/pipeline/DescriptorVulkan.h"
#include "dough/rendering/TextureVulkan.h"

namespace DOH {

	class GraphicsPipelineVulkan {

	private:
		
		VkPipeline mGraphicsPipeline;
		VkPipelineLayout mGraphicsPipelineLayout;
		//NOTE:: Command Pool may be used by other Pipeline
		//			instances so its lifetime is not matched to this
		VkCommandPool mCommandPool;
		//NOTE:: Descriptor Pool may be used by other Pipeline
		//			instances so its lifetime is not matched to this
		VkDescriptorPool mDescriptorPool;

		SwapChainVulkan mSwapChain;
		//std::unique_ptr<SwapChainVulkan> mSwapChain;
		RenderPassVulkan mRenderPass;

		std::vector<VkCommandBuffer> mCommandBuffers;

		DescriptorVulkan mShaderDescriptor;


		std::string mVertShaderPath;
		std::string mFragShaderPath;

	public:
		GraphicsPipelineVulkan();
		GraphicsPipelineVulkan(
			SwapChainVulkan& swapChain,
			RenderPassVulkan renderPass,
			std::string& mVertShaderPath,
			std::string& mFragShaderPath
		);

		void createUniformBufferObject(VkDevice logicDevice, VkDeviceSize bufferSize);
		void uploadShaderUBO(VkDevice logicDevice, VkPhysicalDevice physicalDevice, TextureVulkan& texture);
		void createCommandBuffers(VkDevice logicDevice /*TEMP::*/, VkBuffer vertexBuffer, VkBuffer indexBuffer, uint32_t indexCount/*::TEMP*/);
		void init(VkDevice logicDevice);
		void bind(VkCommandBuffer cmdBuffer);
		void beginRenderPass(size_t framebufferIndex, VkCommandBuffer cmdBuffer);
		void endRenderPass(VkCommandBuffer cmdBuffer);
		void close(VkDevice logicDevice);
		
		//-----TEMP:: these "singleTime" methods should be asynchronous, see tutorial for better explanation and refresher on why these are being used.
		//TODO:: Make buffer for these and flush it at the end instead of creating one and instantly using/binding it
		VkCommandBuffer beginSingleTimeCommands(VkDevice logicDevice);
		void endSingleTimeCommands(VkDevice logicDevice, VkCommandBuffer cmdBuffer);


		inline void setCommandPool(VkCommandPool cmdPool) { mCommandPool = cmdPool; }
		inline const VkCommandPool getCommandPool() const { return mCommandPool; }
		inline void setDescriptorPool(VkDescriptorPool descPool) { mDescriptorPool = descPool; }
		inline const VkDescriptorPool getDescriptorPool() const { return mDescriptorPool; }
		inline DescriptorVulkan& getShaderDescriptor() { return /*mUniformDescriptor*/ mShaderDescriptor; }

		inline VkPipeline get() const { return mGraphicsPipeline; }
		inline VkPipelineLayout getPipelineLayout() const { return mGraphicsPipelineLayout; }
		inline SwapChainVulkan getSwapChain() const { return mSwapChain; }
		inline RenderPassVulkan getRenderPass() const { return mRenderPass; }
		inline std::vector<VkCommandBuffer>& getCommandBuffers() { return mCommandBuffers; }

	public:
		//TODO:: Uniform info including DescriptorSetLayout creation args, size, and more
		
		static GraphicsPipelineVulkan create(
			VkDevice logicDevice,
			SwapChainSupportDetails scsd,
			VkSurfaceKHR surface,
			QueueFamilyIndices& indices,
			uint32_t width,
			uint32_t height,
			std::string& vertShaderPath,
			std::string& fragShaderPath,
			VkCommandPool cmdPool,
			VkDeviceSize uniformBufferSize
		);

		static GraphicsPipelineVulkan createNonInit();
	};
}
