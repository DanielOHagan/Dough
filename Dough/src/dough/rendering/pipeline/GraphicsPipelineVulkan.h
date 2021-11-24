#pragma once

#include "dough/rendering/pipeline/SwapChainVulkan.h"
#include "dough/rendering/pipeline/RenderPassVulkan.h"
#include "dough/rendering/pipeline/DescriptorVulkan.h"
#include "dough/rendering/TextureVulkan.h"
#include "dough/rendering/buffer/VertexArrayVulkan.h"
#include "dough/rendering/shader/ShaderProgramVulkan.h"

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

		std::shared_ptr<SwapChainVulkan> mSwapChain;
		std::shared_ptr<RenderPassVulkan> mRenderPass;
		ShaderProgramVulkan& mShaderProgram;

		std::vector<VkCommandBuffer> mCommandBuffers;


	public:
		GraphicsPipelineVulkan() = delete;
		GraphicsPipelineVulkan(const GraphicsPipelineVulkan& copy) = delete;
		GraphicsPipelineVulkan operator=(const GraphicsPipelineVulkan& assignment) = delete;

		GraphicsPipelineVulkan(
			std::shared_ptr<SwapChainVulkan> swapChain,
			std::shared_ptr<RenderPassVulkan> renderPass,
			ShaderProgramVulkan& shaderProgram
		);
		GraphicsPipelineVulkan(
			VkDevice logicDevice,
			VkCommandPool cmdPool,
			SwapChainCreationInfo& swapChainCreate,
			ShaderProgramVulkan& shaderProgram
		);

		void createUniformObjects(VkDevice logicDevice);
		void uploadShaderUniforms(VkDevice logicDevice, VkPhysicalDevice physicalDevice);
		void createCommandBuffers(VkDevice logicDevice, VertexArrayVulkan& vertexArray);

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
		inline DescriptorVulkan& getShaderDescriptor() const { return mShaderProgram.getShaderDescriptor(); }

		inline VkPipeline get() const { return mGraphicsPipeline; }
		inline VkPipelineLayout getPipelineLayout() const { return mGraphicsPipelineLayout; }
		inline SwapChainVulkan& getSwapChain() const { return *mSwapChain; }
		inline RenderPassVulkan& getRenderPass() const { return *mRenderPass; }
		inline std::vector<VkCommandBuffer>& getCommandBuffers() { return mCommandBuffers; }
	};
}
