#include "dough/rendering/LineRenderer.h"

#include "dough/application/Application.h"
#include "dough/scene/geometry/primitives/Quad.h"

namespace DOH {

	void LineRenderer::init(VkDevice logicDevice, VkExtent2D swapChainExtent, ValueUniformInfo uboSize) {
		const auto& context = Application::get().getRenderer().getContext();
		{
			//Scene
			const StaticVertexInputLayout& sceneVertexLayout = StaticVertexInputLayout::get(SCENE_LINE_VERTEX_TYPE);
			const uint32_t line3dBatchSizeBytes = LINE_BATCH_MAX_LINE_COUNT * sceneVertexLayout.getStride();
			std::shared_ptr<VertexArrayVulkan> vao = context.createVertexArray();
			std::shared_ptr<VertexBufferVulkan> vbo = context.createVertexBuffer(
				sceneVertexLayout,
				static_cast<VkDeviceSize>(line3dBatchSizeBytes),
				VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
			);
			vao->addVertexBuffer(vbo);
			vao->getVertexBuffers()[0]->map(logicDevice, line3dBatchSizeBytes);

			//TODO:: Index buffer usage?

			mSceneLineRenderable = std::make_shared<SimpleRenderable>(vao, false);

			mSceneLineShaderProgram = context.createShaderProgram(
				context.createShader(EShaderType::VERTEX, mSceneLineRendererVertexShaderPath),
				context.createShader(EShaderType::FRAGMENT, mSceneLineRendererFragmentShaderPath)
			);

			ShaderUniformLayout& layout = mSceneLineShaderProgram->getUniformLayout();
			layout.setValue(0, uboSize);

			mSceneLineGraphicsPipelineInfo = std::make_unique<GraphicsPipelineInstanceInfo>(
				sceneVertexLayout,
				*mSceneLineShaderProgram,
				ERenderPass::APP_SCENE
			);
			mSceneLineGraphicsPipelineInfo->getOptionalFields().PolygonMode = VK_POLYGON_MODE_LINE;
			mSceneLineGraphicsPipelineInfo->getOptionalFields().CullMode = VK_CULL_MODE_NONE;
			mSceneLineGraphicsPipelineInfo->getOptionalFields().Topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
			//mSceneLineGraphicsPipelineInfo->getOptionalFields().Topology = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
			mSceneLineGraphicsPipelineInfo->getOptionalFields().ClearRenderablesAfterDraw = false;

			//TODO:: Allow depth testing for line rendering?
			//mSceneLineGraphicsPipelineInfo->getOptionalFields().setDepthTesting(true, VK_COMPARE_OP_LESS_OR_EQUAL);

			mSceneLineGraphicsPipeline = context.createGraphicsPipeline(*mSceneLineGraphicsPipelineInfo, swapChainExtent);
			mSceneLineBatch = std::make_unique<RenderBatchLineList>(SCENE_LINE_VERTEX_TYPE, LINE_BATCH_MAX_LINE_COUNT);
			mSceneLineGraphicsPipeline->addRenderableToDraw(mSceneLineRenderable);
		}

		{
			//UI
			const StaticVertexInputLayout& uiVertexLayout = StaticVertexInputLayout::get(UI_LINE_VERTEX_TYPE);
			const uint32_t line2dBatchSizeBytes = LINE_BATCH_MAX_LINE_COUNT * uiVertexLayout.getStride();
			std::shared_ptr<VertexArrayVulkan> vao = context.createVertexArray();
			std::shared_ptr<VertexBufferVulkan> vbo = context.createVertexBuffer(
				uiVertexLayout,
				static_cast<VkDeviceSize>(line2dBatchSizeBytes),
				VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
			);
			vao->addVertexBuffer(vbo);
			vao->getVertexBuffers()[0]->map(logicDevice, line2dBatchSizeBytes);

			//TODO:: Index buffer usage

			mUiLineRenderable = std::make_shared<SimpleRenderable>(vao, false);
			mUiLineShaderProgram = context.createShaderProgram(
				context.createShader(EShaderType::VERTEX, mUiLineRendererVertexShaderPath),
				context.createShader(EShaderType::FRAGMENT, mUiLineRendererFragmentShaderPath)
			);

			ShaderUniformLayout& layout = mUiLineShaderProgram->getUniformLayout();
			layout.setValue(0, uboSize);

			mUiLineGraphicsPipelineInfo = std::make_unique<GraphicsPipelineInstanceInfo>(
				uiVertexLayout,
				*mUiLineShaderProgram,
				ERenderPass::APP_UI
			);
			mUiLineGraphicsPipelineInfo->getOptionalFields().PolygonMode = VK_POLYGON_MODE_LINE;
			mUiLineGraphicsPipelineInfo->getOptionalFields().CullMode = VK_CULL_MODE_NONE;
			mUiLineGraphicsPipelineInfo->getOptionalFields().Topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
			mUiLineGraphicsPipelineInfo->getOptionalFields().ClearRenderablesAfterDraw = false;

			//TODO:: Allow depth testing for line rendering?
			//mUiLineGraphicsPipelineInfo->getOptionalFields().setDepthTesting(true, VK_COMPARE_OP_LESS_OR_EQUAL);

			mUiLineGraphicsPipeline = context.createGraphicsPipeline(*mUiLineGraphicsPipelineInfo, swapChainExtent);
			mUiLineBatch = std::make_unique<RenderBatchLineList>(UI_LINE_VERTEX_TYPE, LINE_BATCH_MAX_LINE_COUNT);
			mUiLineGraphicsPipeline->addRenderableToDraw(mUiLineRenderable);
		}
	}

	void LineRenderer::close(VkDevice logicDevice) {
		mSceneLineRenderable->getVao().getVertexBuffers()[0]->unmap(logicDevice);
		mSceneLineRenderable->getVao().close(logicDevice);
		mSceneLineShaderProgram->close(logicDevice);
		mSceneLineGraphicsPipeline->close(logicDevice);

		mUiLineRenderable->getVao().getVertexBuffers()[0]->unmap(logicDevice);
		mUiLineRenderable->getVao().close(logicDevice);
		mUiLineShaderProgram->close(logicDevice);
		mUiLineGraphicsPipeline->close(logicDevice);
	}

	void LineRenderer::drawScene(
		VkDevice logicDevice,
		const uint32_t imageIndex,
		void* ubo,
		size_t uboSize,
		VkCommandBuffer cmd,
		CurrentBindingsState& currentBindings,
		AppDebugInfo& debugInfo
	) {
		if (mSceneLineGraphicsPipeline->isEnabled() && mSceneLineBatch->getLineCount() > 0) {
			const uint32_t binding = 0;
			mSceneLineGraphicsPipeline->setFrameUniformData(
				logicDevice,
				imageIndex,
				binding,
				ubo,
				uboSize
			);

			if (currentBindings.Pipeline != mSceneLineGraphicsPipeline->get()) {
				mSceneLineGraphicsPipeline->bind(cmd);
				debugInfo.PipelineBinds++;
				currentBindings.Pipeline = mSceneLineGraphicsPipeline->get();
			}

			mSceneLineRenderable->getVao().setDrawCount(mSceneLineBatch->getVertexCount());

			mSceneLineGraphicsPipeline->recordDrawCommands(imageIndex, cmd, currentBindings);
			debugInfo.SceneDrawCalls += mSceneLineGraphicsPipeline->getVaoDrawCount();

			mSceneLineRenderable->getVao().getVertexBuffers()[0]->setDataMapped(
				logicDevice,
				mSceneLineBatch->getData().data(),
				mSceneLineBatch->getLineCount() * RenderBatchLineList::LINE_3D_SIZE
			);
		}

		mSceneLineBatch->reset();
	}

	void LineRenderer::drawUi(
		VkDevice logicDevice,
		const uint32_t imageIndex,
		void* ubo,
		size_t uboSize,
		VkCommandBuffer cmd,
		CurrentBindingsState& currentBindings,
		AppDebugInfo& debugInfo
	) {
		if (mUiLineGraphicsPipeline->isEnabled() && mUiLineBatch->getLineCount() > 0) {
			const uint32_t binding = 0;
			mUiLineGraphicsPipeline->setFrameUniformData(
				logicDevice,
				imageIndex,
				binding,
				ubo,
				uboSize
			);

			if (currentBindings.Pipeline != mUiLineGraphicsPipeline->get()) {
				mUiLineGraphicsPipeline->bind(cmd);
				debugInfo.PipelineBinds++;
				currentBindings.Pipeline = mUiLineGraphicsPipeline->get();
			}

			mUiLineRenderable->getVao().setDrawCount(mUiLineBatch->getVertexCount());

			mUiLineGraphicsPipeline->recordDrawCommands(imageIndex, cmd, currentBindings);
			debugInfo.UiDrawCalls += mUiLineGraphicsPipeline->getVaoDrawCount();

			mUiLineRenderable->getVao().getVertexBuffers()[0]->setDataMapped(
				logicDevice,
				mUiLineBatch->getData().data(),
				mUiLineBatch->getLineCount() * RenderBatchLineList::LINE_2D_SIZE
			);
		}

		mUiLineBatch->reset();
	}

	void LineRenderer::recreateGraphicsPipelines(
		VkDevice logicDevice,
		VkExtent2D extent,
		const RenderPassVulkan& sceneRenderPass,
		const RenderPassVulkan& uiRenderPass,
		VkDescriptorPool descPool
	) {
		auto& context = Application::get().getRenderer().getContext();
		mSceneLineGraphicsPipeline->recreate(logicDevice, extent, sceneRenderPass.get());
		context.createPipelineUniformObjects(*mSceneLineGraphicsPipeline, descPool);

		mUiLineGraphicsPipeline->recreate(logicDevice, extent, uiRenderPass.get());
		context.createPipelineUniformObjects(*mUiLineGraphicsPipeline, descPool);
	}

	void LineRenderer::createShaderUniforms(VkDevice logicDevice, VkPhysicalDevice physicalDevice, const uint32_t imageCount, VkDescriptorPool descPool) {
		mSceneLineGraphicsPipeline->createShaderUniforms(logicDevice, physicalDevice, imageCount, descPool);
		mUiLineGraphicsPipeline->createShaderUniforms(logicDevice, physicalDevice, imageCount, descPool);
	}
	
	void LineRenderer::updateShaderUniforms(VkDevice logicDevice, const uint32_t imageCount) {
		mSceneLineGraphicsPipeline->updateShaderUniforms(logicDevice, imageCount);
		mUiLineGraphicsPipeline->updateShaderUniforms(logicDevice, imageCount);
	}

	const std::vector<DescriptorTypeInfo> LineRenderer::getDescriptorTypeInfo() const {
		std::vector<DescriptorTypeInfo> sceneDescTypes = mSceneLineGraphicsPipeline->getShaderProgram().getShaderDescriptorLayout().asDescriptorTypes();
		std::vector<DescriptorTypeInfo> uiDescTypes = mUiLineGraphicsPipeline->getShaderProgram().getShaderDescriptorLayout().asDescriptorTypes();

		const size_t sceneDescTypesSize = sceneDescTypes.size();
		const size_t uiDescTypesSize = uiDescTypes.size();
		const size_t totalSize = sceneDescTypesSize + uiDescTypesSize;
		sceneDescTypes.reserve(totalSize);

		for (size_t i = 0; i < uiDescTypesSize; i++) {
			sceneDescTypes.emplace_back(uiDescTypes[i]);
		}

		return sceneDescTypes;
	}

	void LineRenderer::drawLineScene(const glm::vec3& start, const glm::vec3& end, const glm::vec4& colour) {
		if (mSceneLineBatch->hasSpace()) {
			mSceneLineBatch->add3d(start, end, colour);
		} else {
			LOG_WARN("Scene line batch max line count reached: " << mSceneLineBatch->getMaxLineCount());
		}
	}

	void LineRenderer::drawLineUi(const glm::vec2& start, const glm::vec2& end, const glm::vec4& colour) {
		if (mUiLineBatch->hasSpace()) {
			mUiLineBatch->add2d(start, end, colour);
		} else {
			LOG_WARN("UI line batch max line count reached: " << mUiLineBatch->getMaxLineCount());
		}
	}

	void LineRenderer::drawQuadScene(const Quad& quad, const glm::vec4& colour) {
		if (mSceneLineBatch->hasSpace(4)) {
			glm::vec3 botLeft = { quad.Position.x, quad.Position.y, quad.Position.z };
			glm::vec3 botRight = { quad.Position.x + quad.Size.x, quad.Position.y, quad.Position.z };
			glm::vec3 topLeft = { quad.Position.x, quad.Position.y + quad.Size.y, quad.Position.z };
			glm::vec3 topRight = { quad.Position.x + quad.Size.x, quad.Position.y + quad.Size.y, quad.Position.z };

			mSceneLineBatch->add3d(botLeft, topLeft, colour);
			mSceneLineBatch->add3d(topLeft, topRight, colour);
			mSceneLineBatch->add3d(topRight, botRight, colour);
			mSceneLineBatch->add3d(botRight, botLeft, colour);
		} else {
			LOG_WARN("Scene line batch does not have space for " << 4 << " more lines.");
		}
	}

	void LineRenderer::drawQuadUi(const Quad& quad, const glm::vec4& colour) {
		if (mUiLineBatch->hasSpace(4)) {
			glm::vec3 botLeft = { quad.Position.x, quad.Position.y, quad.Position.z };
			glm::vec3 botRight = { quad.Position.x + quad.Size.x, quad.Position.y, quad.Position.z };
			glm::vec3 topLeft = { quad.Position.x, quad.Position.y + quad.Size.y, quad.Position.z };
			glm::vec3 topRight = { quad.Position.x + quad.Size.x, quad.Position.y + quad.Size.y, quad.Position.z };

			mUiLineBatch->add2d(botLeft, topLeft, colour);
			mUiLineBatch->add2d(topLeft, topRight, colour);
			mUiLineBatch->add2d(topRight, botRight, colour);
			mUiLineBatch->add2d(botRight, botLeft, colour);
		} else {
			LOG_WARN("Ui line batch does not have space for " << 4 << " more lines.");
		}
	}
}
