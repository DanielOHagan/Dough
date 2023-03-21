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

		mInputLayer = std::make_shared<DefaultInputLayer>();
		Input::addInputLayer("InnerApp", mInputLayer);

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
		//if (mObjModelsDemo.Update) {
		//	for (const auto& obj : mObjModelsDemo.mRenderableObjects) {
		//		obj->Transformation->updateTranslationMatrix();
		//	}
		//}
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

		if (mObjModelsDemo->Render) {
			if (mObjModelsDemo->ScenePipelineConveyor.isValid() && mObjModelsDemo->WireframePipelineConveyor.isValid()) {
				for (auto& obj : mObjModelsDemo->RenderableObjects) {
					if (obj->Render || mObjModelsDemo->RenderAllStandard) {
						mObjModelsDemo->ScenePipelineConveyor.addRenderable(obj);
					}
					if (obj->RenderWireframe || mObjModelsDemo->RenderAllWireframe) {
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
			renderer2d.drawTextString(*mTextDemo->TextString);
		}

		renderer.endScene();

		renderer.beginUi(mCustomDemo->UiProjMat);
		if (mCustomDemo->RenderUi) {
			mCustomDemo->CustomUiConveyor.safeAddRenderable(mCustomDemo->UiRenderable);
		}
		renderer.endUi();
	}

	void DemoLiciousAppLogic::imGuiRender(float delta) {
		RendererVulkan& renderer = GET_RENDERER;
		Renderer2dVulkan& renderer2d = renderer.getContext().getRenderer2d();

		if (ImGui::Begin("DemoLicious Demos")) {
			ImGui::Text("Demo Settings & Info:");

			ImGui::BeginTabBar("Demo Tab Bar");
			if (ImGui::BeginTabItem("Grid")) {
				ImGui::Checkbox("Render", &mGridDemo->Render);
				ImGui::Checkbox("Update", &mGridDemo->Update);
				ImGui::Checkbox("Draw Colour", &mGridDemo->QuadDrawColour);
				ImGui::Text("Grid Quad Count: %i of Max %i", mGridDemo->TestGridSize[0] * mGridDemo->TestGridSize[1], mGridDemo->TestGridMaxQuadCount);
				int tempTestGridSize[2] = { mGridDemo->TestGridSize[0], mGridDemo->TestGridSize[1] };
				if (ImGui::InputInt2("Grid Size", tempTestGridSize)) {
					if (tempTestGridSize[0] > 0 && tempTestGridSize[1] > 0) {
						const int tempGridQuadCount = tempTestGridSize[0] * tempTestGridSize[1];
						if (tempGridQuadCount <= mGridDemo->TestGridMaxQuadCount) {
							mGridDemo->TestGridSize[0] = tempTestGridSize[0];
							mGridDemo->TestGridSize[1] = tempTestGridSize[1];
							mGridDemo->IsUpToDate = false;
						} else {
							LOG_WARN(
								"New grid size of " << tempTestGridSize[0] << "x" << tempTestGridSize[1] <<
								" (" << tempTestGridSize[0] * tempTestGridSize[1] <<
								") is too large, max quad count is " << mGridDemo->TestGridMaxQuadCount
							);
						}
					}
				}
				if (ImGui::DragFloat2("Quad Size", mGridDemo->TestGridQuadSize, 0.001f, 0.01f, 0.5f)) {
					mGridDemo->IsUpToDate = false;
				}
				if (ImGui::DragFloat2("Quad Gap Size", mGridDemo->TestGridQuadGapSize, 0.001f, 0.01f, 0.5f)) {
					mGridDemo->IsUpToDate = false;
				}
				//ImGui::DragFloat2("Test Grid Origin Pos", );
				//ImGui::Text("UI Quad Count: %i", renderer.getContext().getRenderer2d().getStorage().getUiQuadCount());

				//TOOD:: maybe have radio buttons for RenderStaticGrid or RenderDynamicGrid,
				//	static being the default values and dynamic being from the variables determined by ths menu
				// Maybe have the dynamic settings hidden unless dynamic is selected

				MonoSpaceTextureAtlas& atlas = *renderer.getContext().getRenderer2d().getStorage().getTestTextureAtlas();

				int tempTestTextureRowOffset = mGridDemo->TestTexturesRowOffset;
				if (ImGui::InputInt("Test Texture Row Offset", &tempTestTextureRowOffset)) {
					mGridDemo->TestTexturesRowOffset = tempTestTextureRowOffset < 0 ?
						0 : tempTestTextureRowOffset % atlas.getRowCount();
					mGridDemo->IsUpToDate = false;
				}
				int tempTestTextureColOffset = mGridDemo->TestTexturesColumnOffset;
				if (ImGui::InputInt("Test Texture Col Offset", &tempTestTextureColOffset)) {
					mGridDemo->TestTexturesColumnOffset = tempTestTextureColOffset < 0 ?
						0 : tempTestTextureColOffset % atlas.getColCount();
					mGridDemo->IsUpToDate = false;
				}

				if (ImGui::ColorEdit4("Grid Colour", &mGridDemo->QuadColour.x)) {
					mGridDemo->IsUpToDate = false;
				}

				if (ImGui::Button("Reset Grid")) {
					mGridDemo->TestGridSize[0] = 10;
					mGridDemo->TestGridSize[1] = 10;
					mGridDemo->TestGridQuadSize[0] = 0.1f;
					mGridDemo->TestGridQuadSize[1] = 0.1f;
					mGridDemo->TestGridQuadGapSize[0] = mGridDemo->TestGridQuadSize[0] * 1.5f;
					mGridDemo->TestGridQuadGapSize[1] = mGridDemo->TestGridQuadSize[1] * 1.5f;
					mGridDemo->IsUpToDate = false;
				}

				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Bouncing Quads")) {
				ImGui::Checkbox("Render", &mBouncingQuadDemo->Render);
				ImGui::Checkbox("Update", &mBouncingQuadDemo->Update);
				ImGui::Checkbox("Draw Colour", &mBouncingQuadDemo->QuadDrawColour);
				ImGui::Text("Bouncing Quads Count: %i", mBouncingQuadDemo->BouncingQuads.size());
				auto& demo = mBouncingQuadDemo;
				if (ImGui::InputInt((std::string(ImGuiWrapper::EMPTY_LABEL) + "Add").c_str(), &demo->AddNewQuadCount, 5, 5)) {
					if (demo->AddNewQuadCount < 0) {
						demo->AddNewQuadCount = 0;
					} else if (demo->AddNewQuadCount > 1000) {
						demo->AddNewQuadCount = 1000;
					}
				}
				ImGui::SameLine();
				if (ImGui::Button("Add Quads")) {
					bouncingQuadsDemoAddRandomQuads(demo->AddNewQuadCount);
				}
				if (ImGui::InputInt((std::string(ImGuiWrapper::EMPTY_LABEL) + "Pop").c_str(), &demo->PopQuadCount, 5, 5)) {
					if (demo->PopQuadCount < 0) {
						demo->PopQuadCount = 0;
					}
				}
				ImGui::SameLine();
				if (ImGui::Button("Pop Quads")) {
					bouncingQaudsDemoPopQuads(demo->PopQuadCount);
				}
				if (ImGui::Button("Clear Quads")) {
					bouncingQaudsDemoPopQuads(static_cast<int>(demo->BouncingQuads.size()));
				}

				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Custom")) {
				ImGui::Checkbox("Render Scene", &mCustomDemo->RenderScene);
				ImGui::Checkbox("Render UI", &mCustomDemo->RenderUi);
				ImGui::Checkbox("Update", &mCustomDemo->Update);

				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Obj Models")) {
				auto& renderables = mObjModelsDemo->RenderableObjects;
				auto& demo = mObjModelsDemo;
				static const std::string addLabel = std::string(ImGuiWrapper::EMPTY_LABEL) + "Add";
				static const std::string popLabel = std::string(ImGuiWrapper::EMPTY_LABEL) + "Pop";

				ImGui::Checkbox("Render", &demo->Render);
				ImGui::Checkbox("Update", &demo->Update);
				ImGui::Text("Object Count: %i", renderables.size());
				if (ImGui::InputInt(addLabel.c_str(), &demo->AddNewObjectsCount, 5, 5)) {
					if (demo->AddNewObjectsCount < 0) {
						demo->AddNewObjectsCount = 0;
					} else if (demo->AddNewObjectsCount > 1000) {
						demo->AddNewObjectsCount = 1000;
					}
				}
				ImGui::SameLine();
				if (ImGui::Button("Add Object")) {
					for (int i = 0; i < demo->AddNewObjectsCount; i++) {
						objModelsDemoAddRandomisedObject();
					}
				}
				if (ImGui::InputInt(popLabel.c_str(), &demo->PopObjectsCount, 5, 5)) {
					if (demo->PopObjectsCount < 0) {
						demo->PopObjectsCount = 0;
					}
				}
				ImGui::SameLine();
				if (ImGui::Button("Pop Object")) {
					//Max out pop count to renderables list size
					const int size = static_cast<int>(demo->RenderableObjects.size());
					const int popCount = demo->PopObjectsCount > size ? size : demo->PopObjectsCount;

					for (int i = 0; i < popCount; i++) {
						demo->RenderableObjects.pop_back();
					}
				}
				ImGui::Checkbox("Display Renderable Models List", &mImGuiSettings->RenderObjModelsList);
				if (ImGui::Button("Clear Objects")) {
					renderables.clear();
				}

				ImGui::Checkbox("Render Textured Model", &mObjModelsDemo->RenderableTexturedModel->Render);
				//TODO:: TexturedModel Wireframe rendering not currently supported
				//ImGui::Checkbox("Render Wireframe Textured Model", &mObjModelsDemo->RenderableTexturedModel->RenderWireframe);

				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Text")) {
				ImGui::Checkbox("Render", &mTextDemo->Render);

				ImGui::Text("String length limit: %i", TextDemo::StringLengthLimit);
				EditorGui::displayHelpTooltip(
					R"(Larger strings can be displayed as the text renderer uses a Quad batch of size 10,000 (by default). )"
					R"(The limitation is because ImGui InputText field requires extra implementation for dynamic data on the heap.)"
				);

				if (ImGui::InputTextMultiline("Display Text", mTextDemo->StringBuffer, sizeof(mTextDemo->StringBuffer))) {
					mTextDemo->TextString->setString(mTextDemo->StringBuffer);
				}

				float tempScale = mTextDemo->TextString->getScale();
				if (ImGui::DragFloat("Text Scale", &tempScale, 0.05f, 0.05f, 5.0f)) {
					mTextDemo->TextString->setScale(tempScale);
				}

				if (ImGui::ColorEdit4("String Colour", &mTextDemo->Colour.x)) {
					mTextDemo->TextString->setColour(mTextDemo->Colour);
				}

				ImGui::EndTabItem();
			}

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
			}
			if (ImGui::Button("View atlas texture")) {
				EditorGui::openMonoSpaceTextureAtlasViewerWindow(*renderer2d.getStorage().getTestTextureAtlas());
			}
			EditorGui::displayHelpTooltip("Display Texture Altas texture inside a separate window.");
			if (ImGui::Button("View test texture")) {
				EditorGui::openTextureViewerWindow(*mSharedDemoResources->TestTexture1);
			}
			EditorGui::displayHelpTooltip("Display test texture inside a separate window.");
		}
		ImGui::End();


		if (mImGuiSettings->RenderObjModelsList) {
			if (ImGui::Begin("Renderable Models List", &mImGuiSettings->RenderObjModelsList)) {
				const auto& renderables = mObjModelsDemo->RenderableObjects;
				ImGui::Checkbox("Render All", &mObjModelsDemo->RenderAllStandard);
				ImGui::SameLine();
				ImGui::Checkbox("Render All Wireframe", &mObjModelsDemo->RenderAllWireframe);

				//Render list of renderable OBJ models as a tree separating all into groups of 10 based on index
				const int size = static_cast<int>(renderables.size());
				if (size > 0) {
					ImGui::TreePush();
					ImGui::Unindent();

					int objIndex = 0;
					const int objectsPerNode = 10;
					for (int nodeIterator = objIndex; nodeIterator < size; nodeIterator += objectsPerNode) {
						if (ImGui::TreeNode(std::to_string(objIndex).c_str())) {
							for (int i = objIndex; i < nodeIterator + objectsPerNode && i < size; i++) {
								std::string uniqueImGuiId = "##" + std::to_string(i);
								imGuiDrawObjDemoItem(*renderables[i], uniqueImGuiId);

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

	void DemoLiciousAppLogic::close() {
		RendererVulkan& renderer = GET_RENDERER;

		//Custom demo
		renderer.closeGpuResource(mCustomDemo->SceneVao);
		renderer.closeGpuResource(mCustomDemo->UiShaderProgram);
		renderer.closeGpuResource(mCustomDemo->UiVao);

		//Obj Models Demo
		for (const auto& model : mObjModelsDemo->LoadedModels) {
			renderer.closeGpuResource(model);
		}
		renderer.closeGpuResource(mObjModelsDemo->TexturedModel);
		mObjModelsDemo->LoadedModels.clear();
		mObjModelsDemo->RenderableObjects.clear();
		renderer.closeGpuResource(mObjModelsDemo->SceneShaderProgram);

		//Shared resources
		renderer.closeGpuResource(mSharedDemoResources->TexturedShaderProgram);
		renderer.closeGpuResource(mSharedDemoResources->TestTexture1);
		renderer.closeGpuResource(mSharedDemoResources->TestTexture2);
	}

	void DemoLiciousAppLogic::onResize(float aspectRatio) {
		setUiProjection(aspectRatio);
	}

	void DemoLiciousAppLogic::imGuiDrawObjDemoItem(DOH::RenderableModelVulkan& model, const std::string& uniqueImGuiId) {
		if (ImGui::BeginCombo(("Obj Model" + uniqueImGuiId).c_str(), model.getName().c_str())) {
			int modelFilePathIndex = -1;

			for (const auto& filePath : mObjModelsDemo->ObjModelFilePaths) {
				bool selected = false;
				modelFilePathIndex++;

				std::string label = filePath + uniqueImGuiId;

				if (ImGui::Selectable(label.c_str(), &selected)) {
					model.Model = mObjModelsDemo->LoadedModels[modelFilePathIndex];
					model.Name = mObjModelsDemo->ObjModelFilePaths[modelFilePathIndex];
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

		if (transformed && mObjModelsDemo->Update) {
			transformation.updateTranslationMatrix();
		}
	}

	void DemoLiciousAppLogic::initDemos() {
		RenderingContextVulkan& context = GET_RENDERER.getContext();
	
		//Test grid is repopulated per update to apply changes from editor. To only populate once remove re-population from update and populate here.
		//populateTestGrid(static_cast<uint32_t>(mGridDemo.TestGridSize[0]), static_cast<uint32_t>(mGridDemo.TestGridSize[1]));

		initSharedResources();

		mGridDemo = std::make_unique<GridDemo>();
		mGridDemo->TestGridMaxQuadCount = Renderer2dStorageVulkan::MAX_BATCH_COUNT_QUAD * Renderer2dStorageVulkan::BATCH_MAX_GEO_COUNT_QUAD;

		initBouncingQuadsDemo();
		initCustomDemo();
		initObjModelsDemo();
		initTextDemo();

		context.createPipelineUniformObjects();
	}
	
	void DemoLiciousAppLogic::populateTestGrid(uint32_t width, uint32_t height) {
		const auto& storage = GET_RENDERER.getContext().getRenderer2d().getStorage();
		const auto& atlas = storage.getTestTextureAtlas();
	
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
			SharedDemoResources::TexturedVertexType,
			*mSharedDemoResources->TexturedShaderProgram,
			ERenderPass::APP_SCENE
		);
		mSharedDemoResources->TexturedPipelineInfo->getOptionalFields().setDepthTesting(true, VK_COMPARE_OP_LESS);

		RenderingContextVulkan& context = Application::get().getRenderer().getContext();
		mSharedDemoResources->TexturedConveyor = context.createPipeline(
			mSharedDemoResources->TexturedPipelineName,
			*mSharedDemoResources->TexturedPipelineInfo
		);
	}
	
	void DemoLiciousAppLogic::initBouncingQuadsDemo() {
		mBouncingQuadDemo = std::make_unique<BouncingQuadDemo>();

		bouncingQuadsDemoAddRandomQuads(5000);
	}
	
	void DemoLiciousAppLogic::initCustomDemo() {
		mCustomDemo = std::make_unique<CustomDemo>();

		//for (int i = 0; i < 8; i++) {
		//	std::string path = testTexturesPath + "texture" + std::to_string(i) + ".png";
		//	std::shared_ptr<TextureVulkan> testTexture = ObjInit::texture(path);
		//	mTestTextures.push_back(testTexture);
		//}
	
		mCustomDemo->SceneVao = ObjInit::vertexArray();
		std::shared_ptr<VertexBufferVulkan> sceneVb = ObjInit::stagedVertexBuffer(
			SharedDemoResources::TexturedVertexType,
			mCustomDemo->SceneVertices.data(),
			(size_t) SharedDemoResources::TexturedVertexType * mCustomDemo->SceneVertices.size(),
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		);
		mCustomDemo->SceneVao->addVertexBuffer(sceneVb);
		std::shared_ptr<IndexBufferVulkan> sceneIb = ObjInit::stagedIndexBuffer(
			mCustomDemo->Indices.data(),
			sizeof(uint32_t) * mCustomDemo->Indices.size()
		);
		mCustomDemo->SceneVao->setDrawCount(static_cast<uint32_t>(mCustomDemo->Indices.size()));
		mCustomDemo->SceneVao->setIndexBuffer(sceneIb);
		mCustomDemo->SceneRenderable = std::make_shared<SimpleRenderable>(mCustomDemo->SceneVao);
	
		mCustomDemo->UiShaderProgram = ObjInit::shaderProgram(
			ObjInit::shader(EShaderType::VERTEX, mCustomDemo->UiShaderVertPath),
			ObjInit::shader(EShaderType::FRAGMENT, mCustomDemo->UiShaderFragPath)
		);
		mCustomDemo->UiShaderProgram->getUniformLayout().setValue(0, sizeof(CustomDemo::UniformBufferObject));
	
		mCustomDemo->UiVao = ObjInit::vertexArray();
		std::shared_ptr<VertexBufferVulkan> appUiVb = ObjInit::stagedVertexBuffer(
			mCustomDemo->UiVertexType,
			mCustomDemo->UiVertices.data(),
			static_cast<size_t>(mCustomDemo->UiVertexType) * mCustomDemo->UiVertices.size(),
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		);
		mCustomDemo->UiVao->addVertexBuffer(appUiVb);
		std::shared_ptr<IndexBufferVulkan> appUiIb = ObjInit::stagedIndexBuffer(
			mCustomDemo->UiIndices.data(),
			sizeof(mCustomDemo->UiIndices[0]) * mCustomDemo->UiIndices.size()
		);
		mCustomDemo->UiVao->setDrawCount(static_cast<uint32_t>(mCustomDemo->UiIndices.size()));
		mCustomDemo->UiVao->setIndexBuffer(appUiIb);
		mCustomDemo->UiRenderable = std::make_shared<SimpleRenderable>(mCustomDemo->UiVao);
	
		mCustomDemo->UiPipelineInfo = std::make_unique<GraphicsPipelineInstanceInfo>(
			mCustomDemo->UiVertexType,
			*mCustomDemo->UiShaderProgram,
			ERenderPass::APP_UI
		);
	
		RenderingContextVulkan& context = Application::get().getRenderer().getContext();
		mCustomDemo->CustomUiConveyor = context.createPipeline(
			mCustomDemo->UiPipelineName,
			*mCustomDemo->UiPipelineInfo
		);
	}
	
	void DemoLiciousAppLogic::initObjModelsDemo() {
		mObjModelsDemo = std::make_unique<ObjModelsDemo>();

		for (const auto& filePath : mObjModelsDemo->ObjModelFilePaths) {
			mObjModelsDemo->LoadedModels.emplace_back(ModelVulkan::createModel(filePath, EVertexType::VERTEX_3D));
		}
	
		//Spwan objects on incrementing x/y values of a grid with random z value
		const float padding = 0.5f;
		for (uint32_t x = 0; x < 10; x++) {
			for (uint32_t y = 0; y < 10; y++) {
				//Add an object with a position based off of x & y value from loop, creates a grid like result
				const uint32_t modelIndex = rand() % mObjModelsDemo->LoadedModels.size();
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
				
				mObjModelsDemo->RenderableObjects.emplace_back(std::make_shared<RenderableModelVulkan>(
					mObjModelsDemo->ObjModelFilePaths[modelIndex],
					mObjModelsDemo->LoadedModels[modelIndex],
					transform
				));
			}
		}

		//for (uint32_t i = 0; i < 100; i++) {
		//	objModelsDemoAddRandomisedObject();
		//}
	
		mObjModelsDemo->SceneShaderProgram = ObjInit::shaderProgram(
			ObjInit::shader(
				EShaderType::VERTEX,
				mObjModelsDemo->FlatColourShaderVertPath
			),
			ObjInit::shader(
				EShaderType::FRAGMENT,
				mObjModelsDemo->FlatColourShaderFragPath
			)
		);
		ShaderUniformLayout& cubeLayout = mObjModelsDemo->SceneShaderProgram->getUniformLayout();
		cubeLayout.setValue(0, sizeof(ObjModelsDemo::UniformBufferObject));
		//Push constant for transformation matrix
		cubeLayout.addPushConstant(VK_SHADER_STAGE_VERTEX_BIT, sizeof(glm::mat4x4));
	
		mObjModelsDemo->ScenePipelineInfo = std::make_unique<GraphicsPipelineInstanceInfo>(
			mObjModelsDemo->FlatColourVertexType,
			*mObjModelsDemo->SceneShaderProgram,
			ERenderPass::APP_SCENE
		);
		mObjModelsDemo->ScenePipelineInfo->getOptionalFields().setDepthTesting(true, VK_COMPARE_OP_LESS);

		//TODO:: wireframe for each vertex type
		mObjModelsDemo->SceneWireframePipelineInfo = std::make_unique<GraphicsPipelineInstanceInfo>(
			mObjModelsDemo->FlatColourVertexType,
			*mObjModelsDemo->SceneShaderProgram,
			ERenderPass::APP_SCENE
		);
		mObjModelsDemo->SceneWireframePipelineInfo->getOptionalFields().setDepthTesting(true, VK_COMPARE_OP_LESS);
		mObjModelsDemo->SceneWireframePipelineInfo->getOptionalFields().CullMode = VK_CULL_MODE_NONE;
		mObjModelsDemo->SceneWireframePipelineInfo->getOptionalFields().PolygonMode = VK_POLYGON_MODE_LINE;
	
		auto& context = Application::get().getRenderer().getContext();
		mObjModelsDemo->ScenePipelineConveyor = context.createPipeline(
			mObjModelsDemo->ScenePipelineName,
			*mObjModelsDemo->ScenePipelineInfo
		);
		mObjModelsDemo->WireframePipelineConveyor = context.createPipeline(
			mObjModelsDemo->SceneWireframePipelineName,
			*mObjModelsDemo->SceneWireframePipelineInfo
		);

		mObjModelsDemo->TexturedModel = ModelVulkan::createModel("Dough/res/models/textured_cube.obj", EVertexType::VERTEX_3D_TEXTURED);
		mObjModelsDemo->RenderableTexturedModel = std::make_shared<RenderableModelVulkan>(
			"TexturedObjModel",
			mObjModelsDemo->TexturedModel
		);
	}
	
	void DemoLiciousAppLogic::initTextDemo() {
		mTextDemo = std::make_unique<TextDemo>();

		const auto& renderer2d = Application::get().getRenderer().getContext().getRenderer2d();
	
		//Set a defualt message for the text demo here
		
		//Generate quads for default message
		mTextDemo->TextString = std::make_unique<TextString>(
			mTextDemo->StringBuffer,
			renderer2d.getStorage().getFontBitmap("Arial")
		);
	}
	
	void DemoLiciousAppLogic::bouncingQuadsDemoAddRandomQuads(size_t count) {
		const auto& atlas = GET_RENDERER.getContext().getRenderer2d().getStorage().getTestTextureAtlas();
	
		//Stop quad count going over MaxCount
		if (count + mBouncingQuadDemo->BouncingQuads.size() > mBouncingQuadDemo->MaxBouncingQuadCount) {
			count = mBouncingQuadDemo->MaxBouncingQuadCount - mBouncingQuadDemo->BouncingQuads.size();
		}
	
		for (int i = 0; i < count; i++) {
			//Add quads using the texture atlas
			mBouncingQuadDemo->BouncingQuads.push_back({
				//Semi-random values for starting position, velocitiy and assigned texture
				{
					(static_cast<float>((rand() % 5000)) / 500.0f) - 1.0f,
					(static_cast<float>((rand() % 5000)) / 500.0f) - 1.0f,
					0.8f
				},
				{ mGridDemo->TestGridQuadSize[0], mGridDemo->TestGridQuadSize[1] },
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
	
			mBouncingQuadDemo->BouncingQuadVelocities.emplace_back(
				static_cast<float>((rand() % 800) / 60.0f),
				static_cast<float>((rand() % 800) / 60.0f)
			);
		}
	}
	
	void DemoLiciousAppLogic::bouncingQaudsDemoPopQuads(size_t count) {
		const size_t size = mBouncingQuadDemo->BouncingQuads.size();
		if (count > size) {
			count = size;
		}
	
		for (int i = 0; i < count; i++) {
			mBouncingQuadDemo->BouncingQuads.pop_back();
			mBouncingQuadDemo->BouncingQuadVelocities.pop_back();
		}
	}
	
	void DemoLiciousAppLogic::objModelsDemoAddObject(
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
	
		mObjModelsDemo->RenderableObjects.emplace_back(std::make_shared<RenderableModelVulkan>(
			mObjModelsDemo->ObjModelFilePaths[modelIndex],
			mObjModelsDemo->LoadedModels[modelIndex],
			transform
		));
	}
}
