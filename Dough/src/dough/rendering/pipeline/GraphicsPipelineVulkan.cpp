#include "dough/rendering/pipeline/GraphicsPipelineVulkan.h"

#include "dough/rendering/buffer/BufferVulkan.h"
#include "dough/rendering/shader/ShaderVulkan.h"

namespace DOH {

	GraphicsPipelineVulkan::GraphicsPipelineVulkan()
	:	mGraphicsPipeline(VK_NULL_HANDLE),
		mGraphicsPipelineLayout(VK_NULL_HANDLE),
		mCommandPool(VK_NULL_HANDLE),
		mDescriptorPool(VK_NULL_HANDLE),
		mSwapChain(SwapChainVulkan::createNonInit()),
		mRenderPass(RenderPassVulkan::createNonInit()),
		mShaderDescriptor(0),
		mVertShaderPath(ShaderVulkan::NO_PATH),
		mFragShaderPath(ShaderVulkan::NO_PATH)
	{
	}

	GraphicsPipelineVulkan::GraphicsPipelineVulkan(
		SwapChainVulkan& swapChain,
		RenderPassVulkan renderPass,
		std::string& vertShaderPath,
		std::string& fragShaderPath
	) : mGraphicsPipeline(VK_NULL_HANDLE),
		mGraphicsPipelineLayout(VK_NULL_HANDLE),
		mSwapChain(swapChain),
		mRenderPass(renderPass),
		mShaderDescriptor(0),
		mCommandPool(VK_NULL_HANDLE),
		mDescriptorPool(VK_NULL_HANDLE),
		mVertShaderPath(vertShaderPath),
		mFragShaderPath(fragShaderPath)
	{
	}

	void GraphicsPipelineVulkan::createUniformBufferObject(VkDevice logicDevice, size_t bufferSize) {
		//mUniformDescriptor.setBufferSize(bufferSize);
		//VkDescriptorSetLayoutBinding layoutBinding = DescriptorVulkan::createLayoutBinding(
		//	VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		//	VK_SHADER_STAGE_VERTEX_BIT,
		//	1,
		//	0
		//);
		//mUniformDescriptor.createDescriptorSetLayout(
		//	logicDevice,
		//	layoutBinding,
		//	1
		//);
		mShaderDescriptor.setBufferSize(bufferSize);
		std::vector<VkDescriptorSetLayoutBinding> layoutBindings = {
			DescriptorVulkan::createLayoutBinding(
				VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
				VK_SHADER_STAGE_VERTEX_BIT,
				1,
				0
			),
			DescriptorVulkan::createLayoutBinding(
				VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
				VK_SHADER_STAGE_FRAGMENT_BIT,
				1,
				1
			)
		};
		mShaderDescriptor.createDescriptorSetLayout(
			logicDevice,
			layoutBindings,
			static_cast<uint32_t>(layoutBindings.size())
		);
	}

	void GraphicsPipelineVulkan::uploadShaderUBO(VkDevice logicDevice, VkPhysicalDevice physicalDevice, TextureVulkan& texture) {
		const size_t count = mSwapChain.getImageCount();
		//mUniformDescriptor.createBuffers(logicDevice, physicalDevice, count);
		//mUniformDescriptor.createDescriptorSets(logicDevice, count, mDescriptorPool);
		//mUniformDescriptor.updateDescriptorSets(logicDevice, count);
		mShaderDescriptor.createBuffers(logicDevice, physicalDevice, count);
		mShaderDescriptor.createDescriptorSets(logicDevice, count, mDescriptorPool);
		mShaderDescriptor.setTexture(1, texture.getImageView(), texture.getSampler());
		mShaderDescriptor.updateDescriptorSets(logicDevice, count);
	}

	void GraphicsPipelineVulkan::createCommandBuffers(VkDevice logicDevice /*TEMP::*/, VkBuffer vertexBuffer, VkBuffer indexBuffer, uint32_t indexCount/*::TEMP*/) {
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

			//mUniformDescriptor.bindDescriptorSets(mCommandBuffers[i], mGraphicsPipelineLayout, i);
			mShaderDescriptor.bindDescriptorSets(mCommandBuffers[i], mGraphicsPipelineLayout, i);

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
		//pipelineLayoutCreateInfo.pSetLayouts = &mUniformDescriptor.getDescriptorSetLayout();
		pipelineLayoutCreateInfo.pSetLayouts = &mShaderDescriptor.getDescriptorSetLayout();

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
		mShaderDescriptor.close(logicDevice); //adding in vkDescriptDescriptorSetLayout here, may need to add a "closeBuffers" func to separate from the time when closing descSetLayout

		vkFreeCommandBuffers(logicDevice, mCommandPool, static_cast<uint32_t>(mCommandBuffers.size()), mCommandBuffers.data());

		vkDestroyPipeline(logicDevice, mGraphicsPipeline, nullptr);
		vkDestroyPipelineLayout(logicDevice, mGraphicsPipelineLayout, nullptr);
		mRenderPass.close(logicDevice);
		mSwapChain.close(logicDevice);
	}

	GraphicsPipelineVulkan GraphicsPipelineVulkan::create(
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
	) {
		SwapChainVulkan swapChain = SwapChainVulkan::create(logicDevice, scsd, surface, indices, width, height);
		RenderPassVulkan renderPass = RenderPassVulkan::create(logicDevice, swapChain.getImageFormat());
		GraphicsPipelineVulkan pipeline = GraphicsPipelineVulkan(
			swapChain,
			renderPass,
			vertShaderPath,
			fragShaderPath
		);
		pipeline.setCommandPool(cmdPool);
		pipeline.createUniformBufferObject(logicDevice, uniformBufferSize);
		pipeline.init(logicDevice);

		return pipeline;
	}

	GraphicsPipelineVulkan GraphicsPipelineVulkan::createNonInit() {
		return GraphicsPipelineVulkan();
	}
}
