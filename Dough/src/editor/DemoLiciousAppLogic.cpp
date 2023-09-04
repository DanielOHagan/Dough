#include "editor/DemoLiciousAppLogic.h"

#include "editor/ui/EditorGui.h"

#include "dough/application/Application.h"
#include "dough/rendering/ObjInit.h"

#define GET_RENDERER Application::get().getRenderer()

namespace DOH::EDITOR {

	const std::string DemoLiciousAppLogic::SharedDemoResources::TexturedShaderVertPath = "Dough/res/shaders/spv/Textured.vert.spv";
	const std::string DemoLiciousAppLogic::SharedDemoResources::TexturedShaderFragPath = "Dough/res/shaders/spv/Textured.frag.spv";

	void DemoLiciousAppLogic::init(float aspectRatio) {
		mImGuiSettings = std::make_unique<ImGuiSettings>();

		mInputLayer = std::make_shared<DefaultInputLayer>("DemoLicious");
		Input::addInputLayer(mInputLayer);

		initDemos();

		setUiProjection(aspectRatio);
	}

	void DemoLiciousAppLogic::update(float delta) {
		if (mBouncingQuadDemo->Update) {
			const float translationDelta = 0.2f * delta;
			for (size_t i = 0; i < mBouncingQuadDemo->BouncingQuads.size(); i++) {
				Quad& quad = mBouncingQuadDemo->BouncingQuads[i];
				glm::vec2& velocity = mBouncingQuadDemo->BouncingQuadVelocities[i];

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

		if (mGridDemo->Update && !mGridDemo->IsUpToDate) {
			//Repopulate entire array each update, not very efficient but the test grid is just an example
			populateTestGrid(static_cast<uint32_t>(mGridDemo->TestGridSize[0]), static_cast<uint32_t>(mGridDemo->TestGridSize[1]));
		}

		//NOTE:: Obj models aren't updated per Update cycle, done during ImGui render stage, as currently only the ImGui
		// UI has control over the transform data. And recalculating x number of object's transformation is a lot of wasted
		// work.
		// 
		//if (mObjModelsDemo->Update) {
		//	for (const auto& obj : mObjModelsDemo->mRenderableObjects) {
		//		obj->Transformation->updateTranslationMatrix();
		//	}
		//}

		if (mTileMapDemo->Update) {
			if (mTileMapDemo->PreviewAnimationController->update(delta)) {
				mTileMapDemo->AnimatedQuad.TextureCoords = mTileMapDemo->PreviewAnimationController->getCurrentInnerTexture().TextureCoords;
			}
		}
	}

	void DemoLiciousAppLogic::render() {
		RendererVulkan& renderer = GET_RENDERER;
		RenderingContextVulkan& context = renderer.getContext();
		Renderer2dVulkan& renderer2d = context.getRenderer2d();


		//Use a custom camera to this application to render the scene differently to the editor
		//renderer.beginScene();

		if (mCustomDemo->RenderScene) {
			//context.addRenderableToSceneDrawList(mCustomDemo->ScenePipelineName, mCustomDemo->SceneRenderable);
			
			//mCustomDemo->CustomSceneConveyor.safeAddRenderable(mCustomDemo->SceneRenderable);
			mSharedDemoResources->TexturedConveyor.safeAddRenderable(mCustomDemo->SceneRenderable);
		}

		if (mObjModelsDemo->GpuResourcesLoaded && mObjModelsDemo->Render) {
			if (mObjModelsDemo->ScenePipelineConveyor.isValid() && mObjModelsDemo->WireframePipelineConveyor.isValid()) {
				const bool renderAllStandard = mObjModelsDemo->RenderAllStandard;
				const bool renderAllWireframe = mObjModelsDemo->RenderAllWireframe;

				for (auto& obj : mObjModelsDemo->RenderableObjects) {
					if (renderAllStandard || obj->Render) {
						mObjModelsDemo->ScenePipelineConveyor.addRenderable(obj);
					}
					if (renderAllWireframe || obj->RenderWireframe) {
						mObjModelsDemo->WireframePipelineConveyor.addRenderable(obj);
					}
				}
			}

			if (mSharedDemoResources->TexturedConveyor.isValid() && mObjModelsDemo->RenderableTexturedModel->Render) {
				mSharedDemoResources->TexturedConveyor.addRenderable(mObjModelsDemo->RenderableTexturedModel);
			}
		}

		if (mGridDemo->Render) {
			if (mGridDemo->QuadDrawColour) {
				for (const std::vector<Quad>& sameTexturedQuads : mGridDemo->TexturedTestGrid) {
					renderer2d.drawQuadArrayScene(sameTexturedQuads);
				}
			} else {
				for (const std::vector<Quad>& sameTexturedQuads : mGridDemo->TexturedTestGrid) {
					renderer2d.drawQuadArraySameTextureScene(sameTexturedQuads);
				}
			}
		}

		if (mBouncingQuadDemo->Render) {
			if (mBouncingQuadDemo->QuadDrawColour) {
				renderer2d.drawQuadArrayScene(mBouncingQuadDemo->BouncingQuads);
			} else {
				renderer2d.drawQuadArrayTexturedScene(mBouncingQuadDemo->BouncingQuads);
			}
		}

		if (mTextDemo->Render) {
			renderer2d.drawTextString(*mTextDemo->Text);
		}

		if (mLineDemo->Render) {
			auto& lineRenderer = context.getLineRenderer();
			const uint32_t spaceRemaining = lineRenderer.getSceneMaxLineCount() - lineRenderer.getSceneLineCount();

			if (mLineDemo->RenderTextDemoOutlines) {
				for (const Quad& quad : mTextDemo->Text->getQuads()) {
					lineRenderer.drawQuadScene(quad, { 1.0f, 0.0f, 1.0f, 1.0f });
				}
			}

			for (uint32_t i = 0; i < mLineDemo->LineCount3d && i < spaceRemaining; i++) {
				const size_t lineStartIndex = static_cast<size_t>(i) * LineDemo::LINE_3D_INPUT_COMPONENT_COUNT;
				lineRenderer.drawLineScene(
					{
						mLineDemo->LineData3d[lineStartIndex + 0],
						mLineDemo->LineData3d[lineStartIndex + 1],
						mLineDemo->LineData3d[lineStartIndex + 2]
					},
					{
						mLineDemo->LineData3d[lineStartIndex + 3],
						mLineDemo->LineData3d[lineStartIndex + 4],
						mLineDemo->LineData3d[lineStartIndex + 5],
					},
					{
						mLineDemo->LineData3d[lineStartIndex + 6],
						mLineDemo->LineData3d[lineStartIndex + 7],
						mLineDemo->LineData3d[lineStartIndex + 8],
						mLineDemo->LineData3d[lineStartIndex + 9]
					}
				);
			}

			if (spaceRemaining < mLineDemo->LineCount3d) {
				LOG_WARN("LineDemo 3D lines truncated: " << mLineDemo->LineCount3d - spaceRemaining);
			}
		}

		if (mBoundingBoxDemo->Render) {
			if (mBoundingBoxDemo->Quads.size() > 0) {
				renderer2d.drawQuadArrayScene(mBoundingBoxDemo->Quads);
				context.getLineRenderer().drawQuadScene(mBoundingBoxDemo->BoundingBox->getQuad(), {0.0f, 1.0f, 0.0f, 1.0f});
			}
		}

		if (mTileMapDemo->Render) {
			if (mTileMapDemo->RenderPreviewQuad) {
				renderer2d.drawQuadTexturedScene(mTileMapDemo->PreviewQuad);
				renderer2d.drawQuadTexturedScene(mTileMapDemo->AnimatedQuad);
			}
		}

		renderer.endScene();

		renderer.beginUi(mCustomDemo->UiProjMat);
		if (mCustomDemo->RenderUi) {
			mCustomDemo->CustomUiConveyor.safeAddRenderable(mCustomDemo->UiRenderable);
		}

		if (mLineDemo->Render) {
			auto& lineRenderer = context.getLineRenderer();

			if (mLineDemo->RenderUiQuad) {
				lineRenderer.drawQuadUi(mLineDemo->UiQuadTest, mLineDemo->UiQuadTest.Colour);
			}

			const uint32_t spaceRemaining = lineRenderer.getUiMaxLineCount() - lineRenderer.getUiLineCount();

			for (uint32_t i = 0; i < mLineDemo->LineCount2d && i < spaceRemaining; i++) {
				const size_t lineStartIndex = static_cast<size_t>(i) * LineDemo::LINE_2D_INPUT_COMPONENT_COUNT;
				lineRenderer.drawLineUi(
					{ mLineDemo->LineData2d[lineStartIndex + 0], mLineDemo->LineData2d[lineStartIndex + 1] },
					{ mLineDemo->LineData2d[lineStartIndex + 2], mLineDemo->LineData2d[lineStartIndex + 3] },
					{
						mLineDemo->LineData2d[lineStartIndex + 4],
						mLineDemo->LineData2d[lineStartIndex + 5],
						mLineDemo->LineData2d[lineStartIndex + 6],
						mLineDemo->LineData2d[lineStartIndex + 7]
					}
				);

				if (spaceRemaining < mLineDemo->LineCount2d) {
					LOG_WARN("LineDemo 2D lines truncated: " << mLineDemo->LineCount2d - spaceRemaining);
				}
			}
		}

		renderer.endUi();
	}

	void DemoLiciousAppLogic::imGuiRender(float delta) {
		RendererVulkan& renderer = GET_RENDERER;
		Renderer2dVulkan& renderer2d = renderer.getContext().getRenderer2d();

		if (ImGui::Begin("DemoLicious Demos")) {
			ImGui::Text("Demo Settings & Info:");

			ImGui::BeginTabBar("Demo Tab Bar");

			mGridDemo->renderImGuiMainTab();
			mBouncingQuadDemo->renderImGuiMainTab();
			mCustomDemo->renderImGuiMainTab();
			mObjModelsDemo->renderImGuiMainTab();
			mTextDemo->renderImGuiMainTab();
			mLineDemo->renderImGuiMainTab();
			mBoundingBoxDemo->renderImGuiMainTab();
			mTileMapDemo->renderImGuiMainTab();

			ImGui::EndTabBar();

			ImGui::NewLine();
			ImGui::Text("Cross-demo functions/resources");
			if (ImGui::Button("Disable all demos")) {
				mGridDemo->Render = false;
				mGridDemo->Update = false;

				mBouncingQuadDemo->Render = false;
				mBouncingQuadDemo->Update = false;

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
			}
			if (ImGui::Button("View atlas texture")) {
				EditorGui::openMonoSpaceTextureAtlasViewerWindow(*renderer2d.getStorage().getTestMonoSpaceTextureAtlas());
			}
			EditorGui::displayHelpTooltip("Display Texture Altas texture inside a separate window.");
			if (ImGui::Button("View test texture")) {
				EditorGui::openTextureViewerWindow(*mSharedDemoResources->TestTexture1);
			}
			EditorGui::displayHelpTooltip("Display test texture inside a separate window.");
		}
		ImGui::End();

		mGridDemo->renderImGuiExtras();
		mBouncingQuadDemo->renderImGuiExtras();
		mCustomDemo->renderImGuiExtras();
		mObjModelsDemo->renderImGuiExtras();
		mTextDemo->renderImGuiExtras();
		mLineDemo->renderImGuiExtras();
		mBoundingBoxDemo->renderImGuiExtras();
		mTileMapDemo->renderImGuiExtras();
	}

	void DemoLiciousAppLogic::close() {
		closeDemos();
	}

	void DemoLiciousAppLogic::onResize(float aspectRatio) {
		setUiProjection(aspectRatio);
	}

	void DemoLiciousAppLogic::initDemos() {
		RenderingContextVulkan& context = GET_RENDERER.getContext();
	
		//Test grid is repopulated per update to apply changes from editor. To only populate once remove re-population from update and populate here.
		//populateTestGrid(static_cast<uint32_t>(mGridDemo.TestGridSize[0]), static_cast<uint32_t>(mGridDemo.TestGridSize[1]));

		initSharedResources();

		mGridDemo = std::make_unique<GridDemo>();
		mGridDemo->init();

		mBouncingQuadDemo = std::make_unique<BouncingQuadDemo>();
		mBouncingQuadDemo->init();

		if (mSharedDemoResources != nullptr) {
			mCustomDemo = std::make_unique<CustomDemo>();
			mCustomDemo->init();
			
			mObjModelsDemo = std::make_unique<ObjModelsDemo>();
			mObjModelsDemo->init();
		}
		
		mTextDemo = std::make_unique<TextDemo>();
		mTextDemo->init();

		mLineDemo = std::make_unique<LineDemo>();
		mLineDemo->init();

		mBoundingBoxDemo = std::make_unique<BoundingBoxDemo>();
		mBoundingBoxDemo->init();

		mTileMapDemo = std::make_unique<TileMapDemo>();
		mTileMapDemo->init();

		context.createPipelineUniformObjects();
	}

	void DemoLiciousAppLogic::closeDemos() {
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

		mGridDemo->close();
		mGridDemo.release();

		mLineDemo->close();
		mLineDemo.release();

		mBoundingBoxDemo->close();
		mBoundingBoxDemo.release();

		mTileMapDemo->close();
		mTileMapDemo.release();

		closeSharedResources();
	}
	
	void DemoLiciousAppLogic::populateTestGrid(uint32_t width, uint32_t height) {
		const auto& storage = GET_RENDERER.getContext().getRenderer2d().getStorage();
		const auto& atlas = storage.getTestMonoSpaceTextureAtlas();
	
		mGridDemo->TexturedTestGrid.clear();
		mGridDemo->TexturedTestGrid.resize(storage.getQuadBatchTextureArray().getTextureSlots().size());
		const uint32_t textureSlot = storage.getQuadBatchTextureArray().getTextureSlotIndex(atlas->getId());
	
		uint32_t index = 0;
		for (uint32_t y = 0; y < height; y++) {
			for (uint32_t x = 0; x < width; x++) {
	
				//Push back a Quad with both a texture and a custom colour,
				// in this case whether it is drawn with a texture or with a colour
				// is determined by mGridDemo.QuadDrawColour
	
				mGridDemo->TexturedTestGrid[textureSlot].push_back({
					{
						static_cast<float>(x) * mGridDemo->TestGridQuadGapSize[0],
						static_cast<float>(y) * mGridDemo->TestGridQuadGapSize[1],
						0.5f
					},
					{ mGridDemo->TestGridQuadSize[0], mGridDemo->TestGridQuadSize[1] },
					{ mGridDemo->QuadColour.x, mGridDemo->QuadColour.y, mGridDemo->QuadColour.z, mGridDemo->QuadColour.w },
					0.0f,
					atlas,
					atlas->getInnerTextureCoords(
						x + mGridDemo->TestTexturesRowOffset,
						y + mGridDemo->TestTexturesColumnOffset
					)
				});
				index++;
			}
		}
	
		mGridDemo->IsUpToDate = true;
	}

	void DemoLiciousAppLogic::initSharedResources() {
		mSharedDemoResources = std::make_unique<SharedDemoResources>();

		mSharedDemoResources->TestTexture1 = ObjInit::texture(mSharedDemoResources->TestTexturePath);
		mSharedDemoResources->TestTexture2 = ObjInit::texture(mSharedDemoResources->TestTexture2Path);

		mSharedDemoResources->TexturedShaderProgram = ObjInit::shaderProgram(
			ObjInit::shader(EShaderType::VERTEX, SharedDemoResources::TexturedShaderVertPath),
			ObjInit::shader(EShaderType::FRAGMENT, SharedDemoResources::TexturedShaderFragPath)
		);

		ShaderUniformLayout& customLayout = mSharedDemoResources->TexturedShaderProgram->getUniformLayout();
		customLayout.setValue(0, sizeof(CustomDemo::UniformBufferObject));
		customLayout.setTexture(1, *mSharedDemoResources->TestTexture1);

		mSharedDemoResources->TexturedPipelineInfo = std::make_unique<GraphicsPipelineInstanceInfo>(
			StaticVertexInputLayout::get(SharedDemoResources::TexturedVertexType),
			*mSharedDemoResources->TexturedShaderProgram,
			ERenderPass::APP_SCENE
		);
		mSharedDemoResources->TexturedPipelineInfo->getOptionalFields().setDepthTesting(true, VK_COMPARE_OP_LESS);

		RenderingContextVulkan& context = Application::get().getRenderer().getContext();
		mSharedDemoResources->TexturedConveyor = context.createPipeline(
			mSharedDemoResources->TexturedPipelineName,
			*mSharedDemoResources->TexturedPipelineInfo
		);

		mSharedDemoResources->GpuResourcesLoaded = true;
	}

	void DemoLiciousAppLogic::closeSharedResources() {
		if (mCustomDemo != nullptr || mObjModelsDemo != nullptr) {
			LOG_WARN("All demos using shared resources must be closed before closing shared resources");
		} else if (mSharedDemoResources->GpuResourcesLoaded) {
			RendererVulkan& renderer = GET_RENDERER;
			renderer.closeGpuResource(mSharedDemoResources->TexturedShaderProgram);
			renderer.getContext().closePipeline(
				mSharedDemoResources->TexturedPipelineInfo->getRenderPass(),
				SharedDemoResources::TexturedPipelineName
			);
			renderer.closeGpuResource(mSharedDemoResources->TestTexture1);
			renderer.closeGpuResource(mSharedDemoResources->TestTexture2);

			mSharedDemoResources->GpuResourcesLoaded = false;
		}
	}

	void DemoLiciousAppLogic::ObjModelsDemo::init() {
		for (const auto& filePath : ObjModelFilePaths) {
			LoadedModels.emplace_back(ModelVulkan::createModel(filePath, ColouredVertexInputLayout));
		}
	
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
					transform
				));
			}
		}

		//for (uint32_t i = 0; i < 100; i++) {
		//	objModelsDemoAddRandomisedObject();
		//}
	
		SceneShaderProgram = ObjInit::shaderProgram(
			ObjInit::shader(
				EShaderType::VERTEX,
				FlatColourShaderVertPath
			),
			ObjInit::shader(
				EShaderType::FRAGMENT,
				FlatColourShaderFragPath
			)
		);
		ShaderUniformLayout& cubeLayout = SceneShaderProgram->getUniformLayout();
		cubeLayout.setValue(0, sizeof(ObjModelsDemo::UniformBufferObject));
		//Push constant for transformation matrix
		cubeLayout.addPushConstant(VK_SHADER_STAGE_VERTEX_BIT, sizeof(glm::mat4x4));
	
		ScenePipelineInfo = std::make_unique<GraphicsPipelineInstanceInfo>(
			ColouredVertexInputLayout,
			*SceneShaderProgram,
			ERenderPass::APP_SCENE
		);
		ScenePipelineInfo->getOptionalFields().setDepthTesting(true, VK_COMPARE_OP_LESS);

		//TODO:: wireframe for each vertex type
		SceneWireframePipelineInfo = std::make_unique<GraphicsPipelineInstanceInfo>(
			ColouredVertexInputLayout,
			*SceneShaderProgram,
			ERenderPass::APP_SCENE
		);
		SceneWireframePipelineInfo->getOptionalFields().setDepthTesting(true, VK_COMPARE_OP_LESS);
		SceneWireframePipelineInfo->getOptionalFields().CullMode = VK_CULL_MODE_NONE;
		SceneWireframePipelineInfo->getOptionalFields().PolygonMode = VK_POLYGON_MODE_LINE;
	
		auto& context = Application::get().getRenderer().getContext();
		ScenePipelineConveyor = context.createPipeline(ScenePipelineName, *ScenePipelineInfo);
		WireframePipelineConveyor = context.createPipeline(
			SceneWireframePipelineName,
			*SceneWireframePipelineInfo
		);

		TexturedModel = ModelVulkan::createModel("Dough/res/models/textured_cube.obj", TexturedVertexInputLayout);
		RenderableTexturedModel = std::make_shared<RenderableModelVulkan>("TexturedObjModel", TexturedModel);

		GpuResourcesLoaded = true;
	}

	void DemoLiciousAppLogic::ObjModelsDemo::close() {
		RendererVulkan& renderer = GET_RENDERER;
		for (const auto& model : LoadedModels) {
			renderer.closeGpuResource(model);
		}

		renderer.closeGpuResource(TexturedModel);
		LoadedModels.clear();
		RenderableObjects.clear();
		renderer.closeGpuResource(SceneShaderProgram);

		auto& context = renderer.getContext();
		context.closePipeline(ScenePipelineInfo->getRenderPass(), ScenePipelineName);
		context.closePipeline(SceneWireframePipelineInfo->getRenderPass(), SceneWireframePipelineName);

		GpuResourcesLoaded = false;
	}

	void DemoLiciousAppLogic::ObjModelsDemo::renderImGuiMainTab() {
		if (ImGui::BeginTabItem("Obj Models")) {
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

			ImGui::EndTabItem();
		}
	}

	void DemoLiciousAppLogic::ObjModelsDemo::renderImGuiExtras() {
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
		std::shared_ptr<TransformationData> transform = std::make_shared<TransformationData>(
			glm::vec3((x + posPadding) * 3.0f, (y + posPadding) * 3.0f, (z + posPadding) * 3.0f),
			glm::vec3(yaw, pitch, roll),
			scale
		);
		transform->updateTranslationMatrix();
	
		RenderableObjects.emplace_back(std::make_shared<RenderableModelVulkan>(
			ObjModelFilePaths[modelIndex],
			LoadedModels[modelIndex],
			transform
		));
	}

	void DemoLiciousAppLogic::ObjModelsDemo::imGuiDrawObjDemoItem(DOH::RenderableModelVulkan& model, const std::string& uniqueImGuiId) {
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

	void DemoLiciousAppLogic::CustomDemo::init() {
		//for (int i = 0; i < 8; i++) {
		//	std::string path = testTexturesPath + "texture" + std::to_string(i) + ".png";
		//	std::shared_ptr<TextureVulkan> testTexture = ObjInit::texture(path);
		//	mTestTextures.push_back(testTexture);
		//}

		SceneVao = ObjInit::vertexArray();
		std::shared_ptr<VertexBufferVulkan> sceneVb = ObjInit::stagedVertexBuffer(
			SceneVertexInputLayout,
			SceneVertices.data(),
			static_cast<size_t>(SceneVertexInputLayout.getStride()) * SceneVertices.size(),
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		);
		SceneVao->addVertexBuffer(sceneVb);
		std::shared_ptr<IndexBufferVulkan> sceneIb = ObjInit::stagedIndexBuffer(
			Indices.data(),
			sizeof(uint32_t) * Indices.size()
		);
		SceneVao->setDrawCount(static_cast<uint32_t>(Indices.size()));
		SceneVao->setIndexBuffer(sceneIb);
		SceneRenderable = std::make_shared<SimpleRenderable>(SceneVao);
	
		UiShaderProgram = ObjInit::shaderProgram(
			ObjInit::shader(EShaderType::VERTEX, UiShaderVertPath),
			ObjInit::shader(EShaderType::FRAGMENT, UiShaderFragPath)
		);
		UiShaderProgram->getUniformLayout().setValue(0, sizeof(CustomDemo::UniformBufferObject));
	
		UiVao = ObjInit::vertexArray();
		std::shared_ptr<VertexBufferVulkan> appUiVb = ObjInit::stagedVertexBuffer(
			UiVertexInputLayout,
			UiVertices.data(),
			static_cast<size_t>(UiVertexInputLayout.getStride()) * UiVertices.size(),
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		);
		UiVao->addVertexBuffer(appUiVb);
		std::shared_ptr<IndexBufferVulkan> appUiIb = ObjInit::stagedIndexBuffer(
			UiIndices.data(),
			sizeof(UiIndices[0]) * UiIndices.size()
		);
		UiVao->setDrawCount(static_cast<uint32_t>(UiIndices.size()));
		UiVao->setIndexBuffer(appUiIb);
		UiRenderable = std::make_shared<SimpleRenderable>(UiVao);
	
		UiPipelineInfo = std::make_unique<GraphicsPipelineInstanceInfo>(
			UiVertexInputLayout,
			*UiShaderProgram,
			ERenderPass::APP_UI
		);
	
		RenderingContextVulkan& context = Application::get().getRenderer().getContext();
		CustomUiConveyor = context.createPipeline(UiPipelineName, *UiPipelineInfo);

		GpuResourcesLoaded = true;
	}

	void DemoLiciousAppLogic::CustomDemo::close() {
		RenderScene = false;
		RenderUi = false;
		Update = false;

		RendererVulkan& renderer = GET_RENDERER;

		renderer.closeGpuResource(SceneVao);
		renderer.closeGpuResource(UiShaderProgram);
		renderer.closeGpuResource(UiVao);
		renderer.getContext().closePipeline(UiPipelineInfo->getRenderPass(), UiPipelineName);

		GpuResourcesLoaded = false;
	}

	void DemoLiciousAppLogic::CustomDemo::renderImGuiMainTab() {
		if (ImGui::BeginTabItem("Custom")) {
			ImGui::Checkbox("Render Scene", &RenderScene);
			ImGui::Checkbox("Render UI", &RenderUi);
			ImGui::Checkbox("Update", &Update);

			ImGui::EndTabItem();
		}
	}

	void DemoLiciousAppLogic::CustomDemo::renderImGuiExtras() {
		//No extra windows required for this demo
	}

	void DemoLiciousAppLogic::GridDemo::init() {
		TestGridMaxQuadCount = Renderer2dStorageVulkan::MAX_BATCH_COUNT_QUAD * Renderer2dStorageVulkan::BATCH_MAX_GEO_COUNT_QUAD;
	}

	void DemoLiciousAppLogic::GridDemo::close() {
		TexturedTestGrid.clear();
	}

	void DemoLiciousAppLogic::GridDemo::renderImGuiMainTab() {
		if (ImGui::BeginTabItem("Grid")) {
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

			MonoSpaceTextureAtlas& atlas = *GET_RENDERER.getContext().getRenderer2d().getStorage().getTestMonoSpaceTextureAtlas();

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

			ImGui::EndTabItem();
		}
	}

	void DemoLiciousAppLogic::GridDemo::renderImGuiExtras() {
		//No extra windows required for this demo
	}

	void DemoLiciousAppLogic::BouncingQuadDemo::init() {
		addRandomQuads(5000);
	}

	void DemoLiciousAppLogic::BouncingQuadDemo::close() {
		BouncingQuads.clear();
		BouncingQuadVelocities.clear();
	}


	void DemoLiciousAppLogic::BouncingQuadDemo::renderImGuiMainTab() {
		if (ImGui::BeginTabItem("Bouncing Quads")) {
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

			ImGui::EndTabItem();
		}
	}

	void DemoLiciousAppLogic::BouncingQuadDemo::renderImGuiExtras() {
		//No extra windows required for this demo
	}

	void DemoLiciousAppLogic::BouncingQuadDemo::addRandomQuads(size_t count) {
		const auto& atlas = GET_RENDERER.getContext().getRenderer2d().getStorage().getTestMonoSpaceTextureAtlas();
	
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

	void DemoLiciousAppLogic::BouncingQuadDemo::popQuads(size_t count) {
		const size_t size = BouncingQuads.size();
		if (count > size) {
			count = size;
		}

		for (int i = 0; i < count; i++) {
			BouncingQuads.pop_back();
			BouncingQuadVelocities.pop_back();
		}
	}

	void DemoLiciousAppLogic::TextDemo::init() {
		const auto& renderer2d = GET_RENDERER.getContext().getRenderer2d();
		//Generate quads for default message
		Text = std::make_unique<TextString>(
			StringBuffer,
			renderer2d.getStorage().getFontBitmap("Arial")
		);
	}

	void DemoLiciousAppLogic::TextDemo::close() {
		Text.release();
	}

	void DemoLiciousAppLogic::TextDemo::renderImGuiMainTab() {
		if (ImGui::BeginTabItem("Text")) {
			ImGui::Checkbox("Render", &Render);

			ImGui::Text("String length limit: %i", TextDemo::StringLengthLimit);
			EditorGui::displayHelpTooltip(
				R"(Larger strings can be displayed as the text renderer uses a Quad batch of size 10,000 (by default). )"
				R"(The limitation is because ImGui InputText field requires extra implementation for dynamic data on the heap.)"
			);

			if (ImGui::InputTextMultiline("Display Text", StringBuffer, sizeof(StringBuffer))) {
				Text->setString(StringBuffer);
			}

			float tempScale = Text->getScale();
			if (ImGui::DragFloat("Text Scale", &tempScale, 0.05f, 0.05f, 5.0f)) {
				Text->setScale(tempScale);
			}

			if (ImGui::ColorEdit4("String Colour", &Colour.x)) {
				Text->setColour(Colour);
			}

			ImGui::EndTabItem();
		}
	}

	void DemoLiciousAppLogic::TextDemo::renderImGuiExtras() {
		//No extra windows required for this demo
	}

	void DemoLiciousAppLogic::LineDemo::init() {
		LineData2d = {};
		LineData3d = {};
		LineDataInput[0] = 0.0f;
		LineDataInput[1] = 0.0f;
		LineDataInput[2] = 0.0f;
		LineDataInput[3] = 0.0f;
		LineDataInput[4] = 0.0f;
		LineDataInput[5] = 0.0f;
		LineDataInput[6] = 0.0f;
		LineDataInput[7] = 0.0f;
		LineDataInput[8] = 0.0f;
		LineDataInput[9] = 1.0f;
	}

	void DemoLiciousAppLogic::LineDemo::close() {
		LineData2d.clear();
		LineData3d.clear();
	}

	void DemoLiciousAppLogic::LineDemo::renderImGuiMainTab() {
		if (ImGui::BeginTabItem("Line")) {
			ImGui::Checkbox("Render", &Render);
			ImGui::Checkbox("Render Text Demo Outlines", &RenderTextDemoOutlines);
			ImGui::Checkbox("Render Ui Quad", &RenderUiQuad);

			auto& lineRenderer = GET_RENDERER.getContext().getLineRenderer();

			ImGui::Text("Scene Line Count: %i", lineRenderer.getSceneLineCount());
			EditorGui::displayHelpTooltip("This includes lines created from \"Add Line Scene\" and from any draw{ Primitive }3d() function calls.");
			ImGui::Text("UI Line Count: %i", lineRenderer.getUiLineCount());
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

			ImGui::EndTabItem();
		}
	}

	void DemoLiciousAppLogic::LineDemo::renderImGuiExtras() {
		//No extra windows required for this demo
	}

	void DemoLiciousAppLogic::LineDemo::addLine2d(glm::vec2 start, glm::vec2 end, glm::vec4 colour) {
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
		BoundingBox = std::make_unique<BoundingBox2d>();
	}

	void DemoLiciousAppLogic::BoundingBoxDemo::close() {
		BoundingBox->reset();
		Quads.clear();
	}

	void DemoLiciousAppLogic::BoundingBoxDemo::renderImGuiMainTab() {
		if (ImGui::BeginTabItem("Bounding Box")) {
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

			ImGui::EndTabItem();
		}
	}

	void DemoLiciousAppLogic::BoundingBoxDemo::renderImGuiExtras()  {

	}

	void DemoLiciousAppLogic::TileMapDemo::init() {
		//NOTE:: Texture atlas is created in QuadBatch init because rebiding descriptors not currently available.

		const std::shared_ptr<IndexedTextureAtlas> texAtlas = GET_RENDERER.getContext().getRenderer2d().getStorage().getTestIndexedTextureAtlas();

		SceneTileMap = std::make_unique<TileMap>(*texAtlas, 50, 50);

		PreviewAnimationController = std::make_unique<TextureAtlasAnimationController>(texAtlas->getAnimation("testAnim"));

		PreviewQuad = { { 0.0f, -1.0f, 1.0f }, { 2.0f, 2.0f }, { 1.f, 1.0f, 1.0f, 1.0f }, 0.0f, texAtlas };
		PreviewQuad.setTexture(*texAtlas);

		AnimatedQuad = { { -2.0f, -1.0f, 1.0f }, { 2.0f, 2.0f }, { 1.f, 1.0f, 1.0f, 1.0f }, 0.0f, texAtlas, PreviewAnimationController->getCurrentInnerTexture().TextureCoords };
		AnimatedQuad.setTexture(*texAtlas);
	}

	void DemoLiciousAppLogic::TileMapDemo::close() {
		
	}

	void DemoLiciousAppLogic::TileMapDemo::renderImGuiMainTab() {
		if (ImGui::BeginTabItem("Tile Map")) {
			ImGui::Checkbox("Render", &Render);
			ImGui::Checkbox("Update", &Update);
			ImGui::Checkbox("Render Preview Quad", &RenderPreviewQuad);

			if (ImGui::Button("View Texture Atlas")) {
				EditorGui::openIndexedTextureAtlasViewerWindow(*GET_RENDERER.getContext().getRenderer2d().getStorage().getTestIndexedTextureAtlas());
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
					PreviewQuad.TextureCoords = innerTexture.second.TextureCoords;
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
				AnimatedQuad.TextureCoords = PreviewAnimationController->getCurrentInnerTexture().TextureCoords;
			}

			ImGui::EndTabItem();
		}
	}

	void DemoLiciousAppLogic::TileMapDemo::renderImGuiExtras() {

	}
}
