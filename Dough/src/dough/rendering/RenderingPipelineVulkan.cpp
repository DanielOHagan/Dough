#include "dough/rendering/RenderingPipelineVulkan.h"

#include "dough/rendering/shader/ShaderVulkan.h"
#include "dough/rendering/RenderingContextVulkan.h"

namespace DOH {

	RenderingPipelineVulkan::RenderingPipelineVulkan()
	:	mLogicDevice(VK_NULL_HANDLE),
		mPhysicalDevice(VK_NULL_HANDLE),
		mRenderPass(VK_NULL_HANDLE),
		mPipelineLayout(VK_NULL_HANDLE),
		mGraphicsPipeline(VK_NULL_HANDLE),
		mGraphicsQueue(VK_NULL_HANDLE),
		mPresentQueue(VK_NULL_HANDLE),
		mSwapChain(SwapChainVulkan::createNull()),
		mCommandPool(VK_NULL_HANDLE),
		mCurrentFrame(0)
	{
	}

	void RenderingPipelineVulkan::init(
		VkDevice logicDevice,
		VkPhysicalDevice physicalDevice,
		SwapChainSupportDetails& scSupport,
		VkSurfaceKHR surface,
		QueueFamilyIndices& queueFamilyIndices,
		uint32_t width,
		uint32_t height
	) {
		mLogicDevice = logicDevice;
		mPhysicalDevice = physicalDevice;

		createQueues(queueFamilyIndices);

		mSwapChain.init(mLogicDevice, scSupport, surface, queueFamilyIndices, width, height);

		createRenderPass();
		createGraphicsPipeline();

		mSwapChain.createFramebuffers(mLogicDevice, mRenderPass);

		createCommandPool(queueFamilyIndices);

		mVertexBuffer = BufferVulkan::createStagedBuffer(
			mLogicDevice,
			mPhysicalDevice,
			mCommandPool,
			mGraphicsQueue,
			vertices.data(),
			sizeof(vertices[0]) * vertices.size(),
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		);
		mIndexBuffer = IndexBufferVulkan::createStagedIndexBuffer(
			mLogicDevice,
			mPhysicalDevice,
			mCommandPool,
			mGraphicsQueue,
			indices.data(),
			sizeof(indices[0]) * indices.size(),
			static_cast<uint32_t>(indices.size())
		);
		
		createCommandBuffers();

		createSyncObjects();
	}

	void RenderingPipelineVulkan::close() {
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			vkDestroySemaphore(mLogicDevice, mImageAvailableSemaphores[i], nullptr);
			vkDestroySemaphore(mLogicDevice, mRenderFinishedSemaphores[i], nullptr);
			vkDestroyFence(mLogicDevice, mInFlightFences[i], nullptr);
		}

		vkDestroyCommandPool(mLogicDevice, mCommandPool, nullptr);

		mSwapChain.destroyFramebuffers(mLogicDevice);

		mVertexBuffer.close(mLogicDevice);
		mIndexBuffer.close(mLogicDevice);

		vkDestroyPipeline(mLogicDevice, mGraphicsPipeline, nullptr);
		vkDestroyPipelineLayout(mLogicDevice, mPipelineLayout, nullptr);
		vkDestroyRenderPass(mLogicDevice, mRenderPass, nullptr);

		mSwapChain.close(mLogicDevice);
	}

	void RenderingPipelineVulkan::resizeSwapChain(
		SwapChainSupportDetails& scSupport,
		VkSurfaceKHR surface,
		QueueFamilyIndices& queueFamilyIndices,
		uint32_t width,
		uint32_t height
	) {
		if (mSwapChain.isResizable()) {
			vkDeviceWaitIdle(mLogicDevice);

			closeOldSwapChain();

			mSwapChain.init(mLogicDevice, scSupport, surface, queueFamilyIndices, width, height);
			createRenderPass();
			createGraphicsPipeline();
			mSwapChain.createFramebuffers(mLogicDevice, mRenderPass);
			createCommandBuffers();
		}
	}

	void RenderingPipelineVulkan::drawFrame() {
		//Wait for the fences on the GPU to flag as finished
		vkWaitForFences(mLogicDevice, 1, &mInFlightFences[mCurrentFrame], VK_TRUE, UINT64_MAX);

		uint32_t imageIndex;
		vkAcquireNextImageKHR(mLogicDevice, mSwapChain.getSwapChain(), UINT64_MAX, mImageAvailableSemaphores[mCurrentFrame], VK_NULL_HANDLE, &imageIndex);

		//Check if a previous frame is using this image (i.e. there is its fence to wait on)
		if (mImageFencesInFlight[imageIndex] != VK_NULL_HANDLE) {
			vkWaitForFences(mLogicDevice, 1, &mImageFencesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
		}

		//Mark the image as now ben in use by this frame
		mImageFencesInFlight[imageIndex] = mInFlightFences[mCurrentFrame];

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkSemaphore waitSemaphores[] = {mImageAvailableSemaphores[mCurrentFrame]};
		VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &mCommandBuffers[imageIndex];

		VkSemaphore signalSemaphores[] = {mRenderFinishedSemaphores[mCurrentFrame]};
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		vkResetFences(mLogicDevice, 1, &mInFlightFences[mCurrentFrame]);

		TRY(
			vkQueueSubmit(mGraphicsQueue, 1, &submitInfo, mInFlightFences[mCurrentFrame]) != VK_SUCCESS,
			"Failed to submit Draw Command Buffer."
		);

		VkPresentInfoKHR present = {};
		present.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		present.waitSemaphoreCount = 1;
		present.pWaitSemaphores = signalSemaphores;

		VkSwapchainKHR swapChains[] = {mSwapChain.getSwapChain()};
		present.swapchainCount = 1;
		present.pSwapchains = swapChains;
		present.pImageIndices = &imageIndex;
		present.pResults = nullptr; //Optional

		vkQueuePresentKHR(mPresentQueue, &present);

		mCurrentFrame = (mCurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
	}

	void RenderingPipelineVulkan::createQueues(QueueFamilyIndices& queueFamilyIndices) {
		vkGetDeviceQueue(mLogicDevice, queueFamilyIndices.graphicsFamily.value(), 0, &mGraphicsQueue);
		vkGetDeviceQueue(mLogicDevice, queueFamilyIndices.presentFamily.value(), 0, &mPresentQueue);
	}

	void RenderingPipelineVulkan::createRenderPass() {
		VkAttachmentDescription colourAttachment = {};
		colourAttachment.format = mSwapChain.getImageFormat();
		colourAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colourAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colourAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colourAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colourAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colourAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colourAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference colourAttachmentRef = {};
		colourAttachmentRef.attachment = 0;
		colourAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colourAttachmentRef;

		VkSubpassDependency dependency = {};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		VkRenderPassCreateInfo renderPass = {};
		renderPass.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPass.attachmentCount = 1;
		renderPass.pAttachments = &colourAttachment;
		renderPass.subpassCount = 1;
		renderPass.pSubpasses = &subpass;
		renderPass.dependencyCount = 1;
		renderPass.pDependencies = &dependency;

		TRY(
			vkCreateRenderPass(mLogicDevice, &renderPass, nullptr, &mRenderPass) != VK_SUCCESS,
			"Failed to create Render Pass."
		);
	}

	void RenderingPipelineVulkan::createGraphicsPipeline() {
		std::string vertShaderPath = "res/shaders/vert.spv";
		std::string fragShaderPath = "res/shaders/frag.spv";
		ShaderVulkan vertShader = ShaderVulkan::create(mLogicDevice, EShaderType::VERTEX, vertShaderPath);
		ShaderVulkan fragShader = ShaderVulkan::create(mLogicDevice, EShaderType::FRAGMENT, fragShaderPath);

		VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = vertShader.getShaderModule();
		vertShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = fragShader.getShaderModule();
		fragShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

		VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

		auto bindDesc = Vertex::getBindingDescription();
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
		viewport.y = 0.0f;
		viewport.width = (float) mSwapChain.getExtent().width;
		viewport.height = (float) mSwapChain.getExtent().height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissorRectangle = {};
		scissorRectangle.offset = {0, 0};
		scissorRectangle.extent = mSwapChain.getExtent();

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
		rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;
		rasterizer.depthBiasConstantFactor = 0.0f; //Optional
		rasterizer.depthBiasClamp = 0.0f; //Optional
		rasterizer.depthBiasSlopeFactor = 0.0f; //Optional

		VkPipelineMultisampleStateCreateInfo multisampling = {};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisampling.minSampleShading = 1.0f; //Optional
		multisampling.pSampleMask = nullptr; //Optional
		multisampling.alphaToCoverageEnable = VK_FALSE; //Optional
		multisampling.alphaToOneEnable = VK_FALSE; //Optional

		VkPipelineColorBlendAttachmentState colourBlendAttachment = {};
		colourBlendAttachment.colorWriteMask =
			VK_COLOR_COMPONENT_R_BIT |
			VK_COLOR_COMPONENT_G_BIT |
			VK_COLOR_COMPONENT_B_BIT |
			VK_COLOR_COMPONENT_A_BIT;
		colourBlendAttachment.blendEnable = VK_FALSE;
		colourBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; //Optional
		colourBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; //Optional
		colourBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; //Optional
		colourBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; //Optional
		colourBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; //Optional
		colourBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; //Optional

		VkPipelineColorBlendStateCreateInfo colourBlending = {};
		colourBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colourBlending.logicOpEnable = VK_FALSE;
		colourBlending.logicOp = VK_LOGIC_OP_COPY; //Optional
		colourBlending.attachmentCount = 1;
		colourBlending.pAttachments = &colourBlendAttachment;
		colourBlending.blendConstants[0] = 0.0f; //Optional
		colourBlending.blendConstants[1] = 0.0f; //Optional
		colourBlending.blendConstants[2] = 0.0f; //Optional
		colourBlending.blendConstants[3] = 0.0f; //Optional

		//NOTE:: Read about Dynamic States

		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
		pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutCreateInfo.setLayoutCount = 0; //Optional
		pipelineLayoutCreateInfo.pSetLayouts = nullptr; //Optional
		pipelineLayoutCreateInfo.pushConstantRangeCount = 0; //Optional
		pipelineLayoutCreateInfo.pPushConstantRanges = nullptr; //Optional

		TRY(
			vkCreatePipelineLayout(mLogicDevice, &pipelineLayoutCreateInfo, nullptr, &mPipelineLayout) != VK_SUCCESS,
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
		pipelineCreateInfo.pDepthStencilState = nullptr; //Optional
		pipelineCreateInfo.pColorBlendState = &colourBlending;
		pipelineCreateInfo.pDynamicState = nullptr; //Optional
		pipelineCreateInfo.layout = mPipelineLayout;
		pipelineCreateInfo.renderPass = mRenderPass;
		pipelineCreateInfo.subpass = 0;
		pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE; //Optional
		pipelineCreateInfo.basePipelineIndex = -1; //Optional

		TRY(
			vkCreateGraphicsPipelines(mLogicDevice, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &mGraphicsPipeline) != VK_SUCCESS,
			"Failed to create Graphics Pipeline."
		);

		vertShader.close(mLogicDevice);
		fragShader.close(mLogicDevice);
	}

	void RenderingPipelineVulkan::createCommandPool(QueueFamilyIndices& queueFamilyIndices) {
		VkCommandPoolCreateInfo cmdPoolCreateInfo = {};
		cmdPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		cmdPoolCreateInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
		cmdPoolCreateInfo.flags = 0; //Optional

		TRY(
			vkCreateCommandPool(mLogicDevice, &cmdPoolCreateInfo, nullptr, &mCommandPool) != VK_SUCCESS,
			"Failed to create Command Pool."
		);
	}

	void RenderingPipelineVulkan::createCommandBuffers() {
		mCommandBuffers.resize(mSwapChain.getFramebufferCount());

		VkCommandBufferAllocateInfo cmdBuffAlloc = {};
		cmdBuffAlloc.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		cmdBuffAlloc.commandPool = mCommandPool;
		cmdBuffAlloc.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		cmdBuffAlloc.commandBufferCount = (uint32_t) mCommandBuffers.size();

		TRY(
			vkAllocateCommandBuffers(mLogicDevice, &cmdBuffAlloc, mCommandBuffers.data()) != VK_SUCCESS,
			"Failed to allocate Command Buffers."
		);

		for (size_t i = 0; i < mCommandBuffers.size(); i++) {
			VkCommandBufferBeginInfo beginInfo = {};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT; //Optional
			beginInfo.pInheritanceInfo = nullptr; //Optional

			TRY(
				vkBeginCommandBuffer(mCommandBuffers[i], &beginInfo) != VK_SUCCESS,
				"Failed to begin recording Command Buffer."
			);

			VkRenderPassBeginInfo renderPass = {};
			renderPass.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPass.renderPass = mRenderPass;
			renderPass.framebuffer = mSwapChain.getFramebufferAt(i);
			renderPass.renderArea.offset = {0, 0};
			renderPass.renderArea.extent = mSwapChain.getExtent();

			VkClearValue clearColour = {0.0f, 0.0f, 0.0f, 1.0f};
			renderPass.clearValueCount = 1;
			renderPass.pClearValues = &clearColour;

			vkCmdBeginRenderPass(mCommandBuffers[i], &renderPass, VK_SUBPASS_CONTENTS_INLINE);

			vkCmdBindPipeline(mCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, mGraphicsPipeline);

			VkBuffer vertexBuffers[] = {mVertexBuffer.getBuffer()};
			VkDeviceSize offsets[] = {0};
			vkCmdBindVertexBuffers(mCommandBuffers[i], 0, 1, vertexBuffers, offsets);

			vkCmdBindIndexBuffer(mCommandBuffers[i], mIndexBuffer.getBuffer(), 0, VK_INDEX_TYPE_UINT16);

			vkCmdDrawIndexed(
				mCommandBuffers[i],
				mIndexBuffer.getCount(),
				1,
				0,
				0,
				0
			);

			vkCmdEndRenderPass(mCommandBuffers[i]);

			TRY(
				vkEndCommandBuffer(mCommandBuffers[i]) != VK_SUCCESS,
				"Failed to record Command Buffer."
			);
		}
	}

	void RenderingPipelineVulkan::createSyncObjects() {
		mImageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		mRenderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		mInFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
		mImageFencesInFlight.resize(mSwapChain.getImageCount(), VK_NULL_HANDLE);

		VkSemaphoreCreateInfo semaphore = {};
		semaphore.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fence = {};
		fence.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fence.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			TRY(
				vkCreateSemaphore(mLogicDevice, &semaphore, nullptr, &mImageAvailableSemaphores[i]) != VK_SUCCESS ||
				vkCreateSemaphore(mLogicDevice, &semaphore, nullptr, &mRenderFinishedSemaphores[i]) != VK_SUCCESS,
				"Failed to create Semaphores for a frame."
			);

			TRY(
				vkCreateFence(mLogicDevice, &fence, nullptr, &mInFlightFences[i]) != VK_SUCCESS,
				"Failed to create Fence."
			);
		}
	}

	void RenderingPipelineVulkan::closeOldSwapChain() {
		mSwapChain.destroyFramebuffers(mLogicDevice);

		vkFreeCommandBuffers(mLogicDevice, mCommandPool, static_cast<uint32_t>(mCommandBuffers.size()), mCommandBuffers.data());

		vkDestroyPipeline(mLogicDevice, mGraphicsPipeline, nullptr);
		vkDestroyPipelineLayout(mLogicDevice, mPipelineLayout, nullptr);
		vkDestroyRenderPass(mLogicDevice, mRenderPass, nullptr);

		mSwapChain.close(mLogicDevice);
	}
}