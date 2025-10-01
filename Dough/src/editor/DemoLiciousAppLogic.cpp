#include "editor/DemoLiciousAppLogic.h"

#include "editor/ui/EditorGui.h"

#include "dough/application/Application.h"
#include "dough/rendering/ShapeRenderer.h"
#include "dough/rendering/text/TextRenderer.h"

#include <tracy/public/tracy/Tracy.hpp>

#define GET_RENDERER Application::get().getRenderer()

namespace DOH::EDITOR {

	const char* DemoLiciousAppLogic::SharedDemoResources::TexturedShaderVertPath = "Dough/Dough/res/shaders/spv/Textured.vert.spv";
	const char* DemoLiciousAppLogic::SharedDemoResources::TexturedShaderFragPath = "Dough/Dough/res/shaders/spv/Textured.frag.spv";
	const char* DemoLiciousAppLogic::SharedDemoResources::PERSP_SCENE_CAM_NAME = "SharedCam_PerspScene";
	const char* DemoLiciousAppLogic::SharedDemoResources::ORTHO_UI_CAM_NAME = "SharedCam_OrthoUi";
	const char* DemoLiciousAppLogic::MultiCameraPassDemo::ORTHO_SCENE_CAM_NAME = "MultiCam_OrthoScene";
	const char* DemoLiciousAppLogic::MultiCameraPassDemo::PERSP_UI_CAM_NAME = "MultiCam_PerspUi";

	void DemoLiciousAppLogic::init(float aspectRatio) {
		ZoneScoped;

		mImGuiSettings = std::make_unique<ImGuiSettings>();

		initDemos(aspectRatio);
	}

	void DemoLiciousAppLogic::update(float delta) {
		ZoneScoped;

		mSharedDemoResources->PerspectiveSceneCameraController->onUpdate(delta);
		mSharedDemoResources->OrthoUiCameraController->onUpdate(delta);

		mShapesDemo->update(delta);
		mCustomDemo->update(delta);
		mObjModelsDemo->update(delta);
		mTextDemo->update(delta);
		mLineDemo->update(delta);
		mBoundingBoxDemo->update(delta);
		mTileMapDemo->update(delta);
		mMultiCameraPassDemo->update(delta);
	}

	void DemoLiciousAppLogic::render() {
		ZoneScoped;

		mShapesDemo->render();
		mCustomDemo->render();
		mObjModelsDemo->render();
		mTextDemo->render();
		mLineDemo->render();
		mBoundingBoxDemo->render();
		mTileMapDemo->render();
		mMultiCameraPassDemo->render();
	}

	void DemoLiciousAppLogic::imGuiRender(float delta) {
		ZoneScoped;

		RendererVulkan& renderer = GET_RENDERER;

		if (ImGui::Begin("DemoLicious Demos")) {
			static bool sharedResourcesCollapse = true;
			if (sharedResourcesCollapse = ImGui::CollapsingHeader("Shared Resources")) {
				ImGui::Text("SharedCam_PerspScene Controls");
				EditorGui::controlsEditorPerspectiveCameraController(*mSharedDemoResources->PerspectiveSceneCameraController, SharedDemoResources::PERSP_SCENE_CAM_NAME);
				ImGui::Text("SharedCam_OrthoUi Controls");
				EditorGui::controlsEditorOrthoCameraController(*mSharedDemoResources->OrthoUiCameraController, SharedDemoResources::ORTHO_UI_CAM_NAME);

				ImGui::Text("MultiCam_PerspUi Controls");
				EditorGui::controlsEditorPerspectiveCameraController(*mMultiCameraPassDemo->PerspectiveUiCameraController, MultiCameraPassDemo::PERSP_UI_CAM_NAME);
				ImGui::Text("MultiCam_OrthoScene Controls");
				EditorGui::controlsEditorOrthoCameraController(*mMultiCameraPassDemo->OrthoSceneCameraController, MultiCameraPassDemo::ORTHO_SCENE_CAM_NAME);
			}

			ImGui::Text("Demo Settings & Info:");

			if (ImGui::BeginCombo("Demo", mSelectedDemoIndex != UINT32_MAX ? mDemos[mSelectedDemoIndex].get().getName() : "Click Here To Select a Demo")) {
				uint32_t i = 0;
				for (auto& demo : mDemos) {
					bool selected = false;
					ImGui::Selectable(demo.get().getName(), &selected);
					if (selected) {
						mSelectedDemoIndex = i;
					}
					i++;
				}
				ImGui::EndCombo();
			}

			if (mSelectedDemoIndex != UINT32_MAX) {
				mDemos[mSelectedDemoIndex].get().renderImGuiMainTab();
			}

			ImGui::NewLine();
			ImGui::Text("Cross-demo functions/resources");
			if (ImGui::Button("Disable all demos")) {
				mShapesDemo->mGridDemo->Render = false;
				mShapesDemo->mGridDemo->Update = false;
				
				mShapesDemo->mBouncingQuadDemo->Render = false;
				mShapesDemo->mBouncingQuadDemo->Update = false;
				
				mShapesDemo->mCircleDemo->Render = false;
				mShapesDemo->mCircleDemo->Update = false;
				
				mCustomDemo->RenderScene = false;
				mCustomDemo->RenderUi = false;
				mCustomDemo->Update = false;

				mObjModelsDemo->Render = false;
				mObjModelsDemo->Update = false;

				mTextDemo->Render = false;
				mTextDemo->Update = false;
				
				mLineDemo->Render = false;
				mLineDemo->Update = false;

				mBoundingBoxDemo->Render = false;
				mBoundingBoxDemo->Update = false;

				mMultiCameraPassDemo->Render = false;
				mMultiCameraPassDemo->Update = false;
			}
			if (ImGui::Button("View atlas texture")) {
				EditorGui::openMonoSpaceTextureAtlasViewerWindow(*ShapeRenderer::getTestMonoSpaceTextureAtlas());
			}
			EditorGui::displayHelpTooltip("Display Texture Altas texture inside a separate window.");
			if (ImGui::Button("View test texture")) {
				EditorGui::openTextureViewerWindow(*mSharedDemoResources->TestTexture1);
			}
			EditorGui::displayHelpTooltip("Display test texture inside a separate window.");
		}
		ImGui::End();


		mShapesDemo->renderImGuiExtras();
		mCustomDemo->renderImGuiExtras();
		mObjModelsDemo->renderImGuiExtras();
		mTextDemo->renderImGuiExtras();
		mLineDemo->renderImGuiExtras();
		mBoundingBoxDemo->renderImGuiExtras();
		mTileMapDemo->renderImGuiExtras();
		mMultiCameraPassDemo->renderImGuiExtras();
	}

	void DemoLiciousAppLogic::close() {
		ZoneScoped;

		closeDemos();
	}

	void DemoLiciousAppLogic::onResize(float aspectRatio) {
		ZoneScoped;

		mSharedDemoResources->AspectRatio = aspectRatio;
		updateCamerasAspectRatio(aspectRatio);
	}

	void DemoLiciousAppLogic::initDemos(float aspectRatio) {
		ZoneScoped;

		RenderingContextVulkan& context = GET_RENDERER.getContext();
	
		//Test grid is repopulated per update to apply changes from editor. To only populate once remove re-population from update and populate here.
		//populateTestGrid(static_cast<uint32_t>(mGridDemo.TestGridSize[0]), static_cast<uint32_t>(mGridDemo.TestGridSize[1]));

		initSharedResources(aspectRatio);
		mDemos.reserve(10);

		mShapesDemo = std::make_unique<ShapesDemo>(*mSharedDemoResources);
		mShapesDemo->init();
		mDemos.emplace_back(*mShapesDemo->mGridDemo);
		mDemos.emplace_back(*mShapesDemo->mBouncingQuadDemo);
		mDemos.emplace_back(*mShapesDemo->mCircleDemo);
		
		mCustomDemo = std::make_unique<CustomDemo>(*mSharedDemoResources);
		mCustomDemo->init();
		mDemos.emplace_back(*mCustomDemo);
		
		mObjModelsDemo = std::make_unique<ObjModelsDemo>(*mSharedDemoResources);
		mObjModelsDemo->init();
		mDemos.emplace_back(*mObjModelsDemo);
		
		mTextDemo = std::make_unique<TextDemo>(*mSharedDemoResources);
		mTextDemo->init();
		mDemos.emplace_back(*mTextDemo);
		
		mLineDemo = std::make_unique<LineDemo>(*mSharedDemoResources, *mTextDemo);
		mLineDemo->init();
		mDemos.emplace_back(*mLineDemo);
		
		mBoundingBoxDemo = std::make_unique<BoundingBoxDemo>(*mSharedDemoResources);
		mBoundingBoxDemo->init();
		mDemos.emplace_back(*mBoundingBoxDemo);
		
		mTileMapDemo = std::make_unique<TileMapDemo>(*mSharedDemoResources);
		mTileMapDemo->init();
		mDemos.emplace_back(*mTileMapDemo);

		mMultiCameraPassDemo = std::make_unique<MultiCameraPassDemo>(*mSharedDemoResources);
		mMultiCameraPassDemo->init();
		mDemos.emplace_back(*mMultiCameraPassDemo);
	}

	void DemoLiciousAppLogic::closeDemos() {
		ZoneScoped;

		if (mObjModelsDemo->GpuResourcesLoaded) {
			mObjModelsDemo->close();
		}
		mObjModelsDemo.release();
		
		if (mCustomDemo->GpuResourcesLoaded) {
			mCustomDemo->close();
		}
		mCustomDemo.release();
		
		mTextDemo->close();
		mTextDemo.release();
		
		mShapesDemo->close();
		mShapesDemo.release();
		
		mLineDemo->close();
		mLineDemo.release();
		
		mBoundingBoxDemo->close();
		mBoundingBoxDemo.release();
		
		mTileMapDemo->close();
		mTileMapDemo.release();

		mMultiCameraPassDemo->close();
		mMultiCameraPassDemo.release();

		closeSharedResources();
	}

	void DemoLiciousAppLogic::ShapesDemo::GridDemo::populateTestGrid(uint32_t width, uint32_t height) {
		ZoneScoped;

		const auto& atlas = ShapeRenderer::getTestMonoSpaceTextureAtlas();

		TexturedTestGrid.clear();
		TexturedTestGrid.resize(ShapeRenderer::getShapesTextureArray().getTextureSlots().size());
		const uint32_t textureSlot = ShapeRenderer::getShapesTextureArray().getTextureSlotIndex(atlas->getId());

		uint32_t index = 0;
		for (uint32_t y = 0; y < height; y++) {
			for (uint32_t x = 0; x < width; x++) {

				//Push back a Quad with both a texture and a custom colour,
				// in this case whether it is drawn with a texture or with a colour
				// is determined by mGridDemo.QuadDrawColour

				TexturedTestGrid[textureSlot].push_back({
					{
						static_cast<float>(x) * TestGridQuadGapSize[0],
						static_cast<float>(y) * TestGridQuadGapSize[1],
						0.5f
					},
					{ TestGridQuadSize[0], TestGridQuadSize[1] },
					{ QuadColour.x, QuadColour.y, QuadColour.z, QuadColour.w },
					0.0f,
					atlas,
					atlas->getInnerTextureCoords(
						x + TestTexturesRowOffset,
						y + TestTexturesColumnOffset
					)
				});
				index++;
			}
		}

		IsUpToDate = true;
	}

	void DemoLiciousAppLogic::initSharedResources(float aspectRatio) {
		ZoneScoped;

		RenderingContextVulkan& context = Application::get().getRenderer().getContext();
		DescriptorSetLayoutVulkan& textureSetLayout = context.getCommonDescriptorSetLayouts().SingleTexture;

		mSharedDemoResources = std::make_unique<SharedDemoResources>();
		mSharedDemoResources->AspectRatio = aspectRatio;

		mSharedDemoResources->InputLayer = std::make_shared<DefaultInputLayer>("DemoLicious_Shared");
		Input::addInputLayer(mSharedDemoResources->InputLayer);

		//Shared Cameras
		mSharedDemoResources->PerspectiveSceneCamera = std::make_unique<DOH::PerspectiveCamera>(mSharedDemoResources->AspectRatio);
		context.createCameraGpuData(*mSharedDemoResources->PerspectiveSceneCamera);
		mSharedDemoResources->OrthoUiCamera = std::make_unique<DOH::OrthographicCamera>(
			-mSharedDemoResources->AspectRatio,
			mSharedDemoResources->AspectRatio,
			-1.0f,
			1.0f
		);
		context.createCameraGpuData(*mSharedDemoResources->OrthoUiCamera);

		mSharedDemoResources->PerspectiveSceneCameraController = std::make_unique<EDITOR::EditorPerspectiveCameraController>(
			*mSharedDemoResources->PerspectiveSceneCamera,
			mSharedDemoResources->InputLayer,
			aspectRatio
		);
		mSharedDemoResources->OrthoUiCameraController = std::make_unique<EDITOR::EditorOrthoCameraController>(
			*mSharedDemoResources->OrthoUiCamera,
			mSharedDemoResources->InputLayer,
			aspectRatio
		);

		mSharedDemoResources->PerspectiveSceneCameraController->setPositionXYZ(0.0f, 0.0f, 5.0f);
		
		context.addCameraToUpdateList(SharedDemoResources::PERSP_SCENE_CAM_NAME, *mSharedDemoResources->PerspectiveSceneCamera);
		context.addCameraToUpdateList(SharedDemoResources::ORTHO_UI_CAM_NAME, *mSharedDemoResources->OrthoUiCamera);

		mSharedDemoResources->TestTexture1 = context.createTexture(mSharedDemoResources->TestTexturePath);
		mSharedDemoResources->TestTexture2 = context.createTexture(mSharedDemoResources->TestTexture2Path);

		std::vector<std::reference_wrapper<DescriptorSetLayoutVulkan>> texturedDescSetLayouts = {
			context.getCommonDescriptorSetLayouts().Ubo,
			textureSetLayout
		};
		std::shared_ptr<ShaderDescriptorSetLayoutsVulkan> texturedShaderDescSetLayouts = std::make_shared<ShaderDescriptorSetLayoutsVulkan>(texturedDescSetLayouts);
		mSharedDemoResources->TexturedVertexShader = context.createShader(EShaderStage::VERTEX, SharedDemoResources::TexturedShaderVertPath);
		mSharedDemoResources->TexturedFragmentShader = context.createShader(EShaderStage::FRAGMENT, SharedDemoResources::TexturedShaderFragPath);
		mSharedDemoResources->TexturedShaderProgram = context.createShaderProgram(
			mSharedDemoResources->TexturedVertexShader,
			mSharedDemoResources->TexturedFragmentShader,
			texturedShaderDescSetLayouts
		);

		mSharedDemoResources->TexturedPipelineInfo = std::make_unique<GraphicsPipelineInstanceInfo>(
			StaticVertexInputLayout::get(SharedDemoResources::TexturedVertexType),
			*mSharedDemoResources->TexturedShaderProgram,
			ERenderPass::APP_SCENE
		);
		auto& optionalFields = mSharedDemoResources->TexturedPipelineInfo->enableOptionalFields();
		optionalFields.setDepthTesting(true, VK_COMPARE_OP_LESS);

		mSharedDemoResources->TexturedConveyor = context.createPipelineInCurrentRenderState(
			mSharedDemoResources->TexturedPipelineName,
			*mSharedDemoResources->TexturedPipelineInfo
		);

		mSharedDemoResources->TestTexture1DescSet = DescriptorApiVulkan::allocateDescriptorSetFromLayout(
			context.getLogicDevice(),
			context.getCustomDescriptorPool(),
			textureSetLayout
		);
		const uint32_t textureSamplerBinding = 0;
		DescriptorSetUpdate testTexture1Update = {
			{{ textureSetLayout.getDescriptors()[textureSamplerBinding], *mSharedDemoResources->TestTexture1 }},
			mSharedDemoResources->TestTexture1DescSet
		};
		DescriptorApiVulkan::updateDescriptorSet(context.getLogicDevice(), testTexture1Update);
		mSharedDemoResources->TestTexture2DescSet = DescriptorApiVulkan::allocateDescriptorSetFromLayout(
			context.getLogicDevice(),
			context.getCustomDescriptorPool(),
			textureSetLayout
		);
		DescriptorSetUpdate testTexture2Update = {
			{{ textureSetLayout.getDescriptors()[textureSamplerBinding], *mSharedDemoResources->TestTexture2 }},
			mSharedDemoResources->TestTexture2DescSet
		};
		DescriptorApiVulkan::updateDescriptorSet(context.getLogicDevice(), testTexture2Update);

		mSharedDemoResources->GpuResourcesLoaded = true;

		ShapeRenderer::setSceneCameraData(mSharedDemoResources->PerspectiveSceneCameraController->getCamera().getGpuData());
		ShapeRenderer::setUiCameraData(mSharedDemoResources->OrthoUiCameraController->getCamera().getGpuData());
		TextRenderer::setSceneCameraData(mSharedDemoResources->PerspectiveSceneCameraController->getCamera().getGpuData());
		TextRenderer::setUiCameraData(mSharedDemoResources->OrthoUiCameraController->getCamera().getGpuData());
		LineRenderer::setSceneCameraData(mSharedDemoResources->PerspectiveSceneCameraController->getCamera().getGpuData());
		LineRenderer::setUiCameraData(mSharedDemoResources->OrthoUiCameraController->getCamera().getGpuData());
	}

	void DemoLiciousAppLogic::closeSharedResources() {
		ZoneScoped;

		if (mCustomDemo != nullptr || mObjModelsDemo != nullptr) {
			LOG_WARN("All demos using shared resources must be closed before closing shared resources");
		} else if (mSharedDemoResources->GpuResourcesLoaded) {
			RendererVulkan& renderer = GET_RENDERER;
			renderer.closeGpuResource(mSharedDemoResources->TexturedVertexShader);
			renderer.closeGpuResource(mSharedDemoResources->TexturedFragmentShader);
			renderer.getContext().closePipeline(
				mSharedDemoResources->TexturedPipelineInfo->getRenderPass(),
				SharedDemoResources::TexturedPipelineName
			);
			renderer.closeGpuResource(mSharedDemoResources->TestTexture1);
			renderer.closeGpuResource(mSharedDemoResources->TestTexture2);

			renderer.closeGpuResourceOwner(mSharedDemoResources->PerspectiveSceneCameraController->getCamera().getGpuData());
			renderer.closeGpuResourceOwner(mSharedDemoResources->OrthoUiCameraController->getCamera().getGpuData());

			mSharedDemoResources->GpuResourcesLoaded = false;
		}
	}

	void DemoLiciousAppLogic::updateCamerasAspectRatio(float aspectRatio) {
		mSharedDemoResources->PerspectiveSceneCameraController->onViewportResize(aspectRatio);
		mSharedDemoResources->OrthoUiCameraController->onViewportResize(aspectRatio);

		if (mMultiCameraPassDemo != nullptr) {
			mMultiCameraPassDemo->OrthoSceneCameraController->onViewportResize(aspectRatio);
			mMultiCameraPassDemo->PerspectiveUiCameraController->onViewportResize(aspectRatio);
		}
	}

	void DemoLiciousAppLogic::ObjModelsDemo::init() {
		ZoneScoped;

		RenderingContextVulkan& context = GET_RENDERER.getContext();

		for (const auto& filePath : ObjModelFilePaths) {
			LoadedModels.emplace_back(ModelVulkan::createModel(filePath, ColouredVertexInputLayout));
		}

		std::initializer_list<VkDescriptorSet> sceneDescSetsInstance = { VK_NULL_HANDLE };
		SceneDescSetsInstance = std::make_shared<DescriptorSetsInstanceVulkan>(sceneDescSetsInstance);
	
		//Spwan objects on incrementing x/y values of a grid with random z value
		constexpr float padding = 0.5f;
		for (uint32_t x = 0; x < 10; x++) {
			for (uint32_t y = 0; y < 10; y++) {
				//Add an object with a position based off of x & y value from loop, creates a grid like result
				const uint32_t modelIndex = rand() % LoadedModels.size();
				std::shared_ptr<TransformationData> transform = std::make_shared<TransformationData>(
					glm::vec3(
						(static_cast<float>(x) + padding) * 3.0f,
						(static_cast<float>(y) + padding) * 3.0f,
						static_cast<float>(rand() % 10)
					),
					glm::vec3(
						static_cast<float>(rand() % 360),
						static_cast<float>(rand() % 360),
						static_cast<float>(rand() % 360)
					),
					1.0f
				);
				transform->updateTranslationMatrix();
				
				RenderableObjects.emplace_back(std::make_shared<RenderableModelVulkan>(
					ObjModelFilePaths[modelIndex],
					LoadedModels[modelIndex],
					transform,
					SceneDescSetsInstance
				));
			}
		}

		//for (uint32_t i = 0; i < 100; i++) {
		//	objModelsDemoAddRandomisedObject();
		//}
	
		std::vector<std::reference_wrapper<DescriptorSetLayoutVulkan>> sceneDescSets = { context.getCommonDescriptorSetLayouts().Ubo };
		VkPushConstantRange pushConstant = context.pushConstantInfo(VK_SHADER_STAGE_VERTEX_BIT, sizeof(glm::mat4x4), 0);
		std::vector<VkPushConstantRange> pushConstants = { pushConstant };
		std::shared_ptr<ShaderDescriptorSetLayoutsVulkan> sceneDescSetLayouts = std::make_shared<ShaderDescriptorSetLayoutsVulkan>(pushConstants, sceneDescSets);
		SceneVertexShader = context.createShader(EShaderStage::VERTEX, FlatColourShaderVertPath);
		SceneFragmentShader = context.createShader(EShaderStage::FRAGMENT, FlatColourShaderFragPath);
		SceneShaderProgram = context.createShaderProgram(
			SceneVertexShader,
			SceneFragmentShader,
			sceneDescSetLayouts
		);
	
		ScenePipelineInfo = std::make_unique<GraphicsPipelineInstanceInfo>(
			ColouredVertexInputLayout,
			*SceneShaderProgram,
			ERenderPass::APP_SCENE
		);
		auto& sceneOptionalFields = ScenePipelineInfo->enableOptionalFields();
		sceneOptionalFields.setDepthTesting(true, VK_COMPARE_OP_LESS);

		//TODO:: wireframe for each vertex type
		SceneWireframePipelineInfo = std::make_unique<GraphicsPipelineInstanceInfo>(
			ColouredVertexInputLayout,
			*SceneShaderProgram,
			ERenderPass::APP_SCENE
		);
		auto& wireframeOptionalFields = SceneWireframePipelineInfo->enableOptionalFields();
		wireframeOptionalFields.setDepthTesting(true, VK_COMPARE_OP_LESS);
		wireframeOptionalFields.CullMode = VK_CULL_MODE_NONE;
		wireframeOptionalFields.PolygonMode = VK_POLYGON_MODE_LINE;
	
		ScenePipelineConveyor = context.createPipelineInCurrentRenderState(ScenePipelineName, *ScenePipelineInfo);
		WireframePipelineConveyor = context.createPipelineInCurrentRenderState(SceneWireframePipelineName, *SceneWireframePipelineInfo);

		TexturedModel = ModelVulkan::createModel("Dough/Dough/res/models/textured_cube.obj", TexturedVertexInputLayout);
		std::initializer_list<VkDescriptorSet> descSets = { VK_NULL_HANDLE, SharedResources.TestTexture1DescSet };
		TexturedModelDescriptorSets = std::make_shared<DescriptorSetsInstanceVulkan>(descSets);
		RenderableTexturedModel = std::make_shared<RenderableModelVulkan>("TexturedObjModel", TexturedModel, TexturedModelDescriptorSets);

		GpuResourcesLoaded = true;
	}

	void DemoLiciousAppLogic::ObjModelsDemo::close() {
		ZoneScoped;

		RendererVulkan& renderer = GET_RENDERER;
		for (const auto& model : LoadedModels) {
			renderer.closeGpuResource(model);
		}

		renderer.closeGpuResource(TexturedModel);
		LoadedModels.clear();
		RenderableObjects.clear();
		renderer.closeGpuResource(SceneVertexShader);
		renderer.closeGpuResource(SceneFragmentShader);

		auto& context = renderer.getContext();
		context.closePipeline(ScenePipelineInfo->getRenderPass(), ScenePipelineName);
		context.closePipeline(SceneWireframePipelineInfo->getRenderPass(), SceneWireframePipelineName);

		GpuResourcesLoaded = false;
	}

	void DemoLiciousAppLogic::ObjModelsDemo::update(float delta) {
		//NOTE:: Obj models aren't updated per Update cycle, done during ImGui render stage, as currently only the ImGui
		// UI has control over the transform data. And recalculating x number of object's transformation is a lot of wasted
		// work.
		if (Update) {
			for (const auto& obj : RenderableObjects) {
				obj->Transformation->updateTranslationMatrix();
			}
		}
	}

	void DemoLiciousAppLogic::ObjModelsDemo::render() {
		constexpr uint32_t uboSlot = 0;

		if (GpuResourcesLoaded && Render) {
			if (ScenePipelineConveyor.isValid() && WireframePipelineConveyor.isValid()) {
				const bool renderAllStandard = RenderAllStandard;
				const bool renderAllWireframe = RenderAllWireframe;
		
				for (auto& obj : RenderableObjects) {
					obj->getDescriptorSetsInstance()->getDescriptorSets()[uboSlot] =
						SharedResources.PerspectiveSceneCameraController->getCamera().getGpuData()->DescriptorSets[GET_RENDERER.getContext().getCurrentFrame()];
					if (renderAllStandard || obj->Render) {
						ScenePipelineConveyor.addRenderable(obj);
					}
					if (renderAllWireframe || obj->RenderWireframe) {
						WireframePipelineConveyor.addRenderable(obj);
					}
				}
			}
		
			if (SharedResources.TexturedConveyor.isValid() && RenderableTexturedModel->Render) {
				RenderableTexturedModel->getDescriptorSetsInstance()->getDescriptorSets()[uboSlot] =
					SharedResources.PerspectiveSceneCameraController->getCamera().getGpuData()->DescriptorSets[GET_RENDERER.getContext().getCurrentFrame()];
				SharedResources.TexturedConveyor.addRenderable(RenderableTexturedModel);
			}
		}
	}

	void DemoLiciousAppLogic::ObjModelsDemo::renderImGuiMainTab() {
		ZoneScoped;

		static const std::string addLabel = std::string(ImGuiWrapper::EMPTY_LABEL) + "Add";
		static const std::string popLabel = std::string(ImGuiWrapper::EMPTY_LABEL) + "Pop";

		//TODO:: Pipeline uniform objects are NOT re-created during runtime causing a crash when trying to access them (e.g. camera UBO's)
		//if (ImGui::Button("Load") && !mObjModelsDemo->GpuResourcesLoaded) {
		//	mObjModelsDemo->init();
		//}
		//ImGui::SameLine();
		if (ImGui::Button("Unload") && GpuResourcesLoaded) {
			close();
		}
		EditorGui::displayHelpTooltip("TEMP:: Currently only unloading certain GPU resources during runtime is supported, this is ONLY a demonstration and once unloaded this demo can only be loaded again by restarting.");
		ImGui::TextColored(GpuResourcesLoaded ? ImVec4(0, 1, 0, 1) : ImVec4(1, 0, 0, 1), GpuResourcesLoaded ? "LOADED" : "NOT LOADED");

		ImGui::Checkbox("Render", &Render);
		ImGui::Checkbox("Update", &Update);
		ImGui::Text("Object Count: %i", RenderableObjects.size());
		if (ImGui::InputInt(addLabel.c_str(), &AddNewObjectsCount, 5, 5)) {
			if (AddNewObjectsCount < 0) {
				AddNewObjectsCount = 0;
			} else if (AddNewObjectsCount > 1000) {
				AddNewObjectsCount = 1000;
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("Add Object")) {
			for (int i = 0; i < AddNewObjectsCount; i++) {
				addRandomisedObject();
			}
		}
		if (ImGui::InputInt(popLabel.c_str(), &PopObjectsCount, 5, 5)) {
			if (PopObjectsCount < 0) {
				PopObjectsCount = 0;
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("Pop Object")) {
			//Max out pop count to renderables list size
			const int size = static_cast<int>(RenderableObjects.size());
			const int popCount = PopObjectsCount > size ? size : PopObjectsCount;

			for (int i = 0; i < popCount; i++) {
				RenderableObjects.pop_back();
			}
		}
		ImGui::Checkbox("Display Renderable Models List", &RenderObjModelsList);
		if (ImGui::Button("Clear Objects")) {
			RenderableObjects.clear();
		}

		ImGui::Checkbox("Render Textured Model", &RenderableTexturedModel->Render);
		//TODO:: TexturedModel Wireframe rendering not currently supported
		//ImGui::Checkbox("Render Wireframe Textured Model", &mObjModelsDemo->RenderableTexturedModel->RenderWireframe);
	}

	void DemoLiciousAppLogic::ObjModelsDemo::renderImGuiExtras() {
		ZoneScoped;

		if (RenderObjModelsList) {
			if (ImGui::Begin("Renderable Models List", &RenderObjModelsList)) {
				ImGui::Checkbox("Render All", &RenderAllStandard);
				ImGui::SameLine();
				ImGui::Checkbox("Render All Wireframe", &RenderAllWireframe);

				//Render list of renderable OBJ models as a tree separating all into groups of 10 based on index
				const int size = static_cast<int>(RenderableObjects.size());
				if (size > 0) {
					ImGui::TreePush();
					ImGui::Unindent();

					int objIndex = 0;
					constexpr int objectsPerNode = 10;
					for (int nodeIterator = objIndex; nodeIterator < size; nodeIterator += objectsPerNode) {
						if (ImGui::TreeNode(std::to_string(objIndex).c_str())) {
							for (int i = objIndex; i < nodeIterator + objectsPerNode && i < size; i++) {
								std::string uniqueImGuiId = "##" + std::to_string(i);
								imGuiDrawObjDemoItem(*RenderableObjects[i], uniqueImGuiId);

								//Separate individual OBJ model's UI for easier viewing by displaying an empty
								// line in-between each one in the same node
								if (
									i < nodeIterator + objectsPerNode - 1 &&
									i != size - 1 //Prevent NewLine after final UI item that is part-way through a node's list
								) {
									ImGui::NewLine();
								}
							}

							ImGui::TreePop();
						}
						objIndex += objectsPerNode;
					}

					ImGui::TreePop();
				} else {
					ImGui::TextWrapped("No OBJ renderables exist, use the demo's editor to create some.");
				}
			}

			ImGui::End();
		}
	}

	void DemoLiciousAppLogic::ObjModelsDemo::addObject(
		const uint32_t modelIndex,
		const float x,
		const float y,
		const float z,
		const float posPadding,
		const float yaw,
		const float pitch,
		const float roll,
		const float scale
	) {
		ZoneScoped;

		std::shared_ptr<TransformationData> transform = std::make_shared<TransformationData>(
			glm::vec3((x + posPadding) * 3.0f, (y + posPadding) * 3.0f, (z + posPadding) * 3.0f),
			glm::vec3(yaw, pitch, roll),
			scale
		);
		transform->updateTranslationMatrix();
	
		RenderableObjects.emplace_back(std::make_shared<RenderableModelVulkan>(
			ObjModelFilePaths[modelIndex],
			LoadedModels[modelIndex],
			transform,
			SceneDescSetsInstance
		));
	}

	void DemoLiciousAppLogic::ObjModelsDemo::imGuiDrawObjDemoItem(DOH::RenderableModelVulkan& model, const std::string& uniqueImGuiId) {
		ZoneScoped;

		if (ImGui::BeginCombo(("Obj Model" + uniqueImGuiId).c_str(), model.getName().c_str())) {
			int modelFilePathIndex = -1;

			for (const auto& filePath : ObjModelFilePaths) {
				bool selected = false;
				modelFilePathIndex++;

				std::string label = filePath + uniqueImGuiId;

				if (ImGui::Selectable(label.c_str(), &selected)) {
					model.Model = LoadedModels[modelFilePathIndex];
					model.Name = ObjModelFilePaths[modelFilePathIndex];
					break;
				}
			}

			ImGui::EndCombo();
		}

		ImGui::Checkbox(("Render" + uniqueImGuiId).c_str(), &model.Render);
		ImGui::SameLine();
		ImGui::Checkbox(("Wireframe" + uniqueImGuiId).c_str(), &model.RenderWireframe);

		//Add temp array and set -1 > rotation < 361 to allow for "infinite" drag
		TransformationData& transformation = *model.Transformation;
		float tempRotation[3] = {
			transformation.Rotation[0],
			transformation.Rotation[1],
			transformation.Rotation[2]
		};
		bool transformed = false;
		if (ImGui::DragFloat3(
			("Position" + uniqueImGuiId).c_str(),
			glm::value_ptr(transformation.Position),
			0.05f,
			-10.0f,
			10.0f
		)) {
			transformed = true;
		}
		if (ImGui::DragFloat3(("Rotation" + uniqueImGuiId).c_str(), tempRotation, 1.0f, -1.0f, 361.0f)) {
			if (tempRotation[0] > 360.0f) {
				tempRotation[0] = 0.0f;
			} else if (tempRotation[0] < 0.0f) {
				tempRotation[0] = 360.0f;
			}
			if (tempRotation[1] > 360.0f) {
				tempRotation[1] = 0.0f;
			} else if (tempRotation[1] < 0.0f) {
				tempRotation[1] = 360.0f;
			}
			if (tempRotation[2] > 360.0f) {
				tempRotation[2] = 0.0f;
			} else if (tempRotation[2] < 0.0f) {
				tempRotation[2] = 360.0f;
			}

			transformation.Rotation = { tempRotation[0], tempRotation[1], tempRotation[2] };

			transformed = true;
		}
		if (ImGui::DragFloat(
			("Uniform Scale" + uniqueImGuiId).c_str(),
			&transformation.Scale,
			0.01f,
			0.1f,
			5.0f
		)) {
			transformed = true;
		}

		if (ImGui::Button(("Reset Object" + uniqueImGuiId).c_str())) {
			transformation.Rotation[0] = 0.0f; transformation.Rotation[1] = 0.0f; transformation.Rotation[2] = 0.0f;
			transformation.Position[0] = 0.0f; transformation.Position[1] = 0.0f; transformation.Position[2] = 0.0f;
			transformation.Scale = 1.0f;

			transformed = true;
		}

		if (transformed && Update) {
			transformation.updateTranslationMatrix();
		}
	}

	void DemoLiciousAppLogic::CustomDemo::init() {
		ZoneScoped;

		//for (int i = 0; i < 8; i++) {
		//	std::string path = testTexturesPath + "texture" + std::to_string(i) + ".png";
		//	std::shared_ptr<TextureVulkan> testTexture = ObjInit::texture(path);
		//	mTestTextures.push_back(testTexture);
		//}

		RenderingContextVulkan& context = Application::get().getRenderer().getContext();

		SceneVao = context.createVertexArray();
		std::shared_ptr<VertexBufferVulkan> sceneVb = context.createStagedVertexBuffer(
			SceneVertexInputLayout,
			SceneVertices.data(),
			static_cast<size_t>(SceneVertexInputLayout.getStride()) * SceneVertices.size(),
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		);
		SceneVao->addVertexBuffer(sceneVb);
		std::shared_ptr<IndexBufferVulkan> sceneIb = context.createStagedIndexBuffer(
			Indices.data(),
			sizeof(uint32_t) * Indices.size()
		);
		SceneVao->setDrawCount(static_cast<uint32_t>(Indices.size()));
		SceneVao->setIndexBuffer(sceneIb);
		std::initializer_list<VkDescriptorSet> sceneDescSetsInstance = { VK_NULL_HANDLE, SharedResources.TestTexture1DescSet };
		SceneDescSetsInstance = std::make_shared<DescriptorSetsInstanceVulkan>(sceneDescSetsInstance);
		SceneRenderable = std::make_shared<SimpleRenderable>(SceneVao, SceneDescSetsInstance);
	
		DescriptorSetLayoutVulkan& uboLayout = context.getCommonDescriptorSetLayouts().Ubo;
		std::vector<std::reference_wrapper<DescriptorSetLayoutVulkan>> uiDescSetLayouts = { uboLayout };
		std::shared_ptr<ShaderDescriptorSetLayoutsVulkan> uiShaderDescLayout = std::make_shared<ShaderDescriptorSetLayoutsVulkan>(uiDescSetLayouts);
		UiVertexShader = context.createShader(EShaderStage::VERTEX, UiShaderVertPath);
		UiFragmentShader = context.createShader(EShaderStage::FRAGMENT, UiShaderFragPath);
		UiShaderProgram = context.createShaderProgram(
			UiVertexShader,
			UiFragmentShader,
			uiShaderDescLayout
		);
	
		UiVao = context.createVertexArray();
		std::shared_ptr<VertexBufferVulkan> appUiVb = context.createStagedVertexBuffer(
			UiVertexInputLayout,
			UiVertices.data(),
			static_cast<size_t>(UiVertexInputLayout.getStride()) * UiVertices.size(),
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		);
		UiVao->addVertexBuffer(appUiVb);
		std::shared_ptr<IndexBufferVulkan> appUiIb = context.createStagedIndexBuffer(
			UiIndices.data(),
			sizeof(UiIndices[0]) * UiIndices.size()
		);
		UiVao->setDrawCount(static_cast<uint32_t>(UiIndices.size()));
		UiVao->setIndexBuffer(appUiIb);
		std::initializer_list<VkDescriptorSet> uiDescSets = { VK_NULL_HANDLE };
		std::shared_ptr<DescriptorSetsInstanceVulkan> uiDescSetsInstance = std::make_shared<DescriptorSetsInstanceVulkan>(uiDescSets);
		UiRenderable = std::make_shared<SimpleRenderable>(UiVao, uiDescSetsInstance);
	
		UiPipelineInfo = std::make_unique<GraphicsPipelineInstanceInfo>(
			UiVertexInputLayout,
			*UiShaderProgram,
			ERenderPass::APP_UI
		);
		auto& uiOptionalFields = UiPipelineInfo->enableOptionalFields();
	
		CustomUiConveyor = context.createPipelineInCurrentRenderState(UiPipelineName, *UiPipelineInfo);

		GpuResourcesLoaded = true;
	}

	void DemoLiciousAppLogic::CustomDemo::close() {
		ZoneScoped;

		RenderScene = false;
		RenderUi = false;
		Update = false;

		RendererVulkan& renderer = GET_RENDERER;

		renderer.closeGpuResource(SceneVao);
		renderer.closeGpuResource(UiVertexShader);
		renderer.closeGpuResource(UiFragmentShader);
		renderer.closeGpuResource(UiVao);
		renderer.getContext().closePipeline(UiPipelineInfo->getRenderPass(), UiPipelineName);

		GpuResourcesLoaded = false;
	}

	void DemoLiciousAppLogic::CustomDemo::update(float delta) {

	}

	void DemoLiciousAppLogic::CustomDemo::render() {
		const uint32_t uboSlot = 0;
		if (RenderScene) {
			SceneRenderable->getDescriptorSetsInstance()->getDescriptorSets()[uboSlot] = SharedResources.PerspectiveSceneCameraController->getCamera().getGpuData()->DescriptorSets[GET_RENDERER.getContext().getCurrentFrame()];
			SharedResources.TexturedConveyor.safeAddRenderable(SceneRenderable);
		}
		if (RenderUi) {
			UiRenderable->getDescriptorSetsInstance()->getDescriptorSets()[uboSlot] = SharedResources.OrthoUiCameraController->getCamera().getGpuData()->DescriptorSets[GET_RENDERER.getContext().getCurrentFrame()];
			CustomUiConveyor.safeAddRenderable(UiRenderable);
		}
	}

	void DemoLiciousAppLogic::CustomDemo::renderImGuiMainTab() {
		ZoneScoped;

		ImGui::Checkbox("Render Scene", &RenderScene);
		ImGui::Checkbox("Render UI", &RenderUi);
		ImGui::Checkbox("Update", &Update);
	}

	void DemoLiciousAppLogic::CustomDemo::renderImGuiExtras() {
		//No extra windows required for this demo
	}

	void DemoLiciousAppLogic::ShapesDemo::GridDemo::init() {
		ZoneScoped;

		//NOTE:: Arbitrary limit that doesn't take into account whether any other demo is rendering quads.
		TestGridMaxQuadCount = EBatchSizeLimits::QUAD_MAX_BATCH_COUNT * EBatchSizeLimits::QUAD_BATCH_MAX_GEO_COUNT;
	}

	void DemoLiciousAppLogic::ShapesDemo::GridDemo::close() {
		ZoneScoped;

		TexturedTestGrid.clear();
	}

	void DemoLiciousAppLogic::ShapesDemo::GridDemo::update(float delta) {
		if (Update && !IsUpToDate) {
			//Repopulate entire array each update, not very efficient but the test grid is just an example
			populateTestGrid(static_cast<uint32_t>(TestGridSize[0]), static_cast<uint32_t>(TestGridSize[1]));
		}
	}

	void DemoLiciousAppLogic::ShapesDemo::GridDemo::render() {
		if (Render) {
			if (QuadDrawColour) {
				for (const std::vector<Quad>& sameTexturedQuads : TexturedTestGrid) {
					ShapeRenderer::drawQuadArrayScene(sameTexturedQuads);
				}
			} else {
				for (const std::vector<Quad>& sameTexturedQuads : TexturedTestGrid) {
					ShapeRenderer::drawQuadArraySameTextureScene(sameTexturedQuads);
				}
			}
		}
	}

	void DemoLiciousAppLogic::ShapesDemo::GridDemo::renderImGuiMainTab() {
		ZoneScoped;

		ImGui::Checkbox("Render", &Render);
		ImGui::Checkbox("Update", &Update);
		ImGui::Checkbox("Draw Colour", &QuadDrawColour);
		ImGui::Text("Grid Quad Count: %i of Max %i", TestGridSize[0] * TestGridSize[1], TestGridMaxQuadCount);
		int tempTestGridSize[2] = { TestGridSize[0], TestGridSize[1] };
		if (ImGui::InputInt2("Grid Size", tempTestGridSize)) {
			if (tempTestGridSize[0] > 0 && tempTestGridSize[1] > 0) {
				const int tempGridQuadCount = tempTestGridSize[0] * tempTestGridSize[1];
				if (tempGridQuadCount <= TestGridMaxQuadCount) {
					TestGridSize[0] = tempTestGridSize[0];
					TestGridSize[1] = tempTestGridSize[1];
					IsUpToDate = false;
				} else {
					LOG_WARN(
						"New grid size of " << tempTestGridSize[0] << "x" << tempTestGridSize[1] <<
						" (" << tempTestGridSize[0] * tempTestGridSize[1] <<
						") is too large, max quad count is " << TestGridMaxQuadCount
					);
				}
			}
		}
		if (ImGui::DragFloat2("Quad Size", TestGridQuadSize, 0.001f, 0.01f, 0.5f)) {
			IsUpToDate = false;
		}
		if (ImGui::DragFloat2("Quad Gap Size", TestGridQuadGapSize, 0.001f, 0.01f, 0.5f)) {
			IsUpToDate = false;
		}
		//ImGui::DragFloat2("Test Grid Origin Pos", );
		//ImGui::Text("UI Quad Count: %i", renderer.getContext().getRenderer2d().getStorage().getUiQuadCount());

		//TOOD:: maybe have radio buttons for RenderStaticGrid or RenderDynamicGrid,
		//	static being the default values and dynamic being from the variables determined by ths menu
		// Maybe have the dynamic settings hidden unless dynamic is selected

		MonoSpaceTextureAtlas& atlas = *ShapeRenderer::getTestMonoSpaceTextureAtlas();

		int tempTestTextureRowOffset = TestTexturesRowOffset;
		if (ImGui::InputInt("Test Texture Row Offset", &tempTestTextureRowOffset)) {
			TestTexturesRowOffset = tempTestTextureRowOffset < 0 ?
				0 : tempTestTextureRowOffset % atlas.getRowCount();
			IsUpToDate = false;
		}
		int tempTestTextureColOffset = TestTexturesColumnOffset;
		if (ImGui::InputInt("Test Texture Col Offset", &tempTestTextureColOffset)) {
			TestTexturesColumnOffset = tempTestTextureColOffset < 0 ?
				0 : tempTestTextureColOffset % atlas.getColCount();
			IsUpToDate = false;
		}

		if (ImGui::ColorEdit4("Grid Colour", &QuadColour.x)) {
			IsUpToDate = false;
		}

		if (ImGui::Button("Reset Grid")) {
			TestGridSize[0] = 10;
			TestGridSize[1] = 10;
			TestGridQuadSize[0] = 0.1f;
			TestGridQuadSize[1] = 0.1f;
			TestGridQuadGapSize[0] = TestGridQuadSize[0] * 1.5f;
			TestGridQuadGapSize[1] = TestGridQuadSize[1] * 1.5f;
			IsUpToDate = false;
		}
	}

	void DemoLiciousAppLogic::ShapesDemo::GridDemo::renderImGuiExtras() {
		//No extra windows required for this demo
	}

	void DemoLiciousAppLogic::ShapesDemo::BouncingQuadDemo::init() {
		ZoneScoped;

		addRandomQuads(5000);
	}

	void DemoLiciousAppLogic::ShapesDemo::BouncingQuadDemo::close() {
		ZoneScoped;

		BouncingQuads.clear();
		BouncingQuadVelocities.clear();
	}

	void DemoLiciousAppLogic::ShapesDemo::BouncingQuadDemo::update(float delta) {
		if (Update) {
			const float translationDelta = 0.2f * delta;
			for (size_t i = 0; i < BouncingQuads.size(); i++) {
				Quad& quad = BouncingQuads[i];
				glm::vec2& velocity = BouncingQuadVelocities[i];
		
				if (quad.Position.x + quad.Size.x >= 20.0f || quad.Position.x <= -20.0f) {
					velocity.x = -velocity.x;
				}
		
				if (quad.Position.y + quad.Size.y >= 20.0f || quad.Position.y <= -20.0f) {
					velocity.y = -velocity.y;
				}
		
				quad.Position.x += velocity.x * translationDelta;
				quad.Position.y += velocity.y * translationDelta;
			}
		}
	}

	void DemoLiciousAppLogic::ShapesDemo::BouncingQuadDemo::render() {
		if (Render) {
			if (QuadDrawColour) {
				ShapeRenderer::drawQuadArrayScene(BouncingQuads);
			} else {
				ShapeRenderer::drawQuadArraySameTextureScene(BouncingQuads);
			}
		}

	}

	void DemoLiciousAppLogic::ShapesDemo::BouncingQuadDemo::renderImGuiMainTab() {
		ZoneScoped;

		ImGui::Checkbox("Render", &Render);
		ImGui::Checkbox("Update", &Update);
		ImGui::Checkbox("Draw Colour", &QuadDrawColour);
		ImGui::Text("Bouncing Quads Count: %i", BouncingQuads.size());

		if (ImGui::InputInt((std::string(ImGuiWrapper::EMPTY_LABEL) + "Add").c_str(), &AddNewQuadCount, 5, 5)) {
			if (AddNewQuadCount < 0) {
				AddNewQuadCount = 0;
			} else if (AddNewQuadCount > 5000) {
				AddNewQuadCount = 5000;
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("Add Quads")) {
			addRandomQuads(AddNewQuadCount);
		}
		if (ImGui::InputInt((std::string(ImGuiWrapper::EMPTY_LABEL) + "Pop").c_str(), &PopQuadCount, 5, 5)) {
			if (PopQuadCount < 0) {
				PopQuadCount = 0;
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("Pop Quads")) {
			popQuads(PopQuadCount);
		}
		if (ImGui::Button("Clear Quads")) {
			BouncingQuads.clear();
		}
	}

	void DemoLiciousAppLogic::ShapesDemo::BouncingQuadDemo::renderImGuiExtras() {
		//No extra windows required for this demo
	}

	void DemoLiciousAppLogic::ShapesDemo::BouncingQuadDemo::addRandomQuads(size_t count) {
		ZoneScoped;

		const auto& atlas = ShapeRenderer::getTestMonoSpaceTextureAtlas();
	
		//Stop quad count going over MaxCount
		if (count + BouncingQuads.size() > MaxBouncingQuadCount) {
			count = MaxBouncingQuadCount - BouncingQuads.size();
		}
	
		for (int i = 0; i < count; i++) {
			//Add quads using the texture atlas
			BouncingQuads.push_back({
				//Semi-random values for starting position, velocitiy and assigned texture
				{
					(static_cast<float>((rand() % 5000)) / 500.0f) - 1.0f,
					(static_cast<float>((rand() % 5000)) / 500.0f) - 1.0f,
					0.8f
				},
				{ QuadSize.x, QuadSize.y },
				{
					//TODO:: Easier & cleaner way of getting a random normalised RGB value
					(1.0f / 255.0f) * static_cast<float>(rand() % static_cast<int>(TextureVulkan::COLOUR_MAX_VALUE)),
					(1.0f / 255.0f) * static_cast<float>(rand() % static_cast<int>(TextureVulkan::COLOUR_MAX_VALUE)),
					(1.0f / 255.0f) * static_cast<float>(rand() % static_cast<int>(TextureVulkan::COLOUR_MAX_VALUE)),
					1.0f
				},
				0.0f,
				//Texture atlas
				atlas,

				//Single inner texture
				atlas->getInnerTextureCoords(
					rand() % atlas->getRowCount(),
					rand() % atlas->getColCount()
				)
				//atlas->getInnerTextureCoords(4, 4),

				//Display mutliple inner rows/columns (rand() % may result in two 0 values passed which returns all 0 coords)
				//atlas->getInnerTextureCoords(
				//	rand() % atlas->getRowCount(),
				//	rand() % atlas->getRowCount(),
				//	rand() % atlas->getColCount(),
				//	rand() % atlas->getColCount()
				//)
				//Display whole texture atlas
				//atlas->getInnerTextureCoords(
				//	0,
				//	atlas->getRowCount(),
				//	0,
				//	atlas->getColCount()
				//)
			});
	
			BouncingQuadVelocities.emplace_back(
				static_cast<float>((rand() % 800) / 60.0f),
				static_cast<float>((rand() % 800) / 60.0f)
			);
		}
	}

	void DemoLiciousAppLogic::ShapesDemo::BouncingQuadDemo::popQuads(size_t count) {
		ZoneScoped;

		const size_t size = BouncingQuads.size();
		if (count > size) {
			count = size;
		}

		for (int i = 0; i < count; i++) {
			BouncingQuads.pop_back();
			BouncingQuadVelocities.pop_back();
		}
	}

	void DemoLiciousAppLogic::ShapesDemo::CircleDemo::init() {
		ZoneScoped;

		addRandomCirclesScene(5000);

		TestCircleScene = {};
		TestCircleScene.Position = { 0.5f, 0.5f, 0.1f };
		TestCircleScene.Size = { 0.5f, 0.5f };
		TestCircleScene.Colour = { 0.0f, 0.0f, 0.0f, 1.0f };

		TestCircleUi = {};
		TestCircleUi.Position = { 0.5f, 0.5f, 0.1f };
		TestCircleUi.Size = { 0.5f, 0.5f };
		TestCircleUi.Colour = { 0.0f, 0.0f, 0.0f, 1.0f };
	}

	void DemoLiciousAppLogic::ShapesDemo::CircleDemo::close() {
		ZoneScoped;

		CirclesScene.clear();
	}

	void DemoLiciousAppLogic::ShapesDemo::CircleDemo::update(float delta) {

	}

	void DemoLiciousAppLogic::ShapesDemo::CircleDemo::render() {
		if (Render) {
			if (DrawColour) {
				//Render circles 1 at a time, much slower than passing as an array
				//for (const Circle& circle : CirclesScene) {
				//	ShapeRenderer::drawCircleScene(circle);
				//}
		
				ShapeRenderer::drawCircleArrayScene(CirclesScene);
			} else {
				//Render circles 1 at a time, much slower than passing as an array
				//for (const Circle& circle : CirclesScene) {
				//	ShapeRenderer::drawCircleTexturedScene(circle);
				//}
		
				//ShapeRenderer::drawCircleArrayTexturedScene(CirclesScene);
				ShapeRenderer::drawCircleArraySameTextureScene(CirclesScene);
			}
		
			if (RenderTestCircles) {
				ShapeRenderer::drawCircleScene(TestCircleScene);
				ShapeRenderer::drawCircleUi(TestCircleUi);
			}
		}
	}

	void DemoLiciousAppLogic::ShapesDemo::CircleDemo::renderImGuiMainTab() {
		ZoneScoped;

		ImGui::Checkbox("Render", &Render);
		ImGui::Checkbox("Update", &Update);
		ImGui::Checkbox("Draw Colour", &DrawColour);
		ImGui::Checkbox("RenderTestCircles", &RenderTestCircles);
		ImGui::Text("Count Scene: %i", CirclesScene.size());
		//ImGui::Text("Count UI: %i", CirclesUi.size());

		if (ImGui::InputInt((std::string(ImGuiWrapper::EMPTY_LABEL) + "Add").c_str(), &AddNewCount, 5, 5)) {
			if (AddNewCount < 0) {
				AddNewCount = 0;
			} else if (AddNewCount > 5000) {
				AddNewCount = 5000;
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("Add Scene")) {
			addRandomCirclesScene(AddNewCount);
		}
		if (ImGui::InputInt((std::string(ImGuiWrapper::EMPTY_LABEL) + "Pop").c_str(), &PopCount, 5, 5)) {
			if (PopCount < 0) {
				PopCount = 0;
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("Pop Scene")) {
			popScene(PopCount);
		}
		if (ImGui::Button("Clear Scene")) {
			CirclesScene.clear();
		}

		ImGui::Text("TestCircleScene");
		EditorGui::controlsCircle(TestCircleScene, "TestCircleScene");
		ImGui::Text("TestCircleUi");
		EditorGui::controlsCircle(TestCircleUi, "TestCircleUi");

		ImGui::NewLine();
		{
			ImGui::Text("Scene");
			int i = 0;
			const auto& batches = ShapeRenderer::getCircleSceneBatches();
			for (const auto& batch : batches) {
				ImGui::Text("Batch: %i Count: %i", i, static_cast<int>(batch->getGeometryCount()));
				i++;
			}
			if (i == 0) {
				ImGui::Text("No Batches Used");
			}
		}

		{
			ImGui::Text("UI");
			int i = 0;
			const auto& batches = ShapeRenderer::getCircleUiBatches();
			for (const auto& batch : batches) {
				ImGui::Text("Batch: %i Count: %i", i, static_cast<int>(batch->getGeometryCount()));
				i++;
			}
			if (i == 0) {
				ImGui::Text("No Batches Used");
			}
		}
	}

	void DemoLiciousAppLogic::ShapesDemo::CircleDemo::renderImGuiExtras() {
		
	}

	void DemoLiciousAppLogic::ShapesDemo::CircleDemo::addRandomCirclesScene(size_t count) {
		ZoneScoped;

		const auto& atlas = ShapeRenderer::getTestMonoSpaceTextureAtlas();

		//Stop quad count going over MaxCount
		if (count + CirclesScene.size() > MaxCirclesCount) {
			count = MaxCirclesCount - CirclesScene.size();
		}
		CirclesScene.reserve(CirclesScene.size() + count);

		for (int i = 0; i < count; i++) {
			Circle circle = {};
			circle.Position = {
				(static_cast<float>((rand() % 5000)) / 500.0f) - 1.0f,
				(static_cast<float>((rand() % 5000)) / 500.0f) - 1.0f,
				0.8f
			};
			circle.Size = { 0.1f, 0.1f };
			circle.Colour = {
				//TODO:: Easier & cleaner way of getting a random normalised RGB value
				(1.0f / 255.0f) * static_cast<float>(rand() % static_cast<int>(TextureVulkan::COLOUR_MAX_VALUE)),
				(1.0f / 255.0f) * static_cast<float>(rand() % static_cast<int>(TextureVulkan::COLOUR_MAX_VALUE)),
				(1.0f / 255.0f) * static_cast<float>(rand() % static_cast<int>(TextureVulkan::COLOUR_MAX_VALUE)),
				1.0f
			};
			circle.setTexture(*atlas);
			circle.TextureCoords = atlas->getInnerTextureCoords(
				rand() % atlas->getRowCount(),
				rand() % atlas->getColCount()
			);
			
			CirclesScene.emplace_back(circle);
		}
	}

	void DemoLiciousAppLogic::ShapesDemo::CircleDemo::popScene(size_t count) {
		ZoneScoped;

		const size_t size = CirclesScene.size();
		CirclesScene.resize(count > size ? 0 : size - count);
	}
	
	void DemoLiciousAppLogic::ShapesDemo::init() {
		ZoneScoped;

		mGridDemo = std::make_unique<GridDemo>(SharedResources);
		mBouncingQuadDemo = std::make_unique<BouncingQuadDemo>(SharedResources);
		mCircleDemo = std::make_unique<CircleDemo>(SharedResources);

		mGridDemo->init();
		mBouncingQuadDemo->init();
		mCircleDemo->init();
	}

	void DemoLiciousAppLogic::ShapesDemo::close() {
		ZoneScoped;

		mGridDemo->close();
		mBouncingQuadDemo->close();
		mCircleDemo->close();

		mGridDemo.release();
		mBouncingQuadDemo.release();
		mCircleDemo.release();
	}

	void DemoLiciousAppLogic::ShapesDemo::update(float delta) {
		mGridDemo->update(delta);
		mBouncingQuadDemo->update(delta);
		mCircleDemo->update(delta);
	}

	void DemoLiciousAppLogic::ShapesDemo::render() {
		mGridDemo->render();
		mBouncingQuadDemo->render();
		mCircleDemo->render();
	}

	void DemoLiciousAppLogic::ShapesDemo::renderImGuiMainTab() {
		ZoneScoped;

		mGridDemo->renderImGuiMainTab();
		mBouncingQuadDemo->renderImGuiMainTab();
		mCircleDemo->renderImGuiMainTab();

		//TODO:: Different Shapes demo info

		ImGui::Text("Shapes Batch Renderer Info:");
		ImGui::Text("Quads Drawn: %i", ShapeRenderer::getDrawnQuadCount());
		ImGui::Text("Quads Truncated: %i", ShapeRenderer::getTruncatedQuadCount());
		ImGui::Text("Quad Batch Max Size: %i", EBatchSizeLimits::QUAD_BATCH_MAX_GEO_COUNT);
		ImGui::Text(
			"Quad Batch Count: %i of Max %i",
			ShapeRenderer::getQuadBatchCount(),
			EBatchSizeLimits::QUAD_MAX_BATCH_COUNT
		);
		for (uint32_t i = 0; i < ShapeRenderer::getQuadSceneBatchCount(); i++) {
			RenderBatchQuad& batch = *ShapeRenderer::getQuadSceneRenderBatches()[i];
			ImGui::Text("Scene Batch: %i Geo Count: %i", i, batch.getGeometryCount());
		}
		for (uint32_t i = 0; i < ShapeRenderer::getQuadUiBatchCount(); i++) {
			RenderBatchQuad& batch = *ShapeRenderer::getQuadUiRenderBatches()[i];
			ImGui::Text("UI Batch: %i Geo Count: %i", i, batch.getGeometryCount());
		}
		//TODO:: debug info when multiple texture arrays are supported
		//uint32_t texArrIndex = 0;
		//for (TextureArray& texArr : renderer.getContext().getRenderer2d().getStorage().getTextureArrays()) {
		//	ImGui::Text("Texture Array: %i Texture Count: %i", texArrIndex, texArr.getTextureSlots().size());
		//	texArrIndex++;
		//}
		ImGui::Text(
			"Texture Array: %i Texture Count: %i",
			0,
			ShapeRenderer::getShapesTextureArray().getTextureSlots().size()
		);
		if (ImGui::Button("View Shape Renderer Texture Array")) {
			EditorGui::openTextureArrayViewerWindow("Shape Renderer Texture Array", ShapeRenderer::getShapesTextureArray());
		}
		if (ImGui::Button("Close All Empty Quad Batches")) {
			ShapeRenderer::closeEmptyQuadBatches();
		}
		EditorGui::displayHelpTooltip("Close Empty Quad Batches. This can help clean-up when 1 or more batches have geo counts of 0. Does not include the TextQuad batch.");
		if (ImGui::Button("Close All Empty Circle Batches")) {
			ShapeRenderer::closeEmptyCircleBatches();
		}
		EditorGui::displayHelpTooltip("Close Empty Circle Batches. This can help clean-up when 1 or more batches have geo counts of 0.");
	}

	void DemoLiciousAppLogic::ShapesDemo::renderImGuiExtras() {
		ZoneScoped;

		mGridDemo->renderImGuiExtras();
		mBouncingQuadDemo->renderImGuiExtras();
		mCircleDemo->renderImGuiExtras();
	}

	void DemoLiciousAppLogic::TextDemo::init() {
		ZoneScoped;

		//Generate quads for default message
		SoftMaskScene = std::make_unique<TextString>(
			SoftMaskSceneStringBuffer,
			TextRenderer::getFontBitmap(TextRenderer::ARIAL_SOFT_MASK_NAME)
		);
		
		MsdfTextScene = std::make_unique<TextString>(
			MsdfSceneStringBuffer,
			TextRenderer::getFontBitmap(TextRenderer::ARIAL_MSDF_NAME)
		);

		SoftMaskTextUi = std::make_unique<TextString>(
			SoftMaskUiStringBuffer,
			TextRenderer::getFontBitmap(TextRenderer::ARIAL_SOFT_MASK_NAME)
		);

		MsdfTextUi = std::make_unique<TextString>(
			MsdfUiStringBuffer,
			TextRenderer::getFontBitmap(TextRenderer::ARIAL_MSDF_NAME)
		);

		TextRenderer::setSceneCameraData(SharedResources.PerspectiveSceneCameraController->getCamera().getGpuData());
		TextRenderer::setUiCameraData(SharedResources.OrthoUiCameraController->getCamera().getGpuData());
	}

	void DemoLiciousAppLogic::TextDemo::close() {
		ZoneScoped;

		SoftMaskScene.release();
		MsdfTextScene.release();
		SoftMaskTextUi.release();
		MsdfTextUi.release();
	}

	void DemoLiciousAppLogic::TextDemo::update(float delta) {

	}

	void DemoLiciousAppLogic::TextDemo::render() {
		if (Render) {
			TextRenderer::drawTextStringScene(*SoftMaskScene);
			TextRenderer::drawTextStringScene(*MsdfTextScene);
		
			TextRenderer::drawTextStringUi(*SoftMaskTextUi);
			TextRenderer::drawTextStringUi(*MsdfTextUi);
		}
	}

	void DemoLiciousAppLogic::TextDemo::renderImGuiMainTab() {
		ZoneScoped;

		ImGui::Checkbox("Render", &Render);

		ImGui::Text("Text Batches Geo Count: %i", TextRenderer::getDrawnQuadCount());

		ImGui::Text("String length limit: %i", TextDemo::StringLengthLimit);
		EditorGui::displayHelpTooltip(
			R"(Larger strings can be displayed as the text renderer uses a Quad batch of size 10,000 (by default). )"
			R"(The limitation is because ImGui InputText field requires extra implementation for dynamic data on the heap.)"
		);

		//Soft Mask Scene
		if (ImGui::InputTextMultiline("Display Text Scene", SoftMaskSceneStringBuffer, sizeof(SoftMaskSceneStringBuffer))) {
			SoftMaskScene->setString(SoftMaskSceneStringBuffer);
		}
		float tempScale = SoftMaskScene->getScale();
		if (ImGui::DragFloat("Text Scale", &tempScale, 0.05f, 0.0005f, 5.0f)) {
			if (tempScale > 0.0f) {
				SoftMaskScene->setScale(tempScale);
			}
		}
		float tempRootPos[3] = { SoftMaskScene->Position.x, SoftMaskScene->Position.y, SoftMaskScene->Position.z };
		if (ImGui::DragFloat3("Root Pos", tempRootPos, 0.05f, -10.0f, 10.0f)) {
			SoftMaskScene->setRoot({ tempRootPos[0], tempRootPos[1], tempRootPos[2] });
		}

		//MSDF Scene
		if (ImGui::InputTextMultiline("MSDF Text Scene", MsdfSceneStringBuffer, sizeof(MsdfSceneStringBuffer))) {
			MsdfTextScene->setString(MsdfSceneStringBuffer);
		}
		float tempMsdfScale = MsdfTextScene->getScale();
		if (ImGui::DragFloat("MSDF Text Scene Scale", &tempMsdfScale, 0.05f, 0.0005f, 10.0f)) {
			if (tempMsdfScale > 0.0f) {
				MsdfTextScene->setScale(tempMsdfScale);
			}
		}
		float tempMsdfRootPos[3] = { MsdfTextScene->Position.x, MsdfTextScene->Position.y, MsdfTextScene->Position.z };
		if (ImGui::DragFloat3("MSDF Root Pos", tempMsdfRootPos, 0.05f, -10.0f, 10.0f)) {
			MsdfTextScene->setRoot({ tempMsdfRootPos[0], tempMsdfRootPos[1], tempMsdfRootPos[2] });
		}

		ImGui::Text("UI Text");
		//Soft Mask UI
		if (ImGui::InputTextMultiline("Soft Mask UI Text", SoftMaskUiStringBuffer, sizeof(SoftMaskUiStringBuffer))) {
			SoftMaskTextUi->setString(SoftMaskUiStringBuffer);
		}
		float tempSoftMaskUiScale = SoftMaskTextUi->getScale();
		if (ImGui::DragFloat("Soft Mask UI Scale", &tempSoftMaskUiScale, 0.05f, 0.0005f, 10.0f)) {
			if (tempSoftMaskUiScale > 0.0f) {
				SoftMaskTextUi->setScale(tempSoftMaskUiScale);
			}
		}
		float tempSoftMaskUiRootPos[3] = { SoftMaskTextUi->Position.x, SoftMaskTextUi->Position.y, SoftMaskTextUi->Position.z };
		if (ImGui::DragFloat3("Soft Mask UI Root Pos", tempSoftMaskUiRootPos, 0.05f, -10.0f, 10.0f)) {
			SoftMaskTextUi->setRoot({ tempSoftMaskUiRootPos[0], tempSoftMaskUiRootPos[1], tempSoftMaskUiRootPos[2] });
		}
		//MSDF UI
		if (ImGui::InputTextMultiline("MSDF UI Text", MsdfUiStringBuffer, sizeof(MsdfUiStringBuffer))) {
			MsdfTextUi->setString(MsdfUiStringBuffer);
		}
		float tempMsdfUiScale = MsdfTextUi->getScale();
		if (ImGui::DragFloat("MSDF UI Scale", &tempMsdfUiScale, 0.05f, 0.0005f, 10.0f)) {
			if (tempMsdfUiScale > 0.0f) {
				MsdfTextUi->setScale(tempMsdfUiScale);
			}
		}
		float tempMsdfUiRootPos[3] = { MsdfTextUi->Position.x, MsdfTextUi->Position.y, MsdfTextUi->Position.z };
		if (ImGui::DragFloat3("MSDF UI Root Pos", tempMsdfUiRootPos, 0.05f, -10.0f, 10.0f)) {
			MsdfTextUi->setRoot({ tempMsdfUiRootPos[0], tempMsdfUiRootPos[1], tempMsdfUiRootPos[2] });
		}

		//Colour for all strings, all strings (and even individual characters) can have separate colours, this is limited for brevity.
		if (ImGui::ColorEdit4("String Colour", &Colour.x)) {
			SoftMaskScene->setColour(Colour);
			MsdfTextScene->setColour(Colour);
			SoftMaskTextUi->setColour(Colour);
			MsdfTextUi->setColour(Colour);
		}
		EditorGui::displayHelpTooltip("Each individual letter/glyph can be coloured separately, this UI just doesn't currently allow for it.");
	}

	void DemoLiciousAppLogic::TextDemo::renderImGuiExtras() {
		//No extra windows required for this demo
	}

	void DemoLiciousAppLogic::LineDemo::init() {
		ZoneScoped;

		LineData2d = {};
		LineData3d = {};
		LineDataInput[0] = 0.0f; LineDataInput[1] = 0.0f; LineDataInput[2] = 0.0f;
		LineDataInput[3] = 0.0f; LineDataInput[4] = 0.0f; LineDataInput[5] = 0.0f;
		LineDataInput[6] = 0.0f; LineDataInput[7] = 0.0f; LineDataInput[8] = 0.0f;
		LineDataInput[9] = 1.0f;
	}

	void DemoLiciousAppLogic::LineDemo::close() {
		ZoneScoped;

		LineData2d.clear();
		LineData3d.clear();
	}

	void DemoLiciousAppLogic::LineDemo::update(float delta) {

	}

	void DemoLiciousAppLogic::LineDemo::render() {
		if (Render) {
			const uint32_t spaceRemaining3d = LineRenderer::getSceneMaxLineCount() - LineRenderer::getSceneLineCount();
		
			if (RenderTextDemoOutlines) {
				TextDemo& textDemo = TextDemoRef.get();
				for (const Quad& quad : textDemo.SoftMaskScene->getQuads()) {
					LineRenderer::drawQuadScene(quad, { 1.0f, 0.0f, 1.0f, 1.0f });
				}
		
				for (const Quad& quad : textDemo.MsdfTextScene->getQuads()) {
					LineRenderer::drawQuadScene(quad, { 0.0f, 1.0f, 0.0f, 1.0f });
				}
		
				for (const Quad& quad : textDemo.SoftMaskTextUi->getQuads()) {
					LineRenderer::drawQuadUi(quad, { 1.0f, 0.0f, 1.0f, 1.0f });
				}
		
				for (const Quad& quad : textDemo.MsdfTextUi->getQuads()) {
					LineRenderer::drawQuadUi(quad, { 0.0f, 1.0f, 0.0f, 1.0f });
				}
			}
		
			for (uint32_t i = 0; i < LineCount3d && i < spaceRemaining3d; i++) {
				const size_t lineStartIndex = static_cast<size_t>(i) * LineDemo::LINE_3D_INPUT_COMPONENT_COUNT;
				LineRenderer::drawLineScene(
					{
						LineData3d[lineStartIndex + 0],
						LineData3d[lineStartIndex + 1],
						LineData3d[lineStartIndex + 2]
					},
					{
						LineData3d[lineStartIndex + 3],
						LineData3d[lineStartIndex + 4],
						LineData3d[lineStartIndex + 5],
					},
					{
						LineData3d[lineStartIndex + 6],
						LineData3d[lineStartIndex + 7],
						LineData3d[lineStartIndex + 8],
						LineData3d[lineStartIndex + 9]
					}
				);
			}
		
			if (spaceRemaining3d < LineCount3d) {
				LOG_WARN("LineDemo 3D lines truncated: " << LineCount3d - spaceRemaining3d);
			}
		}

		if (RenderUiQuad) {
			LineRenderer::drawQuadUi(UiQuadTest, UiQuadTest.Colour);

			const uint32_t spaceRemaining = LineRenderer::getUiMaxLineCount() - LineRenderer::getUiLineCount();

			for (uint32_t i = 0; i < LineCount2d && i < spaceRemaining; i++) {
				const size_t lineStartIndex = static_cast<size_t>(i) * LineDemo::LINE_2D_INPUT_COMPONENT_COUNT;
				LineRenderer::drawLineUi(
					{ LineData2d[lineStartIndex + 0], LineData2d[lineStartIndex + 1] },
					{ LineData2d[lineStartIndex + 2], LineData2d[lineStartIndex + 3] },
					{
						LineData2d[lineStartIndex + 4], LineData2d[lineStartIndex + 5],
						LineData2d[lineStartIndex + 6], LineData2d[lineStartIndex + 7]
					}
				);

				if (spaceRemaining < LineCount2d) {
					LOG_WARN("LineDemo 2D lines truncated: " << LineCount2d - spaceRemaining);
				}
			}
		}
	}

	void DemoLiciousAppLogic::LineDemo::renderImGuiMainTab() {
		ZoneScoped;

		ImGui::Checkbox("Render", &Render);
		ImGui::Checkbox("Render Text Demo Outlines", &RenderTextDemoOutlines);
		ImGui::Checkbox("Render Ui Quad", &RenderUiQuad);

		ImGui::Text("Scene Line Count: %i", LineRenderer::getSceneLineCount());
		EditorGui::displayHelpTooltip("This includes lines created from \"Add Line Scene\" and from any draw{ Primitive }3d() function calls.");
		ImGui::Text("UI Line Count: %i", LineRenderer::getUiLineCount());
		EditorGui::displayHelpTooltip("This includes lines created from \"Add Line UI\" and from any draw{ Primitive }2d() function calls.");

		float lineData[LineDemo::LINE_3D_INPUT_COMPONENT_COUNT] = {};
		for (int i = 0; i < LineDemo::LINE_3D_INPUT_COMPONENT_COUNT; i++) {
			lineData[i] = LineDataInput[i];
		}
		ImGui::InputFloat3("Start", lineData);
		ImGui::InputFloat3("End", &lineData[3]);
		ImGui::ColorEdit4("Colour", &lineData[6]);
		if (ImGui::Button("Add Line Scene")) {
			if (lineData[0] != lineData[3] || lineData[1] != lineData[4] || lineData[2] != lineData[5]) {
				addLine3d(
					{ lineData[0], lineData[1], lineData[2] },
					{ lineData[3], lineData[4], lineData[5] },
					{ lineData[6], lineData[7], lineData[8], lineData[9] }
				);
			} else {
				LOG_WARN("Line Start and End are at the same point, line has not been added.")
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("Add Line UI")) {
			if (lineData[0] != lineData[3] || lineData[1] != lineData[4]) {
				addLine2d(
					{ lineData[0], lineData[1] },
					{ lineData[3], lineData[4] },
					{ lineData[6], lineData[7], lineData[8], lineData[9] }
				);
			} else {
				LOG_WARN("Line Start and End are at the same point, line has not been added.")
			}
		}
		for (int i = 0; i < LineDemo::LINE_3D_INPUT_COMPONENT_COUNT; i++) {
			LineDataInput[i] = lineData[i];
		}
		ImGui::InputInt("Pop count", &LinePopCount);
		if (ImGui::Button("Pop Line(s) Scene")) {
			popLines3d(LinePopCount);
		}
		ImGui::SameLine();
		if (ImGui::Button("Pop Line(s) UI")) {
			popLines2d(LinePopCount);
		}
		EditorGui::displayHelpTooltip("Only pops lines created from \"Add Line Scene/UI\", any created by draw{Primitive} calls are not affected.");

		ImGui::Text("UI Quad");
		EditorGui::controlsQuad(UiQuadTest, "Ui Quad");
	}

	void DemoLiciousAppLogic::LineDemo::renderImGuiExtras() {
		//No extra windows required for this demo
	}

	void DemoLiciousAppLogic::LineDemo::addLine2d(glm::vec2 start, glm::vec2 end, glm::vec4 colour) {
		ZoneScoped;

		if (LineCount2d < LINE_BATCH_MAX_LINE_COUNT) {
			LineData2d.reserve(LineData2d.size() + static_cast<size_t>(LINE_2D_INPUT_COMPONENT_COUNT));
			LineData2d.emplace_back(start.x);
			LineData2d.emplace_back(start.y);
			LineData2d.emplace_back(end.x);
			LineData2d.emplace_back(end.y);
			LineData2d.emplace_back(colour.r);
			LineData2d.emplace_back(colour.g);
			LineData2d.emplace_back(colour.b);
			LineData2d.emplace_back(colour.a);

			LineCount2d++;
			LineDataIndex2d += LINE_2D_INPUT_COMPONENT_COUNT;
		}
	}

	void DemoLiciousAppLogic::LineDemo::addLine3d(glm::vec3 start, glm::vec3 end, glm::vec4 colour) {
		ZoneScoped;

		if (LineCount3d < LINE_BATCH_MAX_LINE_COUNT) {
			LineData3d.reserve(LineData3d.size() + static_cast<size_t>(LINE_3D_INPUT_COMPONENT_COUNT));
			LineData3d.emplace_back(start.x);
			LineData3d.emplace_back(start.y);
			LineData3d.emplace_back(start.z);
			LineData3d.emplace_back(end.x);
			LineData3d.emplace_back(end.y);
			LineData3d.emplace_back(end.z);
			LineData3d.emplace_back(colour.r);
			LineData3d.emplace_back(colour.g);
			LineData3d.emplace_back(colour.b);
			LineData3d.emplace_back(colour.a);

			LineCount3d++;
			LineDataIndex3d += LINE_3D_INPUT_COMPONENT_COUNT;
		}
	}

	void DemoLiciousAppLogic::BoundingBoxDemo::init() {
		ZoneScoped;

		BoundingBox = std::make_unique<BoundingBox2d>();
	}

	void DemoLiciousAppLogic::BoundingBoxDemo::close() {
		ZoneScoped;

		BoundingBox->reset();
		Quads.clear();
	}

	void DemoLiciousAppLogic::BoundingBoxDemo::update(float delta) {

	}

	void DemoLiciousAppLogic::BoundingBoxDemo::render() {
		if (Render) {
			if (Quads.size() > 0) {
				ShapeRenderer::drawQuadArrayScene(Quads);
				LineRenderer::drawQuadScene(BoundingBox->getQuad(), { 0.0f, 1.0f, 0.0f, 1.0f });
			}
		}
	}

	void DemoLiciousAppLogic::BoundingBoxDemo::renderImGuiMainTab() {
		ZoneScoped;

		ImGui::Checkbox("Render", &Render);
		ImGui::Text("Quad Count: %i", Quads.size());

		ImGui::Text("Bounding Box");
		EditorGui::infoQuad(BoundingBox->getQuad(), "BoundingBox");

		ImGui::NewLine();
		ImGui::Text("New Quad:");
		EditorGui::controlsQuad(TempQuadToAdd, "QuadToAdd");
		if (ImGui::Button("Add Quad")) {
			if (TempQuadToAdd.Size.x != 0.0f && TempQuadToAdd.Size.y != 0.0f) {

				Quads.emplace_back(TempQuadToAdd);
				BoundingBox->resizeToFit(TempQuadToAdd);
			} else {
				LOG_WARN("New Quad must have a size.x & size.y greater than 0 to add to bounding box.");
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("Pop Quad")) {
			if (!Quads.empty()) {
				Quads.pop_back();

				if (Quads.empty()) {
					BoundingBox->reset();
				} else {
					//TODO:: very inefficient to re-calculate for all geo,
					//	maybe include in box a vector of all geo
					
					//Since individual geo can't be removed from bounding box without potentially falsifying the bounding box pos & size
					//a full reset is required to guarantee it is correct.
					BoundingBox->reset();
					for (Quad& quad : Quads) {
						BoundingBox->resizeToFit(quad);
					}
				}
			}
		}
		if (ImGui::Button("Reset")) {
			BoundingBox->reset();
			Quads.clear();
		}
		ImGui::SameLine();
		if (ImGui::Button("Reset New Quad")) {
			TempQuadToAdd = {};
		}
	}

	void DemoLiciousAppLogic::BoundingBoxDemo::renderImGuiExtras()  {

	}

	void DemoLiciousAppLogic::TileMapDemo::init() {
		ZoneScoped;

		//NOTE:: Texture atlas is created in QuadBatch init because rebiding descriptors not currently available.

		const std::shared_ptr<IndexedTextureAtlas> texAtlas = ShapeRenderer::getTestIndexedTextureAtlas();

		SceneTileMap = std::make_unique<TileMap>(*texAtlas, 50, 50);

		PreviewAnimationController = std::make_unique<TextureAtlasAnimationController>(texAtlas->getAnimation("testAnim"));

		PreviewQuad = { { 0.0f, -1.0f, 1.0f }, { 2.0f, 2.0f }, { 1.f, 1.0f, 1.0f, 1.0f }, 0.0f, texAtlas };
		PreviewQuad.setTexture(*texAtlas);

		AnimatedQuad = { { -2.0f, -1.0f, 1.0f }, { 2.0f, 2.0f }, { 1.f, 1.0f, 1.0f, 1.0f }, 0.0f, texAtlas, PreviewAnimationController->getCurrentInnerTexture().getTexCoordsAsSquare() };
		AnimatedQuad.setTexture(*texAtlas);
	}

	void DemoLiciousAppLogic::TileMapDemo::close() {
		
	}

	void DemoLiciousAppLogic::TileMapDemo::update(float delta) {
		if (Update) {
			if (PreviewAnimationController->update(delta)) {
				AnimatedQuad.TextureCoords = PreviewAnimationController->getCurrentInnerTexture().getTexCoordsAsSquare();
			}
		}
	}

	void DemoLiciousAppLogic::TileMapDemo::render() {
		if (Render) {
			if (RenderPreviewQuad) {
				ShapeRenderer::drawQuadTexturedScene(PreviewQuad);
				ShapeRenderer::drawQuadTexturedScene(AnimatedQuad);
			}
		}
	}

	void DemoLiciousAppLogic::TileMapDemo::renderImGuiMainTab() {
		ZoneScoped;

		ImGui::Checkbox("Render", &Render);
		ImGui::Checkbox("Update", &Update);
		ImGui::Checkbox("Render Preview Quad", &RenderPreviewQuad);

		if (ImGui::Button("View Texture Atlas")) {
			EditorGui::openIndexedTextureAtlasViewerWindow(*ShapeRenderer::getTestIndexedTextureAtlas());
		}

		EditorGui::controlsQuad(PreviewQuad, "PreviewQuad");

		ImGui::Text("Inner Textures");

		for (const auto& innerTexture : SceneTileMap->getTextureAtlas().getInnerTextures()) {
			const char* texName = innerTexture.first.c_str();
			bool previewButton = texName == PreviewedInnerTexture;
			if (previewButton) {
				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.5f, 0.0f, 1.0f));
			}
			if (ImGui::Button(innerTexture.first.c_str())) {
				PreviewedInnerTexture = innerTexture.first.c_str();
				PreviewQuad.TextureCoords = innerTexture.second.getTexCoordsAsSquare();
			}
			if (previewButton) {
				ImGui::PopStyleColor(1);
			}
		}

		if (ImGui::Button("Reset Preview")) {
			PreviewQuad.TextureCoords = Quad::DEFAULT_TEXTURE_COORDS;
			PreviewedInnerTexture = "NONE";
		}

		ImGui::Text("Animation");
		const bool isPlaying = PreviewAnimationController->isPlaying();
		if (ImGui::Button(isPlaying ? "Pause" : "Play")) {
			PreviewAnimationController->setPlaying(!isPlaying);
		}
		if (ImGui::Button("Reset Animation")) {
			PreviewAnimationController->reset();
			AnimatedQuad.TextureCoords = PreviewAnimationController->getCurrentInnerTexture().getTexCoordsAsSquare();
		}
	}

	void DemoLiciousAppLogic::TileMapDemo::renderImGuiExtras() {

	}

	void DemoLiciousAppLogic::MultiCameraPassDemo::init() {
		RenderingContextVulkan& context = Application::get().getRenderer().getContext();

		OrthoSceneCamera = std::make_unique<DOH::OrthographicCamera>(-SharedResources.AspectRatio, SharedResources.AspectRatio, -1.0f, 1.0f);
		context.createCameraGpuData(*OrthoSceneCamera);
		context.addCameraToUpdateList(MultiCameraPassDemo::ORTHO_SCENE_CAM_NAME, *OrthoSceneCamera);
		OrthoSceneCameraController = std::make_unique<EDITOR::EditorOrthoCameraController>(
			*OrthoSceneCamera,
			SharedResources.InputLayer,
			SharedResources.AspectRatio
		);

		PerspectiveUiCamera = std::make_unique<DOH::PerspectiveCamera>(SharedResources.AspectRatio);
		PerspectiveUiCameraController = std::make_unique<EDITOR::EditorPerspectiveCameraController>(
			*PerspectiveUiCamera,
			SharedResources.InputLayer,
			SharedResources.AspectRatio
		);
		context.createCameraGpuData(*PerspectiveUiCamera);
		context.addCameraToUpdateList(MultiCameraPassDemo::PERSP_UI_CAM_NAME, *PerspectiveUiCamera);

		//TODO:: Not sure if the UI persp cam is in the right coord space, should be consistent with other persp cams in the engine.

		DescriptorSetLayoutVulkan& cameraUboLayout = context.getCommonDescriptorSetLayouts().Ubo;
		std::vector<std::reference_wrapper<DescriptorSetLayoutVulkan>> ui3dDescSets = { cameraUboLayout };
		VkPushConstantRange pushConstant = context.pushConstantInfo(VK_SHADER_STAGE_VERTEX_BIT, sizeof(UniformBufferObject), 0);
		std::vector<VkPushConstantRange> pushConstants = { pushConstant };
		std::shared_ptr<ShaderDescriptorSetLayoutsVulkan> ui3dDescSetLayouts = std::make_shared<ShaderDescriptorSetLayoutsVulkan>(pushConstants, ui3dDescSets);
		Ui3dVertexShader = context.createShader(EShaderStage::VERTEX, FlatColourShaderVertPath);
		Ui3dFragmentShader = context.createShader(EShaderStage::FRAGMENT, FlatColourShaderFragPath);
		Ui3dShaderProgram = context.createShaderProgram(
			Ui3dVertexShader,
			Ui3dFragmentShader,
			ui3dDescSetLayouts
		);

		//TODO:: This is not needed as it won't point to the correct imageIndex and, right now, should be updated during demo::render
		std::initializer_list<VkDescriptorSet> ui3dDescSetsInstance = {
			//Maybe have cameras' descSetInstance do a getCurrentImageIndex each frame, however, the RenderingContext::ImageIndex is changed at the start of drawFrame so it might not work/be a frame behind
			PerspectiveUiCameraController->getCamera().getGpuData()->DescriptorSets[context.getCurrentFrame()]
			//SharedResources.mOrthoUiCameraController->getCamera().getGpuData()->DescriptorSets[context.getCurrentFrame()]
		};
		Ui3dDescSetsInstance = std::make_shared<DescriptorSetsInstanceVulkan>(ui3dDescSetsInstance);

		Ui3dPipelineInfo = std::make_unique<GraphicsPipelineInstanceInfo>(
			ColouredVertexInputLayout,
			*Ui3dShaderProgram,
			ERenderPass::APP_UI
		);
		//auto& ui3dOptionalFields = Ui3dPipelineInfo->enableOptionalFields();
		
		Ui3dPipelineConveyor = context.createPipelineInCurrentRenderState(Ui3dPipelineName, *Ui3dPipelineInfo);

		UiQuad = {{ 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f, 0.0f, 1.0f }};
		
		UiCubeModelName = "UI 3D Cube";
		CubeUiTransformation = std::make_shared<TransformationData>();
		CubeUiTransformation->Position.z = 0.5f; //Since Ortho is 
		CubeUiTransformation->Scale = 0.1f;

		Ui3dModel = ModelVulkan::createModel("Dough/Dough/res/models/testCube.obj", ColouredVertexInputLayout);
		Ui3dModelInstance = std::make_shared<RenderableModelVulkan>(
			UiCubeModelName,
			Ui3dModel,
			CubeUiTransformation,
			Ui3dDescSetsInstance
		);

		Ui3dPipelineConveyor.addRenderable(Ui3dModelInstance);
	}

	void DemoLiciousAppLogic::MultiCameraPassDemo::close() {
		RendererVulkan& renderer = GET_RENDERER;

		renderer.closeGpuResource(Ui3dModel);
		renderer.closeGpuResourceOwner(Ui3dShaderProgram);

		renderer.closeGpuResourceOwner(OrthoSceneCameraController->getCamera().getGpuData());
		renderer.closeGpuResourceOwner(PerspectiveUiCameraController->getCamera().getGpuData());

		//TODO:: Some way of telling the CurrentRenderState to close specific pipelines.
		//e.g. renderer.closePipelineInCurrentRenderState(ERenderPass::___, pipelineName);
	}

	void DemoLiciousAppLogic::MultiCameraPassDemo::update(float delta) {
		OrthoSceneCameraController->onUpdate(delta);
		PerspectiveUiCameraController->onUpdate(delta);
	}

	void DemoLiciousAppLogic::MultiCameraPassDemo::render() {
		auto& context = Application::get().getRenderer().getContext();

		if (Render) {
			ShapeRenderer::drawQuadUi(UiQuad);

			//TODO:: A much better way than this!!
			//	This points to the WRONG imageIndex as this is done before the swap chain retrieves the next image. This WILL be fixed.
			const uint32_t uboSlot = 0;
			Ui3dModelInstance->getDescriptorSetsInstance()->getDescriptorSets()[uboSlot] = PerspectiveUiCameraController->getCamera().getGpuData()->DescriptorSets[context.getCurrentFrame()];
			Ui3dPipelineConveyor.addRenderable(Ui3dModelInstance);
		}
	}

	void DemoLiciousAppLogic::MultiCameraPassDemo::renderImGuiMainTab() {
		ImGui::Checkbox("Render", &Render);
		ImGui::Checkbox("Update", &Update);

		EditorGui::controlsQuad(UiQuad, "Ui Quad");
		//TODO:: EditorGui::controls/info -RenderableModelVulkan(&UiModelInstance, Ui3dModelInstance->Name.c_str())
		// Another demo uses a function (ObjModelsDemo::imGuiDrawObjDemoItem) that is very similar but the scales, interpolation values and demo specifics make it so that that code isn't suitable here.
		// Repurpose it to make it more universal?

		ImGui::NewLine();

		ImGui::Text("Ui3d Model Controls: ");
		//Add temp array and set -1 > rotation < 361 to allow for "infinite" drag
		TransformationData& transformation = *Ui3dModelInstance->Transformation;
		float tempRotation[3] = {
			transformation.Rotation[0],
			transformation.Rotation[1],
			transformation.Rotation[2]
		};
		bool transformed = false;
		if (ImGui::DragFloat3(
			"Position",
			glm::value_ptr(transformation.Position),
			0.05f,
			-10.0f,
			10.0f
		)) {
			transformed = true;
		}
		if (ImGui::DragFloat3("Rotation", tempRotation, 1.0f, -1.0f, 361.0f)) {
			if (tempRotation[0] > 360.0f) {
				tempRotation[0] = 0.0f;
			} else if (tempRotation[0] < 0.0f) {
				tempRotation[0] = 360.0f;
			}
			if (tempRotation[1] > 360.0f) {
				tempRotation[1] = 0.0f;
			} else if (tempRotation[1] < 0.0f) {
				tempRotation[1] = 360.0f;
			}
			if (tempRotation[2] > 360.0f) {
				tempRotation[2] = 0.0f;
			} else if (tempRotation[2] < 0.0f) {
				tempRotation[2] = 360.0f;
			}

			transformation.Rotation = { tempRotation[0], tempRotation[1], tempRotation[2] };

			transformed = true;
		}
		if (ImGui::DragFloat(
			"Uniform Scale",
			&transformation.Scale,
			0.01f,
			0.1f,
			5.0f
		)) {
			transformed = true;
		}

		if (ImGui::Button("Reset Object")) {
			transformation.Rotation[0] = 0.0f;
			transformation.Rotation[1] = 0.0f;
			transformation.Rotation[2] = 0.0f;
			transformation.Position[0] = 0.0f;
			transformation.Position[1] = 0.0f;
			transformation.Position[2] = 0.0f;
			transformation.Scale = 1.0f;

			transformed = true;
		}

		if (transformed && Update) {
			transformation.updateTranslationMatrix();
		}
	}

	void DemoLiciousAppLogic::MultiCameraPassDemo::renderImGuiExtras() {
		
	}
}
