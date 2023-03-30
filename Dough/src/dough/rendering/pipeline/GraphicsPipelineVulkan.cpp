#include "dough/rendering/pipeline/GraphicsPipelineVulkan.h"

#include "dough/rendering/buffer/BufferVulkan.h"
#include "dough/rendering/pipeline/shader/ShaderVulkan.h"
#include "dough/rendering/ObjInit.h"
#include "dough/Logging.h"
#include "dough/rendering/RenderingContextVulkan.h"

namespace DOH {

	GraphicsPipelineVulkan::GraphicsPipelineVulkan(
		VkDevice logicDevice,
		GraphicsPipelineInstanceInfo& instanceInfo,
		VkRenderPass renderPass,
		VkExtent2D extent
	) : mGraphicsPipeline(VK_NULL_HANDLE),
		mGraphicsPipelineLayout(VK_NULL_HANDLE),
		mInstanceInfo(instanceInfo)
	{
		createUniformObjects(logicDevice);
		createPipelineLayout(logicDevice);
		createPipeline(logicDevice, extent, renderPass);
	}

	GraphicsPipelineVulkan::~GraphicsPipelineVulkan() {
		if (isUsingGpuResource()) {
			LOG_ERR(
				"GraphicsPipeline GPU resource NOT released before destructor was called." <<
				" Handle: " << mGraphicsPipeline << " Layout:" << mGraphicsPipelineLayout
			);

			//TODO:: Verbose error message to include mInstanceInfo

		}
	}

	void GraphicsPipelineVulkan::createPipelineLayout(VkDevice logicDevice) {
		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
		pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutCreateInfo.setLayoutCount = 1;
		pipelineLayoutCreateInfo.pSetLayouts = &mInstanceInfo.getShaderProgram().getShaderDescriptorLayout().getDescriptorSetLayout();

		if (mInstanceInfo.getShaderProgram().getUniformLayout().hasPushConstant()) {
			const auto& pushConstantRanges = mInstanceInfo.getShaderProgram().getUniformLayout().getPushConstantRanges();
			pipelineLayoutCreateInfo.pushConstantRangeCount =
				static_cast<uint32_t>(pushConstantRanges.size());
			pipelineLayoutCreateInfo.pPushConstantRanges = pushConstantRanges.data();
		} else {
			pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
			pipelineLayoutCreateInfo.pPushConstantRanges = nullptr;
		}

		VK_TRY(
			vkCreatePipelineLayout(
				logicDevice,
				&pipelineLayoutCreateInfo,
				nullptr,
				&mGraphicsPipelineLayout
			),
			"Failed to create Pipeline Layout."
		);

		mUsingGpuResource = true;
	}

	void GraphicsPipelineVulkan::createPipeline(
		VkDevice logicDevice,
		VkExtent2D extent,
		VkRenderPass renderPass
	) {
		mInstanceInfo.getShaderProgram().loadModules(logicDevice);
		TRY(!mInstanceInfo.getShaderProgram().areShadersLoaded(), "Shader Modules not loaded");

		VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = mInstanceInfo.getShaderProgram().getVertexShader().getShaderModule();
		vertShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = mInstanceInfo.getShaderProgram().getFragmentShader().getShaderModule();
		fragShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

		VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

		//IMPORTANT:: vertex input binding is always slot 0
		const uint32_t binding = 0;
		const auto bindingDesc = getVertexTypeBindingDesc(mInstanceInfo.getVertexType(), binding, VK_VERTEX_INPUT_RATE_VERTEX);
		const auto vertexAttribs = getVertexTypeAsAttribDesc(mInstanceInfo.getVertexType(), binding);

		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.pVertexBindingDescriptions = &bindingDesc;
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexAttribs.size());
		vertexInputInfo.pVertexAttributeDescriptions = vertexAttribs.data();

		VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		VkViewport viewport = {};
		viewport.x = 0.0f;
		viewport.y = 0;
		viewport.width = static_cast<float>(extent.width);
		viewport.height = static_cast<float>(extent.height);
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
		rasterizer.polygonMode = mInstanceInfo.getOptionalFields().PolygonMode;
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = mInstanceInfo.getOptionalFields().CullMode;
		rasterizer.frontFace = mInstanceInfo.getOptionalFields().FrontFace;
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
		colourBlendAttachment.blendEnable = mInstanceInfo.getOptionalFields().BlendingEnabled ? VK_TRUE : VK_FALSE;
		colourBlendAttachment.srcColorBlendFactor = mInstanceInfo.getOptionalFields().ColourBlendSrcFactor;
		colourBlendAttachment.dstColorBlendFactor = mInstanceInfo.getOptionalFields().ColourBlendDstFactor;
		colourBlendAttachment.colorBlendOp = mInstanceInfo.getOptionalFields().ColourBlendOp;
		colourBlendAttachment.srcAlphaBlendFactor = mInstanceInfo.getOptionalFields().AlphaBlendSrcFactor;
		colourBlendAttachment.dstAlphaBlendFactor = mInstanceInfo.getOptionalFields().AlphaBlendDstFactor;
		colourBlendAttachment.alphaBlendOp = mInstanceInfo.getOptionalFields().AlphaBlendOp;

		VkPipelineColorBlendStateCreateInfo colourBlending = {};
		colourBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colourBlending.logicOpEnable = VK_FALSE;
		colourBlending.attachmentCount = 1;
		colourBlending.pAttachments = &colourBlendAttachment;

		VkPipelineDepthStencilStateCreateInfo depthStencil = {};
		depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencil.depthTestEnable = mInstanceInfo.getOptionalFields().DepthTestingEnabled ? VK_TRUE : VK_FALSE;
		depthStencil.depthWriteEnable = mInstanceInfo.getOptionalFields().DepthTestingEnabled ? VK_TRUE : VK_FALSE;
		depthStencil.depthCompareOp = mInstanceInfo.getOptionalFields().DepthCompareOp;
		depthStencil.depthBoundsTestEnable = VK_FALSE;

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
		pipelineCreateInfo.pDepthStencilState = &depthStencil;

		VK_TRY(
			vkCreateGraphicsPipelines(logicDevice, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &mGraphicsPipeline),
			"Failed to create Graphics Pipeline."
		);

		mUsingGpuResource = true;

		mInstanceInfo.getShaderProgram().closeModules(logicDevice);
	}

	void GraphicsPipelineVulkan::close(VkDevice logicDevice) {
		vkDestroyPipeline(logicDevice, mGraphicsPipeline, nullptr);
		vkDestroyPipelineLayout(logicDevice, mGraphicsPipelineLayout, nullptr);

		mUsingGpuResource = false;
	}

	void GraphicsPipelineVulkan::recreate(
		VkDevice logicDevice,
		VkExtent2D extent,
		VkRenderPass renderPass
	) {
		mInstanceInfo.getShaderProgram().closePipelineSpecificObjects(logicDevice);
		vkDestroyPipeline(logicDevice, mGraphicsPipeline, nullptr);

		createPipeline(logicDevice, extent, renderPass);
		mInstanceInfo.getShaderProgram().getShaderDescriptorLayout().createDescriptorSetLayout(logicDevice);
	}

	void GraphicsPipelineVulkan::createUniformObjects(VkDevice logicDevice) {
		mInstanceInfo.getShaderProgram().getShaderDescriptorLayout().createDescriptorSetLayoutBindings(
			logicDevice,
			mInstanceInfo.getShaderProgram().getUniformLayout().getTotalUniformCount()
		);
		mInstanceInfo.getShaderProgram().getShaderDescriptorLayout().createDescriptorSetLayout(logicDevice);
	}

	void GraphicsPipelineVulkan::createShaderUniforms(
		VkDevice logicDevice,
		VkPhysicalDevice physicalDevice,
		uint32_t imageCount,
		VkDescriptorPool descPool
	) {
		DescriptorSetLayoutVulkan& desc = mInstanceInfo.getShaderProgram().getShaderDescriptorLayout();
		desc.createValueBuffers(logicDevice, physicalDevice, imageCount);
		desc.createDescriptorSets(logicDevice, imageCount, descPool);
	}

	void GraphicsPipelineVulkan::updateShaderUniforms(VkDevice logicDevice, uint32_t imageCount) {
		mInstanceInfo.getShaderProgram().getShaderDescriptorLayout().updateAllDescriptorSets(logicDevice, imageCount);
	}
	
	void GraphicsPipelineVulkan::setFrameUniformData(VkDevice logicDevice, uint32_t image, uint32_t binding, void* data, size_t size) {
		mInstanceInfo.getShaderProgram().getShaderDescriptorLayout().getBuffersFromBinding(binding)[image]
			->setDataUnmapped(logicDevice, data, size);
	}

	void GraphicsPipelineVulkan::recordDrawCommands(uint32_t imageIndex, VkCommandBuffer cmd) {
		if (mInstanceInfo.getShaderProgram().getUniformLayout().hasUniforms()) {
			mInstanceInfo.getShaderProgram().getShaderDescriptorLayout().bindDescriptorSets(
				cmd,
				mGraphicsPipelineLayout,
				imageIndex
			);
		}

		for (const auto& renderable : mRenderableDrawList) {
			renderable->getVao().bind(cmd);

			for (const VkPushConstantRange& pushConstant : mInstanceInfo.getShaderProgram().getUniformLayout().getPushConstantRanges()) {
				vkCmdPushConstants(
					cmd,
					mGraphicsPipelineLayout,
					pushConstant.stageFlags,
					pushConstant.offset,
					pushConstant.size,
					renderable->getPushConstantPtr()
				);
			}

			vkCmdDrawIndexed(
				cmd,
				renderable->getVao().getDrawCount(),
				1,
				0,
				0,
				0
			);
		}

		if (mInstanceInfo.getOptionalFields().ClearRenderablesAfterDraw) {
			mRenderableDrawList.clear();
		}
	}
}
