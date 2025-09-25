#include "dough/rendering/pipeline/GraphicsPipelineVulkan.h"

#include "dough/rendering/pipeline/ShaderProgram.h"
#include "dough/Utils.h"
#include "dough/rendering/VertexInputLayout.h"
#include "dough/rendering/RenderPassVulkan.h"
#include "dough/rendering/RenderingContextVulkan.h"
#include "dough/rendering/renderables/SimpleRenderable.h"

#include <tracy/public/tracy/Tracy.hpp>

namespace DOH {

	GraphicsPipelineOptionalFields& GraphicsPipelineInstanceInfo::enableOptionalFields() {
		ZoneScoped;

		if (mOptionalFields == nullptr) {
			mOptionalFields = std::make_unique<GraphicsPipelineOptionalFields>();
		}

		return *mOptionalFields;
	}

	GraphicsPipelineVulkan::GraphicsPipelineVulkan(GraphicsPipelineInstanceInfo& instanceInfo)
	:	mGraphicsPipeline(VK_NULL_HANDLE),
		mGraphicsPipelineLayout(VK_NULL_HANDLE),
		mInstanceInfo(instanceInfo)
	{}

	GraphicsPipelineVulkan::~GraphicsPipelineVulkan() {
		ZoneScoped;

		if (isUsingGpuResource()) {
			LOG_ERR(
				"GraphicsPipeline_2 GPU resource NOT released before destructor was called." <<
				" Handle: " << mGraphicsPipeline << " Layout:" << mGraphicsPipelineLayout
			);

			//TODO:: Verbose error message to include mInstanceInfo


			//NOTE:: This is to stop the IGPUResource::~IGPUReource from logging a misleading error message.
			mUsingGpuResource = false;
		}
	}

	void GraphicsPipelineVulkan::close(VkDevice logicDevice) {
		ZoneScoped;

		vkDestroyPipeline(logicDevice, mGraphicsPipeline, nullptr);
		vkDestroyPipelineLayout(logicDevice, mGraphicsPipelineLayout, nullptr);
		mUsingGpuResource = false;
	}

	void GraphicsPipelineVulkan::init(VkDevice logicDevice, VkExtent2D extent, VkRenderPass renderPass) {
		ZoneScoped;

		createPipelineLayout(logicDevice);
		createPipeline(logicDevice, extent, renderPass);
	}

	void GraphicsPipelineVulkan::resize(VkDevice logicDevice, VkExtent2D extent, VkRenderPass renderPass) {
		ZoneScoped;

		vkDestroyPipeline(logicDevice, mGraphicsPipeline, nullptr);
		createPipeline(logicDevice, extent, renderPass);
	}

	void GraphicsPipelineVulkan::recordDrawCommand(VkCommandBuffer cmd, IRenderable& renderable, CurrentBindingsState& currentBindings, uint32_t descSetOffset) {
		ZoneScoped;

		if (renderable.hasDescriptorSetsInstance()) {
			//TODO:: Multiple desc set binds at once from an array? How does that mix with PipelineLayout?
			// Should probably design around PipelineLayouts instead of Pipelines, should lower descriptor set binding and help create BindGroups.

			DescriptorSetsInstanceVulkan& shaderResourceData = *renderable.getDescriptorSetsInstance();
			const uint32_t descSetCount = static_cast<uint32_t>(shaderResourceData.getDescriptorSets().size());
			bool differentLayout = mGraphicsPipelineLayout != currentBindings.PipelineLayout; //NOTE:: Doesn't check if the layout is compatible, only if it's a different instance.
			for (uint32_t i = descSetOffset; i < descSetCount; i++) {

				//TODO:: Bind correct descSet for slots that use a descriptor for different frames (i.e. cameras)
				//	Need a way of pointing to the correct descSet if that slot is using imageIndex

				VkDescriptorSet descSet = shaderResourceData.getDescriptorSets()[i];
				if (differentLayout || currentBindings.DescriptorSets[i] != descSet) {
					vkCmdBindDescriptorSets(
						cmd,
						VK_PIPELINE_BIND_POINT_GRAPHICS,
						mGraphicsPipelineLayout,
						i,
						1,
						&descSet,
						0,
						nullptr
					);
					currentBindings.DescriptorSets[i] = descSet;
					currentBindings.PipelineLayout = mGraphicsPipelineLayout;
					//TODO:: currentBindings.LastBindIndex = i; To show when debugging that any bindings above are useless.
					//debugInfo.DescriptorSetBinds++;

					//TODO:: This should make profiling easier, save passing round a debugInfo reference
					//DescriptorApiVulkan::bind(
					//	cmd,
					//	VK_PIPELINE_BIND_POINT_GRAPHICS,
					//	mGraphicsPipelineLayout,
					//	i,
					//	1,
					//	&descSet,
					//	0,
					//	nullptr
					//);
					//Or a full RenderApiVulkan which holds all current binding info, probably better for multithreaded (which is definitely coming)
				}
			}
		}

		//TODO:: Currently binding a vao (through vao.bind()) binds both vb and ib, removing that link in vao.bind() this code should look something more like this:
		// Not using vao.bind() because that also calls ib.bind(). Currently trying to "un-link" that
		// AND this only works for single VAOs with a single vb
		const VkBuffer vb = renderable.getVao().getVertexBuffers()[0]->getBuffer();
		if (currentBindings.VertexBuffer != vb) {
			VkDeviceSize offset = 0;
			vkCmdBindVertexBuffers(cmd, 0, 1, &vb, &offset);
			currentBindings.VertexBuffer = vb;
		}

		for (const VkPushConstantRange& pushConstant : mInstanceInfo.getShaderProgram().getDescriptorSetLayouts().getPushConstants()) {
			vkCmdPushConstants(
				cmd,
				mGraphicsPipelineLayout,
				pushConstant.stageFlags,
				pushConstant.offset,
				pushConstant.size,
				renderable.getPushConstantPtr()
			);
		}

		//TODO:: better way of calling the intended draw cmd. e.g. virtual renderable->draw(cmd) or switch(renderable->getDrawCmdType())
		if (renderable.isIndexed()) {
			IndexBufferVulkan& ib = renderable.getVao().getIndexBuffer();
			if (currentBindings.IndexBuffer != ib.getBuffer()) {
				ib.bind(cmd);
				currentBindings.IndexBuffer = ib.getBuffer();
			}

			vkCmdDrawIndexed(
				cmd,
				renderable.getVao().getDrawCount(),
				1,
				0,
				0,
				0
			);
		} else {
			vkCmdDraw(cmd, renderable.getVao().getDrawCount(), 1, 0, 0);
		}
	}

	void GraphicsPipelineVulkan::createPipelineLayout(VkDevice logicDevice) {
		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
		pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutCreateInfo.setLayoutCount = mInstanceInfo.getShaderProgram().getDescriptorSetLayouts().getSetCount();
		std::vector<VkDescriptorSetLayout> layouts = mInstanceInfo.getShaderProgram().getDescriptorSetLayouts().getNativeSetLayouts();
		pipelineLayoutCreateInfo.pSetLayouts = layouts.data();
		pipelineLayoutCreateInfo.pushConstantRangeCount = mInstanceInfo.getShaderProgram().getDescriptorSetLayouts().getPushConstantCount();
		pipelineLayoutCreateInfo.pPushConstantRanges = mInstanceInfo.getShaderProgram().getDescriptorSetLayouts().getPushConstants().data();

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

	void GraphicsPipelineVulkan::createPipeline(VkDevice logicDevice, VkExtent2D extent, VkRenderPass renderPass) {
		ZoneScoped;

		//Make sure shaders are loaded
		//TODO:: Checking for if individual shaders are loaded
		if (!mInstanceInfo.getShaderProgram().areShadersLoaded()) {
			mInstanceInfo.getShaderProgram().init(logicDevice);
		}

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

		//IMPORTANT:: in DOH vertex input binding is always slot 0
		const uint32_t binding = 0;
		const auto vertexAttribs = mInstanceInfo.getVertexInputLayout().asAttribDesc(binding);

		VkVertexInputBindingDescription vertexBindingDesc = {};
		vertexBindingDesc.binding = binding;
		vertexBindingDesc.stride = mInstanceInfo.getVertexInputLayout().getStride();
		vertexBindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.pVertexBindingDescriptions = &vertexBindingDesc;
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexAttribs.size());
		vertexInputInfo.pVertexAttributeDescriptions = vertexAttribs.data();

		VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		VkViewport viewport = {};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
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
		rasterizer.lineWidth = 1.0f;
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

		VkPipelineColorBlendStateCreateInfo colourBlending = {};
		colourBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colourBlending.logicOpEnable = VK_FALSE;
		colourBlending.attachmentCount = 1;
		colourBlending.pAttachments = &colourBlendAttachment;

		VkPipelineDepthStencilStateCreateInfo depthStencil = {};
		depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;

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

		//Optional Fields
		if (mInstanceInfo.hasOptionalFields()) {
			GraphicsPipelineOptionalFields& optFields = mInstanceInfo.getOptionalFields();

			inputAssembly.topology = optFields.Topology;

			rasterizer.polygonMode = optFields.PolygonMode;
			rasterizer.cullMode = optFields.CullMode;
			rasterizer.frontFace = optFields.FrontFace;

			colourBlendAttachment.blendEnable = optFields.BlendingEnabled ? VK_TRUE : VK_FALSE;
			colourBlendAttachment.srcColorBlendFactor = optFields.ColourBlendSrcFactor;
			colourBlendAttachment.dstColorBlendFactor = optFields.ColourBlendDstFactor;
			colourBlendAttachment.colorBlendOp = optFields.ColourBlendOp;
			colourBlendAttachment.srcAlphaBlendFactor = optFields.AlphaBlendSrcFactor;
			colourBlendAttachment.dstAlphaBlendFactor = optFields.AlphaBlendDstFactor;
			colourBlendAttachment.alphaBlendOp = optFields.AlphaBlendOp;

			depthStencil.depthTestEnable = optFields.DepthTestingEnabled ? VK_TRUE : VK_FALSE;
			depthStencil.depthWriteEnable = optFields.DepthTestingEnabled ? VK_TRUE : VK_FALSE;
			depthStencil.depthCompareOp = optFields.DepthCompareOp;
			depthStencil.depthBoundsTestEnable = VK_FALSE;
		} else {
			//Defaults for optional fields
			inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

			rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
			rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
			rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

			colourBlendAttachment.blendEnable = VK_FALSE;
			colourBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
			colourBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
			colourBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
			colourBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
			colourBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
			colourBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

			depthStencil.depthTestEnable = VK_FALSE;
			depthStencil.depthWriteEnable = VK_FALSE;
			depthStencil.depthCompareOp = VK_COMPARE_OP_NEVER;
			depthStencil.depthBoundsTestEnable = VK_FALSE;
		}

		VK_TRY(
			vkCreateGraphicsPipelines(logicDevice, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &mGraphicsPipeline),
			"Failed to create Graphics Pipeline."
		);

		mUsingGpuResource = true;

		//TODO:: Currently this auto closes shaders. Maybe have a way of not closing if more than one pipeline uses the same module(s)
		mInstanceInfo.getShaderProgram().close(logicDevice);
	}
}
