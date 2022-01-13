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
		std::vector<std::reference_wrapper<VertexArrayVulkan>> mVaoDrawList;

		//Sync objects
		const int MAX_FRAMES_IN_FLIGHT = 2;
		size_t mCurrentFrame = 0;
		std::vector<VkSemaphore> mImageAvailableSemaphores;
		std::vector<VkSemaphore> mRenderFinishedSemaphores;
		std::vector<VkFence> mFramesInFlightFences;
		std::vector<VkFence> mImageFencesInFlight;


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

		void init(VkDevice logicDevice);
		void createUniformObjects(VkDevice logicDevice);
		void uploadShaderUniforms(VkDevice logicDevice, VkPhysicalDevice physicalDevice);
		void createSyncObjects(VkDevice logicDevice);
		void closeSyncObjects(VkDevice logicDevice);

		uint32_t aquireNextImageIndex(VkDevice logicDevice);
		void present(VkDevice logicDevice, VkQueue graphicsQueue, VkQueue presentQueue, uint32_t imageIndex);

		void createCommandBuffers(VkDevice logicDevice);
		void recordDrawCommands(uint32_t imageIndex);
		inline void addVaoToDraw(VertexArrayVulkan& vertexArray) { mVaoDrawList.push_back(vertexArray); }
		inline void clearVaoToDraw() { mVaoDrawList.clear(); }
		void bind(VkCommandBuffer cmdBuffer);
		void beginRenderPass(size_t framebufferIndex, VkCommandBuffer cmdBuffer);
		void endRenderPass(VkCommandBuffer cmdBuffer);
		void close(VkDevice logicDevice);
		
		//-----TEMP:: these "singleTime" methods should be asynchronous, see tutorial for better explanation and refresher on why these are being used.
		//TODO:: Make buffer for these and flush it at the end instead of creating one and instantly using/binding it
		//Maybe have a function in RenderingContext class that takes in a GraphicsPipeline to work on so it is pipeline agnostic
		VkCommandBuffer beginSingleTimeCommands(VkDevice logicDevice);
		void endSingleTimeCommands(VkDevice logicDevice, VkCommandBuffer cmdBuffer);


		inline void setCommandPool(VkCommandPool cmdPool) { mCommandPool = cmdPool; }
		inline const VkCommandPool getCommandPool() const { return mCommandPool; }
		inline void setDescriptorPool(VkDescriptorPool descPool) { mDescriptorPool = descPool; }
		inline const VkDescriptorPool getDescriptorPool() const { return mDescriptorPool; }
		inline DescriptorVulkan& getShaderDescriptor() const { return mShaderProgram.getShaderDescriptor(); }
		inline ShaderProgramVulkan& getShaderProgram() const { return mShaderProgram; }

		inline VkPipeline get() const { return mGraphicsPipeline; }
		inline SwapChainVulkan& getSwapChain() const { return *mSwapChain; }
		inline std::vector<VkCommandBuffer>& getCommandBuffers() { return mCommandBuffers; }
	};
}
