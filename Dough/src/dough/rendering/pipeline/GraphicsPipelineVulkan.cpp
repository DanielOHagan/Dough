#include "dough/rendering/pipeline/GraphicsPipelineVulkan.h"

#include "dough/rendering/buffer/BufferVulkan.h"
#include "dough/rendering/shader/ShaderVulkan.h"
#include "dough/rendering/ObjInit.h"

namespace DOH {

	GraphicsPipelineVulkan::GraphicsPipelineVulkan(
		std::shared_ptr<SwapChainVulkan> swapChain,
		std::shared_ptr<RenderPassVulkan> renderPass,
		ShaderProgramVulkan& shaderProgram
	) : mGraphicsPipeline(VK_NULL_HANDLE),
		mGraphicsPipelineLayout(VK_NULL_HANDLE),
		mSwapChain(swapChain),
		mRenderPass(renderPass),
		mShaderProgram(shaderProgram),
		mCommandPool(VK_NULL_HANDLE),
		mDescriptorPool(VK_NULL_HANDLE)
	{}

	GraphicsPipelineVulkan::GraphicsPipelineVulkan(
		VkDevice logicDevice,
		VkCommandPool cmdPool,
		SwapChainCreationInfo& swapChainCreate,
		ShaderProgramVulkan& shaderProgram
	) : mGraphicsPipeline(VK_NULL_HANDLE),
		mGraphicsPipelineLayout(VK_NULL_HANDLE),
		mShaderProgram(shaderProgram),
		mCommandPool(cmdPool),
		mDescriptorPool(VK_NULL_HANDLE)
	{
		mSwapChain = ObjInit::swapChain(swapChainCreate);
		mRenderPass = ObjInit::renderPass(mSwapChain->getImageFormat());

		createUniformObjects(logicDevice);
		init(logicDevice);
		createCommandBuffers(logicDevice);
		createSyncObjects(logicDevice);
	}

	void GraphicsPipelineVulkan::createUniformObjects(VkDevice logicDevice) {
		ShaderUniformLayout& layout = mShaderProgram.getUniformLayout();

		layout.initDescriptorSetLayoutBindings(layout.getTotalUniformCount());
		std::vector<VkDescriptorSetLayoutBinding>::iterator layoutBindingIter = layout.getDescriptorSetLayoutBindings().begin();

		//Create values' layout binding
		for (const auto& [binding, value] : layout.getValueUniformMap()) {
			layout.getDescriptorSetLayoutBindings()[binding] = DescriptorVulkan::createLayoutBinding(
				VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
				VK_SHADER_STAGE_VERTEX_BIT,
				1,
				binding
			);
		}
		
		//Create textures' layout binding
		for (const auto& [binding, value] : layout.getTextureUniformMap()) {
			layout.getDescriptorSetLayoutBindings()[binding] = DescriptorVulkan::createLayoutBinding(
				VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
				VK_SHADER_STAGE_FRAGMENT_BIT,
				1,
				binding
			);
		}

		mShaderProgram.getShaderDescriptor().createDescriptorSetLayout(logicDevice);
	}

	void GraphicsPipelineVulkan::createSyncObjects(VkDevice logicDevice) {
		mImageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		mRenderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		mFramesInFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
		mImageFencesInFlight.resize(mSwapChain->getImageCount(), VK_NULL_HANDLE);

		VkSemaphoreCreateInfo semaphore = {};
		semaphore.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fence = {};
		fence.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fence.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			TRY(
				vkCreateSemaphore(logicDevice, &semaphore, nullptr, &mImageAvailableSemaphores[i]) != VK_SUCCESS ||
				vkCreateSemaphore(logicDevice, &semaphore, nullptr, &mRenderFinishedSemaphores[i]) != VK_SUCCESS,
				"Failed to create Semaphores for a frame."
			);

			TRY(
				vkCreateFence(logicDevice, &fence, nullptr, &mFramesInFlightFences[i]) != VK_SUCCESS,
				"Failed to create Fence."
			);
		}
	}

	void GraphicsPipelineVulkan::closeSyncObjects(VkDevice logicDevice) {
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			vkDestroySemaphore(logicDevice, mImageAvailableSemaphores[i], nullptr);
			vkDestroySemaphore(logicDevice, mRenderFinishedSemaphores[i], nullptr);
			vkDestroyFence(logicDevice, mFramesInFlightFences[i], nullptr);
		}
	}

	uint32_t GraphicsPipelineVulkan::aquireNextImageIndex(VkDevice logicDevice) {
		vkWaitForFences(logicDevice, 1, &mFramesInFlightFences[mCurrentFrame], VK_TRUE, UINT64_MAX);
		
		uint32_t imageIndex;
		vkAcquireNextImageKHR(
			logicDevice,
			mSwapChain->get(),
			UINT64_MAX,
			mImageAvailableSemaphores[mCurrentFrame],
			VK_NULL_HANDLE,
			&imageIndex
		);

		return imageIndex;
	}

	void GraphicsPipelineVulkan::present(VkDevice logicDevice, VkQueue graphicsQueue, VkQueue presentQueue, uint32_t imageIndex) {
		if (mImageFencesInFlight[imageIndex] != VK_NULL_HANDLE) {
			vkWaitForFences(logicDevice, 1, &mImageFencesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
		}

		//Mark the image as now being in use by this frame
		mImageFencesInFlight[imageIndex] = mFramesInFlightFences[mCurrentFrame];

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkSemaphore waitSemaphores[] = { mImageAvailableSemaphores[mCurrentFrame] };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &mCommandBuffers[imageIndex];

		VkSemaphore signalSemaphores[] = { mRenderFinishedSemaphores[mCurrentFrame] };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		vkResetFences(logicDevice, 1, &mFramesInFlightFences[mCurrentFrame]);

		TRY(
			vkQueueSubmit(graphicsQueue, 1, &submitInfo, mFramesInFlightFences[mCurrentFrame]) != VK_SUCCESS,
			"Failed to submit Draw Command Buffer."
		);

		VkPresentInfoKHR present = {};
		present.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		present.waitSemaphoreCount = 1;
		present.pWaitSemaphores = signalSemaphores;

		VkSwapchainKHR swapChains[] = { mSwapChain->get() };
		present.swapchainCount = 1;
		present.pSwapchains = swapChains;
		present.pImageIndices = &imageIndex;

		vkQueuePresentKHR(presentQueue, &present);

		//TODO:: Check result of vkQueuePresentKHR here

		mCurrentFrame = (mCurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

	}

	void GraphicsPipelineVulkan::uploadShaderUniforms(VkDevice logicDevice, VkPhysicalDevice physicalDevice) {
		const size_t count = mSwapChain->getImageCount();
		DescriptorVulkan& desc = mShaderProgram.getShaderDescriptor();
		desc.createValueBuffers(logicDevice, physicalDevice, count);
		desc.createDescriptorSets(logicDevice, count, mDescriptorPool);
		desc.updateDescriptorSets(logicDevice, count);
	}

	void GraphicsPipelineVulkan::createCommandBuffers(VkDevice logicDevice) {
		mCommandBuffers.resize(mSwapChain->getFramebufferCount());

		VkCommandBufferAllocateInfo cmdBuffAlloc = {};
		cmdBuffAlloc.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		cmdBuffAlloc.commandPool = mCommandPool;
		cmdBuffAlloc.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		cmdBuffAlloc.commandBufferCount = (uint32_t)mCommandBuffers.size();

		TRY(
			vkAllocateCommandBuffers(logicDevice, &cmdBuffAlloc, mCommandBuffers.data()) != VK_SUCCESS,
			"Failed to allocate Command Buffers."
		);
	}

	void GraphicsPipelineVulkan::recordDrawCommands(uint32_t imageIndex) {
		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		TRY(
			vkBeginCommandBuffer(mCommandBuffers[imageIndex], &beginInfo) != VK_SUCCESS,
			"Failed to begin recording Command Buffer."
		);

		beginRenderPass(imageIndex, mCommandBuffers[imageIndex]);

		bind(mCommandBuffers[imageIndex]);

		for (VertexArrayVulkan& vao : mVaoDrawList) {
			vao.bind(mCommandBuffers[imageIndex]);
			mShaderProgram.getShaderDescriptor().bindDescriptorSets(mCommandBuffers[imageIndex], mGraphicsPipelineLayout, imageIndex);

			vkCmdDrawIndexed(
				mCommandBuffers[imageIndex],
				vao.getIndexBuffer().getCount(),
				1,
				0,
				0,
				0
			);
		}

		endRenderPass(mCommandBuffers[imageIndex]);

		TRY(
			vkEndCommandBuffer(mCommandBuffers[imageIndex]) != VK_SUCCESS,
			"Failed to record Command Buffer."
		);
	}

	void GraphicsPipelineVulkan::init(VkDevice logicDevice) {
		mShaderProgram.loadModules(logicDevice);

		TRY(!mShaderProgram.areShadersLoaded(), "Shader Modules not loaded");

		VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = mShaderProgram.getVertexShader().getShaderModule();
		vertShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = mShaderProgram.getFragmentShader().getShaderModule();
		fragShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

		VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

		VkVertexInputBindingDescription bindDesc = Vertex::getBindingDescription();
		auto attribDescs = Vertex::getAttributeDescriptions();

		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.pVertexBindingDescriptions = &bindDesc;
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attribDescs.size());
		vertexInputInfo.pVertexAttributeDescriptions = attribDescs.data();

		VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		VkViewport viewport = {};
		viewport.x = 0.0f;
		viewport.y = 0;
		viewport.width = (float) mSwapChain->getExtent().width;
		viewport.height = (float) mSwapChain->getExtent().height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissorRectangle = {};
		scissorRectangle.offset = {0, 0};
		scissorRectangle.extent = mSwapChain->getExtent();

		VkPipelineViewportStateCreateInfo viewportState = {};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.pViewports = &viewport;
		viewportState.scissorCount = 1;
		viewportState.pScissors = &scissorRectangle;

		VkPipelineRasterizationStateCreateInfo rasterizer = {};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;

		VkPipelineMultisampleStateCreateInfo multisampling = {};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		VkPipelineColorBlendAttachmentState colourBlendAttachment = {};
		colourBlendAttachment.colorWriteMask =
			VK_COLOR_COMPONENT_R_BIT |
			VK_COLOR_COMPONENT_G_BIT |
			VK_COLOR_COMPONENT_B_BIT |
			VK_COLOR_COMPONENT_A_BIT;
		colourBlendAttachment.blendEnable = VK_FALSE;

		VkPipelineColorBlendStateCreateInfo colourBlending = {};
		colourBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colourBlending.logicOpEnable = VK_FALSE;
		colourBlending.attachmentCount = 1;
		colourBlending.pAttachments = &colourBlendAttachment;

		//NOTE:: Read about Dynamic States

		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
		pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutCreateInfo.setLayoutCount = 1;
		pipelineLayoutCreateInfo.pSetLayouts = &mShaderProgram.getShaderDescriptor().getDescriptorSetLayout();

		TRY(
			vkCreatePipelineLayout(logicDevice, &pipelineLayoutCreateInfo, nullptr, &mGraphicsPipelineLayout) != VK_SUCCESS,
			"Failed to create Pipeline Layout."
		);

		VkGraphicsPipelineCreateInfo pipelineCreateInfo = {};
		pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineCreateInfo.stageCount = 2;
		pipelineCreateInfo.pStages = shaderStages;
		pipelineCreateInfo.pVertexInputState = &vertexInputInfo;
		pipelineCreateInfo.pInputAssemblyState = &inputAssembly;
		pipelineCreateInfo.pViewportState = &viewportState;
		pipelineCreateInfo.pRasterizationState = &rasterizer;
		pipelineCreateInfo.pMultisampleState = &multisampling;
		pipelineCreateInfo.pColorBlendState = &colourBlending;
		pipelineCreateInfo.layout = mGraphicsPipelineLayout;
		pipelineCreateInfo.renderPass = mRenderPass->get();
		pipelineCreateInfo.subpass = 0;

		TRY(
			vkCreateGraphicsPipelines(logicDevice, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &mGraphicsPipeline) != VK_SUCCESS,
			"Failed to create Graphics Pipeline."
		);

		mShaderProgram.closeModules(logicDevice);

		mSwapChain->createFramebuffers(logicDevice, mRenderPass->get());
	}

	void GraphicsPipelineVulkan::bind(VkCommandBuffer cmdBuffer) {
		vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mGraphicsPipeline);
	}

	void GraphicsPipelineVulkan::beginRenderPass(size_t framebufferIndex, VkCommandBuffer cmdBuffer) {
		mRenderPass->begin(mSwapChain->getFramebufferAt(framebufferIndex), mSwapChain->getExtent(), cmdBuffer);
	}

	void GraphicsPipelineVulkan::endRenderPass(VkCommandBuffer cmdBuffer) {
		vkCmdEndRenderPass(cmdBuffer);
	}

	void GraphicsPipelineVulkan::close(VkDevice logicDevice) {
		closeSyncObjects(logicDevice);

		vkFreeCommandBuffers(
			logicDevice,
			mCommandPool,
			static_cast<uint32_t>(mCommandBuffers.size()),
			mCommandBuffers.data()
		);

		vkDestroyPipeline(logicDevice, mGraphicsPipeline, nullptr);
		vkDestroyPipelineLayout(logicDevice, mGraphicsPipelineLayout, nullptr);
		mRenderPass->close(logicDevice);
		mSwapChain->close(logicDevice);
	}
}
