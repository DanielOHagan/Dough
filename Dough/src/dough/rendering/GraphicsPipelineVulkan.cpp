#include "dough/rendering/GraphicsPipelineVulkan.h"

#include "dough/rendering/buffer/BufferVulkan.h"
#include "dough/rendering/shader/ShaderVulkan.h"

namespace DOH {

	GraphicsPipelineVulkan::GraphicsPipelineVulkan()
	:	mGraphicsPipeline(VK_NULL_HANDLE),
		mGraphicsPipelineLayout(VK_NULL_HANDLE),
		mSwapChain(SwapChainVulkan::createNonInit()),
		mRenderPass(RenderPassVulkan::createNonInit()),
		mDescriptorSetLayout(VK_NULL_HANDLE),
		mUniformBufferSize(0),
		mCommandPool(VK_NULL_HANDLE),
		mDescriptorPool(VK_NULL_HANDLE),
		mVertShaderPath(ShaderVulkan::NO_PATH),
		mFragShaderPath(ShaderVulkan::NO_PATH)
	{
	}

	GraphicsPipelineVulkan::GraphicsPipelineVulkan(
		SwapChainVulkan swapChain,
		RenderPassVulkan renderPass,
		std::string& vertShaderPath,
		std::string& fragShaderPath
	) : mGraphicsPipeline(VK_NULL_HANDLE),
		mGraphicsPipelineLayout(VK_NULL_HANDLE),
		mSwapChain(swapChain),
		mRenderPass(renderPass),
		mDescriptorSetLayout(VK_NULL_HANDLE),
		mUniformBufferSize(0),
		mCommandPool(VK_NULL_HANDLE),
		mDescriptorPool(VK_NULL_HANDLE),
		mVertShaderPath(vertShaderPath),
		mFragShaderPath(fragShaderPath)
	{
	}

	void GraphicsPipelineVulkan::createUniformBuffers(VkDevice logicDevice, VkPhysicalDevice physicalDevice) {
		size_t imageCount = mSwapChain.getImageCount();

		mUniformBuffers.resize(imageCount);

		for (size_t i = 0; i < imageCount; i++) {
			mUniformBuffers[i] = BufferVulkan::createBuffer(
				logicDevice,
				physicalDevice,
				mUniformBufferSize,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
			);
		}
	}

	void GraphicsPipelineVulkan::createDescriptorSetLayout(VkDevice logicDevice) {
		VkDescriptorSetLayoutBinding layoutBinding = {};
		layoutBinding.binding = 0;
		layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		layoutBinding.descriptorCount = 1;
		layoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	
		VkDescriptorSetLayoutCreateInfo dslCreateInfo = {};
		dslCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		dslCreateInfo.bindingCount = 1;
		dslCreateInfo.pBindings = &layoutBinding;
	
		TRY(
			vkCreateDescriptorSetLayout(logicDevice, &dslCreateInfo, nullptr, &mDescriptorSetLayout) != VK_SUCCESS,
			"Failed to create descriptor set layout."
		);
	}

	void GraphicsPipelineVulkan::createDescriptorSets(VkDevice logicDevice) {
		size_t imageCount = mSwapChain.getImageCount();
		std::vector<VkDescriptorSetLayout> layouts(imageCount, mDescriptorSetLayout);

		VkDescriptorSetAllocateInfo allocation = {};
		allocation.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocation.descriptorPool = mDescriptorPool;
		allocation.descriptorSetCount = static_cast<uint32_t>(imageCount);
		allocation.pSetLayouts = layouts.data();

		mDescriptorSets.resize(imageCount);

		TRY(
			vkAllocateDescriptorSets(logicDevice, &allocation, mDescriptorSets.data()) != VK_SUCCESS,
			"Failed to allocate descriptor sets."
		);

		for (size_t i = 0; i < imageCount; i++) {
			VkDescriptorBufferInfo bufferInfo = {};
			bufferInfo.buffer = mUniformBuffers[i].getBuffer();
			bufferInfo.offset = 0;
			bufferInfo.range = mUniformBufferSize;

			VkWriteDescriptorSet descWrite = {};
			descWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descWrite.dstSet = mDescriptorSets[i];
			descWrite.dstBinding = 0;
			descWrite.dstArrayElement = 0;
			descWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descWrite.descriptorCount = 1;
			descWrite.pBufferInfo = &bufferInfo;

			vkUpdateDescriptorSets(logicDevice, 1, &descWrite, 0, nullptr);
		}
	}

	void GraphicsPipelineVulkan::createCommandBuffers(VkDevice logicDevice /*TEMP:*/, VkBuffer vertexBuffer, VkBuffer indexBuffer, uint32_t indexCount) {
		mCommandBuffers.resize(mSwapChain.getFramebufferCount());

		VkCommandBufferAllocateInfo cmdBuffAlloc = {};
		cmdBuffAlloc.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		cmdBuffAlloc.commandPool = mCommandPool;
		cmdBuffAlloc.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		cmdBuffAlloc.commandBufferCount = (uint32_t) mCommandBuffers.size();

		TRY(
			vkAllocateCommandBuffers(logicDevice, &cmdBuffAlloc, mCommandBuffers.data()) != VK_SUCCESS,
			"Failed to allocate Command Buffers."
		);

		for (size_t i = 0; i < mCommandBuffers.size(); i++) {
			VkCommandBufferBeginInfo beginInfo = {};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

			TRY(
				vkBeginCommandBuffer(mCommandBuffers[i], &beginInfo) != VK_SUCCESS,
				"Failed to begin recording Command Buffer."
			);

			beginRenderPass(i, mCommandBuffers[i]);

			bind(mCommandBuffers[i]);

			//vao.bind(mCommandBuffers[i], mGraphicsPipelineLayout, &mDescriptorSets[i]);
			//To define rest of this scope's logic
			VkBuffer vertexBuffers[] = {vertexBuffer};
			VkDeviceSize offsets[] = {0};
			vkCmdBindVertexBuffers(mCommandBuffers[i], 0, 1, vertexBuffers, offsets);
			vkCmdBindIndexBuffer(mCommandBuffers[i], indexBuffer, 0, VK_INDEX_TYPE_UINT16);

			vkCmdBindDescriptorSets(
				mCommandBuffers[i],
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				mGraphicsPipelineLayout,
				0,
				1,
				&mDescriptorSets[i],
				0,
				nullptr
			);

			vkCmdDrawIndexed(
				mCommandBuffers[i],
				indexCount,
				1,
				0,
				0,
				0
			);

			endRenderPass(mCommandBuffers[i]);

			TRY(
				vkEndCommandBuffer(mCommandBuffers[i]) != VK_SUCCESS,
				"Failed to record Command Buffer."
			);
		}
	}

	void GraphicsPipelineVulkan::init(VkDevice logicDevice) {
		ShaderVulkan vertShader = ShaderVulkan::create(logicDevice, EShaderType::VERTEX, mVertShaderPath);
		ShaderVulkan fragShader = ShaderVulkan::create(logicDevice, EShaderType::FRAGMENT, mFragShaderPath);

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

		auto bindDesc = getBindingDescription();
		auto attribDescs = getAttributeDescriptions();

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
		pipelineLayoutCreateInfo.pSetLayouts = &mDescriptorSetLayout;

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
		pipelineCreateInfo.renderPass = mRenderPass.get();
		pipelineCreateInfo.subpass = 0;

		TRY(
			vkCreateGraphicsPipelines(logicDevice, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &mGraphicsPipeline) != VK_SUCCESS,
			"Failed to create Graphics Pipeline."
		);

		vertShader.close(logicDevice);
		fragShader.close(logicDevice);

		mSwapChain.createFramebuffers(logicDevice, mRenderPass.get());
	}

	void GraphicsPipelineVulkan::bind(VkCommandBuffer cmdBuffer) {
		vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mGraphicsPipeline);
	}

	void GraphicsPipelineVulkan::beginRenderPass(size_t framebufferIndex, VkCommandBuffer cmdBuffer) {
		mRenderPass.begin(mSwapChain.getFramebufferAt(framebufferIndex), mSwapChain.getExtent(), cmdBuffer);
	}

	void GraphicsPipelineVulkan::endRenderPass(VkCommandBuffer cmdBuffer) {
		vkCmdEndRenderPass(cmdBuffer);
	}

	void GraphicsPipelineVulkan::close(VkDevice logicDevice) {
		for (BufferVulkan uniformBuffer : mUniformBuffers) {
			uniformBuffer.close(logicDevice);
		}

		vkFreeCommandBuffers(logicDevice, mCommandPool, static_cast<uint32_t>(mCommandBuffers.size()), mCommandBuffers.data());

		vkDestroyPipeline(logicDevice, mGraphicsPipeline, nullptr);
		vkDestroyPipelineLayout(logicDevice, mGraphicsPipelineLayout, nullptr);
		mRenderPass.close(logicDevice);
		mSwapChain.close(logicDevice);

		vkDestroyDescriptorSetLayout(logicDevice, mDescriptorSetLayout, nullptr);
	}

	void GraphicsPipelineVulkan::create(
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
	) {
		dst->mSwapChain = SwapChainVulkan::create(logicDevice, scsd, surface, indices, width, height);
		dst->mRenderPass = RenderPassVulkan::create(logicDevice, dst->mSwapChain.getImageFormat());
		dst->mVertShaderPath.assign(vertShaderPath);
		dst->mFragShaderPath.assign(fragShaderPath);
		dst->mCommandPool = cmdPool;
		dst->mUniformBufferSize = uniformBufferSize;
		dst->createDescriptorSetLayout(logicDevice);
		dst->init(logicDevice);
	}

	GraphicsPipelineVulkan GraphicsPipelineVulkan::createNonInit() {
		return GraphicsPipelineVulkan();
	}
}