#include "dough/rendering/LineRenderer.h"

#include "dough/application/Application.h"
#include "dough/scene/geometry/primitives/Quad.h"
#include "dough/rendering/pipeline/ShaderDescriptorSetLayoutsVulkan.h"
#include "dough/rendering/SwapChainVulkan.h"
#include "dough/ImGuiWrapper.h"

#include <tracy/public/tracy/Tracy.hpp>

namespace DOH {

	std::unique_ptr<LineRenderer> LineRenderer::INSTANCE = nullptr;

	const char* LineRenderer::SCENE_LINE_SHADER_PATH_VERT = "Dough/Dough/res/shaders/spv/LineRenderer3d.vert.spv";
	const char* LineRenderer::SCENE_LINE_SHADER_PATH_FRAG = "Dough/Dough/res/shaders/spv/LineRenderer3d.frag.spv";
	const char* LineRenderer::UI_LINE_SHADER_PATH_VERT = "Dough/Dough/res/shaders/spv/LineRenderer2d.vert.spv";
	const char* LineRenderer::UI_LINE_SHADER_PATH_FRAG = "Dough/Dough/res/shaders/spv/LineRenderer2d.frag.spv";
	const char* LineRenderer::NAME_SHORT_HAND = "LineRdr";
	const char* LineRenderer::NAME_LONG_HAND = "LineRenderer";

	void LineRenderingObjects::addOwnedResourcesToClose(RenderingContextVulkan& context) {
		ShaderProgram->addOwnedResourcesToClose(context);
		context.addGpuResourceToClose(VertexShader);
		context.addGpuResourceToClose(FragmentShader);
		context.addGpuResourceToClose(GraphicsPipeline);
		context.addGpuResourceToClose(Renderable->getVaoPtr());
	}

	LineRenderer::LineRenderer(RenderingContextVulkan& context)
	:	mContext(context),
		mWarnOnNullSceneCameraData(true),
		mWarnOnNullUiCameraData(true)
	{}

	void LineRenderer::initImpl() {
		ZoneScoped;

		std::vector<std::reference_wrapper<DescriptorSetLayoutVulkan>> linePipelinesDescSets = { mContext.getCommonDescriptorSetLayouts().Ubo };
		//NOTE:: Line Renderer currently uses the same descriptor set layouts so the one instance can be used for both SCENE and UI
		std::shared_ptr<ShaderDescriptorSetLayoutsVulkan> lineShadersDescSetLayouts = std::make_shared<ShaderDescriptorSetLayoutsVulkan>(linePipelinesDescSets);
		std::shared_ptr<DescriptorSetsInstanceVulkan> descSetInstance = std::make_shared<DescriptorSetsInstanceVulkan>(1);
		descSetInstance->setDescriptorSetArray(CAMERA_UBO_SLOT, { VK_NULL_HANDLE, VK_NULL_HANDLE }); //Camera UBO
		{
			//Scene
			const StaticVertexInputLayout& sceneVertexLayout = StaticVertexInputLayout::get(SCENE_LINE_VERTEX_TYPE);
			const uint32_t line3dBatchSizeBytes = LINE_BATCH_MAX_LINE_COUNT * sceneVertexLayout.getStride();
			std::shared_ptr<VertexArrayVulkan> vao = mContext.createVertexArray();
			std::shared_ptr<VertexBufferVulkan> vbo = mContext.createVertexBuffer(
				sceneVertexLayout,
				static_cast<VkDeviceSize>(line3dBatchSizeBytes),
				VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
			);
			vao->addVertexBuffer(vbo);
			vao->getVertexBuffers()[0]->map(mContext.getLogicDevice(), line3dBatchSizeBytes);

			//TODO:: Index buffer usage?

			mSceneLineList = std::make_unique<LineRenderingObjects>();

			mSceneLineList->Renderable = std::make_shared<SimpleRenderable>(vao, descSetInstance, false);

			mSceneLineList->VertexShader = mContext.createShader(EShaderStage::VERTEX, SCENE_LINE_SHADER_PATH_VERT);
			mSceneLineList->FragmentShader = mContext.createShader(EShaderStage::FRAGMENT, SCENE_LINE_SHADER_PATH_FRAG);
			mSceneLineList->ShaderProgram = mContext.createShaderProgram(
				mSceneLineList->VertexShader,
				mSceneLineList->FragmentShader,
				lineShadersDescSetLayouts
			);

			mSceneLineList->GraphicsPipelineInfo = std::make_unique<GraphicsPipelineInstanceInfo>(
				sceneVertexLayout,
				*mSceneLineList->ShaderProgram,
				ERenderPass::APP_SCENE
			);
			auto& optionalFields = mSceneLineList->GraphicsPipelineInfo->enableOptionalFields();
			optionalFields.PolygonMode = VK_POLYGON_MODE_LINE;
			optionalFields.CullMode = VK_CULL_MODE_NONE;
			optionalFields.Topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
			//optionalFields.Topology = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
			optionalFields.ClearRenderablesAfterDraw = false;

			//TODO:: Allow depth testing for line rendering?
			//optionalFields.setDepthTesting(true, VK_COMPARE_OP_LESS_OR_EQUAL);

			mSceneLineList->GraphicsPipeline = mContext.createGraphicsPipeline(*mSceneLineList->GraphicsPipelineInfo);
			mSceneLineList->GraphicsPipeline->init(
				mContext.getLogicDevice(),
				mContext.getSwapChain().getExtent(),
				mContext.getRenderPass(ERenderPass::APP_SCENE).get()
			);
			mSceneLineList->Batch = std::make_unique<RenderBatchLineList>(SCENE_LINE_VERTEX_TYPE, LINE_BATCH_MAX_LINE_COUNT);
			mSceneLineList->GraphicsPipeline->addRenderableToDraw(mSceneLineList->Renderable);
		}

		{
			//UI
			const StaticVertexInputLayout& uiVertexLayout = StaticVertexInputLayout::get(UI_LINE_VERTEX_TYPE);
			const uint32_t line2dBatchSizeBytes = LINE_BATCH_MAX_LINE_COUNT * uiVertexLayout.getStride();
			std::shared_ptr<VertexArrayVulkan> vao = mContext.createVertexArray();
			std::shared_ptr<VertexBufferVulkan> vbo = mContext.createVertexBuffer(
				uiVertexLayout,
				static_cast<VkDeviceSize>(line2dBatchSizeBytes),
				VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
			);
			vao->addVertexBuffer(vbo);
			vao->getVertexBuffers()[0]->map(mContext.getLogicDevice(), line2dBatchSizeBytes);

			//TODO:: Index buffer usage

			mUiLineList = std::make_unique<LineRenderingObjects>();

			mUiLineList->Renderable = std::make_shared<SimpleRenderable>(vao, descSetInstance, false);
			mUiLineList->VertexShader = mContext.createShader(EShaderStage::VERTEX, UI_LINE_SHADER_PATH_VERT);
			mUiLineList->FragmentShader = mContext.createShader(EShaderStage::FRAGMENT, UI_LINE_SHADER_PATH_FRAG);
			mUiLineList->ShaderProgram = mContext.createShaderProgram(
				mUiLineList->VertexShader,
				mUiLineList->FragmentShader,
				lineShadersDescSetLayouts
			);

			mUiLineList->GraphicsPipelineInfo = std::make_unique<GraphicsPipelineInstanceInfo>(
				uiVertexLayout,
				*mUiLineList->ShaderProgram,
				ERenderPass::APP_UI
			);
			auto& optionalFields = mUiLineList->GraphicsPipelineInfo->enableOptionalFields();
			optionalFields.PolygonMode = VK_POLYGON_MODE_LINE;
			optionalFields.CullMode = VK_CULL_MODE_NONE;
			optionalFields.Topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
			optionalFields.ClearRenderablesAfterDraw = false;

			//TODO:: Allow depth testing for line rendering?
			//mUiLineGraphicsPipelineInfo->getOptionalFields().setDepthTesting(true, VK_COMPARE_OP_LESS_OR_EQUAL);

			mUiLineList->GraphicsPipeline = mContext.createGraphicsPipeline(*mUiLineList->GraphicsPipelineInfo);
			mUiLineList->GraphicsPipeline->init(
				mContext.getLogicDevice(),
				mContext.getSwapChain().getExtent(),
				mContext.getRenderPass(ERenderPass::APP_UI).get()
			);
			mUiLineList->Batch = std::make_unique<RenderBatchLineList>(UI_LINE_VERTEX_TYPE, LINE_BATCH_MAX_LINE_COUNT);
			mUiLineList->GraphicsPipeline->addRenderableToDraw(mUiLineList->Renderable);
		}
	}

	void LineRenderer::closeImpl() {
		ZoneScoped;

		mSceneLineList->addOwnedResourcesToClose(mContext);
		mUiLineList->addOwnedResourcesToClose(mContext);
	}

	void LineRenderer::drawSceneImpl(
		uint32_t imageIndex,
		VkCommandBuffer cmd,
		CurrentBindingsState& currentBindings
	) {
		ZoneScoped;

		if (mSceneCameraData == nullptr) {
			if (mWarnOnNullSceneCameraData) {
				LOG_WARN("ShapeRenderer::drawSceneImpl mSceneCameraData is null");
			}
			return;
		}

		AppDebugInfo& debugInfo = Application::get().getDebugInfo();
		const uint32_t lineCount = mSceneLineList->Batch->getLineCount();

		if (lineCount > 0) {
			if (currentBindings.Pipeline != mSceneLineList->GraphicsPipeline->get()) {
				mSceneLineList->GraphicsPipeline->bind(cmd);
				debugInfo.PipelineBinds++;
				currentBindings.Pipeline = mSceneLineList->GraphicsPipeline->get();
			}

			mSceneLineList->Renderable->getVao().setDrawCount(mSceneLineList->Batch->getVertexCount());
			mSceneLineList->Renderable->getVao().getVertexBuffers()[0]->setDataMapped(
				mContext.getLogicDevice(),
				mSceneLineList->Batch->getData().data(),
				lineCount * RenderBatchLineList::LINE_3D_SIZE
			);

			mSceneLineList->GraphicsPipeline->recordDrawCommand(imageIndex, cmd, *mSceneLineList->Renderable, currentBindings, 0);
			debugInfo.SceneDrawCalls++;
		}

		mSceneLineList->Batch->reset();
	}

	void LineRenderer::drawUiImpl(
		uint32_t imageIndex,
		VkCommandBuffer cmd,
		CurrentBindingsState& currentBindings
	) {
		ZoneScoped;

		if (mUiCameraData == nullptr) {
			if (mWarnOnNullUiCameraData) {
				LOG_WARN("LineRenderer::drawUiImpl mUiCameraData is null");
			}
			return;
		}

		AppDebugInfo& debugInfo = Application::get().getDebugInfo();
		const uint32_t lineCount = mUiLineList->Batch->getLineCount();

		if (lineCount > 0) {
			if (currentBindings.Pipeline != mUiLineList->GraphicsPipeline->get()) {
				mUiLineList->GraphicsPipeline->bind(cmd);
				debugInfo.PipelineBinds++;
				currentBindings.Pipeline = mUiLineList->GraphicsPipeline->get();
			}

			mUiLineList->Renderable->getVao().setDrawCount(mUiLineList->Batch->getVertexCount());

			mUiLineList->Renderable->getVao().getVertexBuffers()[0]->setDataMapped(
				mContext.getLogicDevice(),
				mUiLineList->Batch->getData().data(),
				lineCount * RenderBatchLineList::LINE_2D_SIZE
			);

			mUiLineList->GraphicsPipeline->recordDrawCommand(imageIndex, cmd, *mUiLineList->Renderable, currentBindings, 0);
			debugInfo.UiDrawCalls++;
		}

		mUiLineList->Batch->reset();
	}

	void LineRenderer::onSwapChainResizeImpl(SwapChainVulkan& swapChain) {
		ZoneScoped;

		mSceneLineList->GraphicsPipeline->resize(
			mContext.getLogicDevice(),
			swapChain.getExtent(),
			mContext.getRenderPass(ERenderPass::APP_SCENE).get()
		);
		mUiLineList->GraphicsPipeline->resize(
			mContext.getLogicDevice(),
			swapChain.getExtent(),
			mContext.getRenderPass(ERenderPass::APP_UI).get()
		);
	}

	void LineRenderer::drawLineSceneImpl(const glm::vec3& start, const glm::vec3& end, const glm::vec4& colour) {
		ZoneScoped;

		if (mSceneLineList->Batch->hasSpace()) {
			mSceneLineList->Batch->add3d(start, end, colour);
		} else {
			LOG_WARN("Scene line batch max line count reached: " << mSceneLineList->Batch->getMaxLineCount());
		}
	}

	void LineRenderer::drawLineUiImpl(const glm::vec2& start, const glm::vec2& end, const glm::vec4& colour) {
		ZoneScoped;

		if (mUiLineList->Batch->hasSpace()) {
			mUiLineList->Batch->add2d(start, end, colour);
		} else {
			LOG_WARN("UI line batch max line count reached: " << mUiLineList->Batch->getMaxLineCount());
		}
	}

	void LineRenderer::drawQuadSceneImpl(const Quad& quad, const glm::vec4& colour) {
		ZoneScoped;

		if (mSceneLineList->Batch->hasSpace(4)) {
			glm::vec3 botLeft = { quad.Position.x, quad.Position.y, quad.Position.z };
			glm::vec3 botRight = { quad.Position.x + quad.Size.x, quad.Position.y, quad.Position.z };
			glm::vec3 topLeft = { quad.Position.x, quad.Position.y + quad.Size.y, quad.Position.z };
			glm::vec3 topRight = { quad.Position.x + quad.Size.x, quad.Position.y + quad.Size.y, quad.Position.z };

			mSceneLineList->Batch->add3d(botLeft, topLeft, colour);
			mSceneLineList->Batch->add3d(topLeft, topRight, colour);
			mSceneLineList->Batch->add3d(topRight, botRight, colour);
			mSceneLineList->Batch->add3d(botRight, botLeft, colour);
		} else {
			LOG_WARN("Scene line batch does not have space for " << 4 << " more lines.");
		}
	}

	void LineRenderer::drawQuadUiImpl(const Quad& quad, const glm::vec4& colour) {
		ZoneScoped;

		if (mUiLineList->Batch->hasSpace(4)) {
			glm::vec3 botLeft = { quad.Position.x, quad.Position.y, quad.Position.z };
			glm::vec3 botRight = { quad.Position.x + quad.Size.x, quad.Position.y, quad.Position.z };
			glm::vec3 topLeft = { quad.Position.x, quad.Position.y + quad.Size.y, quad.Position.z };
			glm::vec3 topRight = { quad.Position.x + quad.Size.x, quad.Position.y + quad.Size.y, quad.Position.z };

			mUiLineList->Batch->add2d(botLeft, topLeft, colour);
			mUiLineList->Batch->add2d(topLeft, topRight, colour);
			mUiLineList->Batch->add2d(topRight, botRight, colour);
			mUiLineList->Batch->add2d(botRight, botLeft, colour);
		} else {
			LOG_WARN("Ui line batch does not have space for " << 4 << " more lines.");
		}
	}

	void LineRenderer::init(RenderingContextVulkan& context) {
		if (INSTANCE == nullptr) {
			INSTANCE = std::make_unique<LineRenderer>(context);
			INSTANCE->initImpl();
		} else {
			LOG_WARN("LineRenderer::init called when already initialised.");
		}
	}

	void LineRenderer::close() {
		if (INSTANCE != nullptr) {
			INSTANCE->closeImpl();
			INSTANCE.release();
			INSTANCE = nullptr;
		} else {
			LOG_WARN("LineRenderer::close called when not initialised.");
		}
	}

	void LineRenderer::onSwapChainResize(SwapChainVulkan& swapChain) {
		if (INSTANCE != nullptr) {
			INSTANCE->onSwapChainResizeImpl(swapChain);
		} else {
			LOG_WARN("LineRenderer::onSwapChainResize called when not initialised.");
		}
	}

	void LineRenderer::drawScene(uint32_t imageIndex, VkCommandBuffer cmd, CurrentBindingsState& currentBindings) {
		if (INSTANCE != nullptr) {
			INSTANCE->drawSceneImpl(imageIndex, cmd, currentBindings);
		} else {
			LOG_WARN("LineRenderer::drawScene called when not initialised.");
		}
	}

	void LineRenderer::drawUi(uint32_t imageIndex, VkCommandBuffer cmd, CurrentBindingsState& currentBindings) {
		if (INSTANCE != nullptr) {
			INSTANCE->drawUiImpl(imageIndex, cmd, currentBindings);
		} else {
			LOG_WARN("LineRenderer::drawUi called when not initialised.");
		}
	}

	void LineRenderer::setSceneCameraDataImpl(std::shared_ptr<CameraGpuData> cameraData) {
		mSceneCameraData = cameraData;
		mSceneLineList->Renderable->getDescriptorSetsInstance()->setDescriptorSetArray(LineRenderer::CAMERA_UBO_SLOT, { cameraData->DescriptorSets[0], cameraData->DescriptorSets[1] });
	}

	void LineRenderer::setUiCameraDataImpl(std::shared_ptr<CameraGpuData> cameraData) {
		mUiCameraData = cameraData;
		mUiLineList->Renderable->getDescriptorSetsInstance()->setDescriptorSetArray(LineRenderer::CAMERA_UBO_SLOT, { cameraData->DescriptorSets[0], cameraData->DescriptorSets[1] });
	}

	void LineRenderer::drawImGuiImpl(EImGuiContainerType type) {
		ZoneScoped;

		bool open = false;
		//IMPORTANT:: Works because there are only two options.
		bool isWindow = type == EImGuiContainerType::WINDOW;

		if (isWindow) {
			open = ImGui::Begin(LineRenderer::NAME_LONG_HAND);
		} else {
			open = ImGui::BeginTabItem(LineRenderer::NAME_SHORT_HAND);
		}


		if (open) {

			ImGui::Text("This is the line renderer");

			ImGui::Text("TODO:: This will be filled with related info.");

			//TODO:: Fill this out with info and controls.

		}

		if (isWindow) {
			ImGui::End();
		} else if (open) { //Here type MUST be TAB
			ImGui::EndTabItem();
		}
	}
}
