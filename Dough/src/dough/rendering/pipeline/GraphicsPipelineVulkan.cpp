#include "dough/rendering/pipeline/GraphicsPipelineVulkan.h"

#include "dough/rendering/buffer/BufferVulkan.h"
#include "dough/rendering/pipeline/shader/ShaderVulkan.h"
#include "dough/rendering/ObjInit.h"
#include "dough/Logging.h"

namespace DOH {

	GraphicsPipelineVulkan::GraphicsPipelineVulkan(ShaderProgramVulkan& shaderProgram)
	:	mGraphicsPipeline(VK_NULL_HANDLE),
		mGraphicsPipelineLayout(VK_NULL_HANDLE),
		mShaderProgram(shaderProgram),
		mDescriptorPool(VK_NULL_HANDLE)
	{}

	GraphicsPipelineVulkan::GraphicsPipelineVulkan(
		VkDevice logicDevice,
		VkCommandPool cmdPool,
		VkExtent2D extent,
		VkRenderPass renderPass,
		ShaderProgramVulkan& shaderProgram,
		VkVertexInputBindingDescription vertexInputBindingDesc,
		std::vector<VkVertexInputAttributeDescription>& vertexAttributes
	) : mGraphicsPipeline(VK_NULL_HANDLE),
		mGraphicsPipelineLayout(VK_NULL_HANDLE),
		mShaderProgram(shaderProgram),
		mDescriptorPool(VK_NULL_HANDLE)
	{
		createUniformObjects(logicDevice);
		init(logicDevice, vertexInputBindingDesc, vertexAttributes, extent, renderPass);
	}

	void GraphicsPipelineVulkan::init(
		VkDevice logicDevice,
		VkVertexInputBindingDescription bindingDesc,
		std::vector<VkVertexInputAttributeDescription>& vertexAttributes,
		VkExtent2D extent,
		VkRenderPass renderPass
	) {
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

		VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

		VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.pVertexBindingDescriptions = &bindingDesc;
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexAttributes.size());
		vertexInputInfo.pVertexAttributeDescriptions = vertexAttributes.data();

		VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		VkViewport viewport = {};
		viewport.x = 0.0f;
		viewport.y = 0;
		viewport.width = (float)extent.width;
		viewport.height = (float)extent.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissorRectangle = {};
		scissorRectangle.offset = { 0, 0 };
		scissorRectangle.extent = extent;

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
		colourBlendAttachment.blendEnable = VK_TRUE;
		colourBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		colourBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		colourBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
		colourBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		colourBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		colourBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

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
		pipelineLayoutCreateInfo.pushConstantRangeCount = static_cast<uint32_t>(mShaderProgram.getUniformLayout().getPushConstantRanges().size());
		pipelineLayoutCreateInfo.pPushConstantRanges = mShaderProgram.getUniformLayout().hasPushConstant() ?
			mShaderProgram.getUniformLayout().getPushConstantRanges().data() : nullptr;

		VK_TRY(
			vkCreatePipelineLayout(logicDevice, &pipelineLayoutCreateInfo, nullptr, &mGraphicsPipelineLayout),
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
		pipelineCreateInfo.renderPass = renderPass;
		pipelineCreateInfo.subpass = 0;

		VK_TRY(
			vkCreateGraphicsPipelines(logicDevice, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &mGraphicsPipeline),
			"Failed to create Graphics Pipeline."
		);

		mShaderProgram.closeModules(logicDevice);
	}

	void GraphicsPipelineVulkan::close(VkDevice logicDevice) {
		vkDestroyPipeline(logicDevice, mGraphicsPipeline, nullptr);
		vkDestroyPipelineLayout(logicDevice, mGraphicsPipelineLayout, nullptr);
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

		//Create texture arrays' layout binding
		for (const auto& [binding, value] : layout.getTextureArrayUniformMap()) {
			layout.getDescriptorSetLayoutBindings()[binding] = DescriptorVulkan::createLayoutBinding(
				VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
				VK_SHADER_STAGE_FRAGMENT_BIT,
				value.Count,
				binding
			);
		}

		mShaderProgram.getShaderDescriptor().createDescriptorSetLayout(logicDevice);
	}

	void GraphicsPipelineVulkan::uploadShaderUniforms(VkDevice logicDevice, VkPhysicalDevice physicalDevice, uint32_t imageCount) {
		DescriptorVulkan& desc = mShaderProgram.getShaderDescriptor();
		desc.createValueBuffers(logicDevice, physicalDevice, imageCount);
		desc.createDescriptorSets(logicDevice, imageCount, mDescriptorPool);
		desc.updateDescriptorSets(logicDevice, imageCount);
	}

	void GraphicsPipelineVulkan::recordDrawCommands(uint32_t imageIndex, VkCommandBuffer cmd) {
		for (VertexArrayVulkan& vao : mVaoDrawList) {
			vao.bind(cmd);
			if (mShaderProgram.getUniformLayout().hasUniforms()) {
				mShaderProgram.getShaderDescriptor().bindDescriptorSets(cmd, mGraphicsPipelineLayout, imageIndex);
			}

			vkCmdDrawIndexed(
				cmd,
				vao.getIndexBuffer().getCount(),
				1,
				0,
				0,
				0
			);
		}
	}

	void GraphicsPipelineVulkan::bind(VkCommandBuffer cmdBuffer) {
		vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mGraphicsPipeline);
	}
}
