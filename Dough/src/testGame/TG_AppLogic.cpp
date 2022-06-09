#include "testGame/TG_AppLogic.h"

#include "dough/rendering/ObjInit.h"
#include "dough/application/Application.h"
#include "dough/input/InputCodes.h"
#include "dough/Logging.h"

#include <imgui/imgui.h>

#define GET_RENDERER Application::get().getRenderer()

namespace TG {

	TG_AppLogic::TG_AppLogic()
	:	IApplicationLogic(),
		mSelectedDemo(EDemo::NONE)
	{}

	TG_AppLogic::TG_AppLogic(EDemo demo)
	:	IApplicationLogic(),
		mSelectedDemo(demo)
	{}

	void TG_AppLogic::init(float aspectRatio) {
		mGridDemo.mTestGridMaxQuadCount = Renderer2dStorageVulkan::MAX_BATCH_COUNT_QUAD * Renderer2dStorageVulkan::BATCH_MAX_GEO_COUNT_QUAD;

		initDemos();

		setUiProjection(aspectRatio);

		TG_mOrthoCameraController = std::make_shared<TG_OrthoCameraController>(aspectRatio);
		mPerspectiveCameraController = std::make_shared<TG_PerspectiveCameraController>(aspectRatio);

		mPerspectiveCameraController->setPosition({ 0.0f, 0.0f, 5.0f });
	}

	void TG_AppLogic::update(float delta) {
		//Handle input first
		if (Input::isKeyPressed(DOH_KEY_F1)) {
			mImGuiSettings.RenderDebugWindow = true;
		}

		if (mImGuiSettings.UseOrthographicCamera) {
			TG_mOrthoCameraController->onUpdate(delta);
		} else {
			mPerspectiveCameraController->onUpdate(delta);
		}

		if (mBouncingQuadDemo.Update) {
			const float translationDelta = 0.2f * delta;
			for (size_t i = 0; i < mBouncingQuadDemo.mBouncingQuads.size(); i++) {
				Quad& quad = mBouncingQuadDemo.mBouncingQuads[i];
				glm::vec2& velocity = mBouncingQuadDemo.mBouncingQuadVelocities[i];

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

		if (mGridDemo.Update) {
			//Repopulate entire array each update, not very efficient but the test grid is just an example
			populateTestGrid(mGridDemo.mTestGridSize[0], mGridDemo.mTestGridSize[1]);
		}

		if (mCubeDemo.Update) {
			if (mCubeDemo.AutoRotate) {
				mCubeDemo.Rotation[1] += mCubeDemo.AutoRotateSpeed * delta;

				if (mCubeDemo.Rotation[1] > 360.0f) {
					mCubeDemo.Rotation[1] = 0.0f;
				} else if (mCubeDemo.Rotation[1] < 0.0f) {
					mCubeDemo.Rotation[1] = 360.0f;
				}
			}

			//Update translation matrix
			mCubeDemo.mTranslation = glm::mat4x4(1.0f);
			mCubeDemo.mTranslation = glm::translate(
				mCubeDemo.mTranslation,
				{
					mCubeDemo.Position[0],
					mCubeDemo.Position[1],
					mCubeDemo.Position[2]
				}
			);
			if (mCubeDemo.Rotation[0] > 0.0f) {
				mCubeDemo.mTranslation = glm::rotate(
					mCubeDemo.mTranslation,
					glm::radians(mCubeDemo.Rotation[0]),
					{ 1.0f, 0.0f, 0.0f }
				);
			}
			if (mCubeDemo.Rotation[1] > 0.0f) {
				mCubeDemo.mTranslation = glm::rotate(
					mCubeDemo.mTranslation,
					glm::radians(mCubeDemo.Rotation[1]),
					{ 0.0f, 1.0f, 0.0f }
				);
			}
			if (mCubeDemo.Rotation[2] > 0.0f) {
				mCubeDemo.mTranslation = glm::rotate(
					mCubeDemo.mTranslation,
					glm::radians(mCubeDemo.Rotation[2]),
					{ 0.0f, 0.0f, 1.0f}
				);
			}

			mCubeDemo.mTranslation = glm::scale(
				mCubeDemo.mTranslation,
				{
					mCubeDemo.Scale,
					mCubeDemo.Scale,
					mCubeDemo.Scale
				}
			);
		}
	}

	void TG_AppLogic::render() {
		RendererVulkan& renderer = GET_RENDERER;
		Renderer2dVulkan& renderer2d = renderer.getContext().getRenderer2d();

		renderer.beginScene(
			mImGuiSettings.UseOrthographicCamera ?
				TG_mOrthoCameraController->getCamera() : mPerspectiveCameraController->getCamera()
		);

		if (mCustomDemo.RenderScene) {
			renderer.getContext().addVaoToSceneDrawList("Custom", *mCustomDemo.mSceneVertexArray);
		}

		if (mCubeDemo.Render) {
			renderer.getContext().addVaoToSceneDrawList("Cube", mCubeDemo.mCubeModel->getVao());
		}

		if (mGridDemo.Render) {
			for (std::vector<Quad>& sameTexturedQuads : mGridDemo.mTexturedTestGrid) {
				//Different ways of drawing quads (inserting into RenderBatches)

				//for (Quad& quad : sameTexturedQuads) {
				//	rrenderer2d.drawQuadScene(quad);
				//}
				//for (Quad& quad : sameTexturedQuads) {
				//	renderer2d.drawQuadTexturedScene(quad);
				//}
				//renderer2d.drawQuadArrayScene(sameTexturedQuads);
				//renderer2d.drawQuadArrayTexturedScene(sameTexturedQuads);
				renderer2d.drawQuadArraySameTextureScene(sameTexturedQuads);
			}
		}

		if (mBouncingQuadDemo.Render) {
			//for (Quad& quad : mBouncingQuadDemo.mBouncingQuads) {
			//	rrenderer2d.drawQuadScene(quad);
			//}
			for (Quad& quad : mBouncingQuadDemo.mBouncingQuads) {
				renderer2d.drawQuadTexturedScene(quad);
			}
			//renderer2d.drawQuadArrayScene(mBouncingQuadDemo.mBouncingQuads);
			//renderer2d.drawQuadArrayTexturedScene(mBouncingQuadDemo.mBouncingQuads);
		}

		renderer.endScene();

		renderer.beginUi(mCustomDemo.mUiProjMat);
		if (mCustomDemo.RenderUi) {
			//renderer.getContext().addVaoToUiDrawList(*mCustomDemo.mUiVao);
			renderer.getContext().addVaoToUiDrawList("CustomUi", *mCustomDemo.mUiVao);
		}
		renderer.endUi();
	}

	void TG_AppLogic::imGuiRender() {
		if (mImGuiSettings.RenderDebugWindow) {
			imGuiRenderDebugWindow();
		}
	}

	void TG_AppLogic::imGuiRenderDebugWindow() {
		//TODO:: Clean up basic layout and styling, maybe add a few imgui helper functions

		RendererVulkan& renderer = GET_RENDERER;

		ImGui::Begin("Debug Window");
		ImGui::BeginTabBar("Main Tab Bar");
		if (ImGui::BeginTabItem("Debug")) {

			ImGui::SetNextItemOpen(mImGuiSettings.ApplicationCollapseMenuOpen);
			if (ImGui::CollapsingHeader("Application")) {
				ApplicationLoop& loop = Application::get().getLoop();
				Window& window = Application::get().getWindow();
				bool focused = Application::get().isFocused();

				ImGui::Text("Runtime: %fs", Time::convertMillisToSeconds(Application::get().getAppInfoTimer().getCurrentTickingTimeMillis()));
				std::vector<std::string> monitorNames = window.getAllAvailableMonitorNames();
				if (ImGui::BeginCombo("Monitor", window.getSelectedMonitorName().c_str())) {
					int monitorNameIndex2 = -1;
					for (const std::string& name : monitorNames) {
						bool selected = false;
						ImGui::Selectable(name.c_str(), &selected);
						monitorNameIndex2++;
						if (selected) {
							window.selectMonitor(monitorNameIndex2);
							break;
						}
					}
					ImGui::EndCombo();
				}
				ImGui::Text("Window Size: (%i, %i)", window.getWidth(), window.getHeight());
				bool displayModeWindowActive = window.getDisplayMode() == EWindowDisplayMode::WINDOWED;
				if (
					ImGui::RadioButton("Windowed", displayModeWindowActive) &&
					window.getDisplayMode() != EWindowDisplayMode::WINDOWED
				) {
					window.selectDisplayMode(EWindowDisplayMode::WINDOWED);
				}
				ImGui::SameLine();
				bool displayModeBorderlessWindowActive = window.getDisplayMode() == EWindowDisplayMode::BORDERLESS_FULLSCREEN;
				if (
					ImGui::RadioButton("Borderless Windowed", displayModeBorderlessWindowActive) &&
					window.getDisplayMode() != EWindowDisplayMode::BORDERLESS_FULLSCREEN
				) {
					window.selectDisplayMode(EWindowDisplayMode::BORDERLESS_FULLSCREEN);
				}
				ImGui::SameLine();
				bool displayModeFullscreenActive = window.getDisplayMode() == EWindowDisplayMode::FULLSCREEN;
				if (
					ImGui::RadioButton("Fullscreen", displayModeFullscreenActive) &&
					window.getDisplayMode() != EWindowDisplayMode::FULLSCREEN
				) {
					window.selectDisplayMode(EWindowDisplayMode::FULLSCREEN);
				}
				ImGui::Text("Is Focused: ");
				ImGui::SameLine();
				ImGui::TextColored(focused ? ImVec4(0, 1, 0, 1) : ImVec4(1, 0, 0, 1), focused ? "FOCUSED" : "NOT FOCUSED");
				ImGui::Text("Current and Target FPS/UPS");
				ImGui::Text("FPS: %i \t(Fore: %i, Back: %i)", (int)loop.getFps(), (int)loop.getTargetFps(), (int)loop.getTargetBackgroundFps());
				imGuiDisplayHelpTooltip("Max: Current Target UPS or 360, whichever is lower. Min: 15\nFPS displayed is the count of frames in the last full second interval");
				ImGui::Text("UPS: %i \t(Fore: %i, Back: %i)", (int)loop.getUps(), (int)loop.getTargetUps(), (int)loop.getTargetBackgroundUps());
				imGuiDisplayHelpTooltip("Max: 1000. Min: Current Target FPS or 15, whichever is higher.\nUPS displayed is the count of frames in the last full second interval");
				int tempTargetFps = (int)loop.getTargetFps();
				if (ImGui::InputInt("Target FPS", &tempTargetFps)) {
					if (loop.isValidTargetFps((float)tempTargetFps)) {
						loop.setTargetFps((float)tempTargetFps);
					}
				}
				int tempTargetUps = (int)loop.getTargetUps();
				if (ImGui::InputInt("Target UPS", &tempTargetUps)) {
					if (loop.isValidTargetUps((float)tempTargetUps)) {
						loop.setTargetUps((float)tempTargetUps);
					}
				}

				{
					const glm::vec2 cursorPos = Input::getCursorPos();
					ImGui::Text("Cursor Position: X: %i, Y: %i", (int)cursorPos.x, (int)cursorPos.y);
				}

				bool runInBackground = loop.isRunningInBackground();
				if (ImGui::Checkbox("Run In Background", &runInBackground)) {
					loop.setRunInBackground(runInBackground);
				}
				if (ImGui::Button("Stop Rendering ImGui Windows")) {
					mImGuiSettings.RenderDebugWindow = false;
				}
				imGuiDisplayHelpTooltip("When hidden press F1 to start rendering ImGui again");

				if (ImGui::Button("Quit Application")) {
					Application::get().stop();
				}

				mImGuiSettings.ApplicationCollapseMenuOpen = true;
			} else {
			mImGuiSettings.ApplicationCollapseMenuOpen = false;
			}

			ImGui::SetNextItemOpen(mImGuiSettings.RenderingCollapseMenuOpen);
			if (ImGui::CollapsingHeader("Rendering")) {
				//TODO:: ImGui::Checkbox("Render Wireframes", &mRenderWireframes);
				const RenderingDebugInfo& renderInfo = renderer.getContext().getRenderingDebugInfo();
				ImGui::BeginTable("Draw Call Info", 2);
				ImGui::TableSetupColumn("Pipeline");
				ImGui::TableSetupColumn("Draw Call Count");
				ImGui::TableHeadersRow();
				ImGui::TableSetColumnIndex(0);
				imGuiPrintDrawCallTableColumn("Scene", renderInfo.SceneDrawCalls);
				imGuiPrintDrawCallTableColumn("UI", renderInfo.UiDrawCalls);
				imGuiPrintDrawCallTableColumn("Quad Batch", renderInfo.BatchRendererDrawCalls);
				imGuiPrintDrawCallTableColumn("Total", renderInfo.TotalDrawCalls);
				ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg1, ImColor::ImColor(128, 128, 255, 100));
				ImGui::EndTable();
				ImGui::Text("Quad Batch Max Size: %i", Renderer2dStorageVulkan::BatchSizeLimits::BATCH_MAX_GEO_COUNT_QUAD);
				ImGui::Text(
					"Quad Batch Count: %i of Max %i",
					renderer.getContext().getRenderer2d().getQuadBatchCount(),
					Renderer2dStorageVulkan::BatchSizeLimits::MAX_BATCH_COUNT_QUAD
				);
				uint32_t quadBatchIndex = 0;
				for (RenderBatchQuad& batch : renderer.getContext().getRenderer2d().getStorage().getQuadRenderBatches()) {
					ImGui::Text("Batch: %i Geo Count: %i", quadBatchIndex, batch.getGeometryCount());
					quadBatchIndex++;
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
					renderer.getContext().getRenderer2d().getStorage().getQuadBatchTextureArray().getTextureSlots().size()
				);
				if (ImGui::Button("Close All Empty Quad Batches")) {
					renderer.getContext().getRenderer2d().closeEmptyQuadBatches();
				}
				imGuiDisplayHelpTooltip("Close Empty Quad Batches. This can help clean-up when 1 or more batches have geo counts of 0");

				mImGuiSettings.RenderingCollapseMenuOpen = true;
			} else {
				mImGuiSettings.RenderingCollapseMenuOpen = false;
			}

			ImGui::SetNextItemOpen(mImGuiSettings.CurrentDemoCollapseMenuOpen);
			if (ImGui::CollapsingHeader("Demo")) {
				//ImGui::Text("2D: ");
				//ImGui::SameLine();
				//ImGui::Checkbox("Grid", &mGridDemo.mEnabled);
				//ImGui::SameLine();
				//ImGui::Checkbox("Bouncing Quads", &mBouncingQuadDemo.mEnabled);
				//ImGui::SameLine();
				//ImGui::Checkbox("Custom", &mCustomDemo.mEnabled);
				//ImGui::Text("3D: ");
				//ImGui::SameLine();
				//ImGui::Checkbox("Cube", &mCubeDemo.mEnabled);

				if (ImGui::Button("Disable all demos")) {
					mGridDemo.Render = false;
					mGridDemo.Update = false;

					mBouncingQuadDemo.Render = false;
					mBouncingQuadDemo.Update = false;

					mCustomDemo.RenderScene = false;
					mCustomDemo.RenderUi = false;
					mCustomDemo.Update = false;

					mCubeDemo.Render = false;
					mCubeDemo.Update = false;
				}

				ImGui::Text("Demo Settings & Info:");

				ImGui::BeginTabBar("Demo Tab Bar");
				if (ImGui::BeginTabItem("Grid")) {
					ImGui::Checkbox("Render", &mGridDemo.Render);
					ImGui::Checkbox("Update", &mGridDemo.Update);
					ImGui::Text("Grid Quad Count: %i of Max %i", mGridDemo.mTestGridSize[0] * mGridDemo.mTestGridSize[1], mGridDemo.mTestGridMaxQuadCount);
					int tempTestGridSize[2] = { mGridDemo.mTestGridSize[0], mGridDemo.mTestGridSize[1] };
					if (ImGui::InputInt2("Grid Size", tempTestGridSize)) {
						if (tempTestGridSize[0] > 0 && tempTestGridSize[1] > 0) {
							const int tempGridQuadCount = tempTestGridSize[0] * tempTestGridSize[1];
							if (tempGridQuadCount <= mGridDemo.mTestGridMaxQuadCount) {
								mGridDemo.mTestGridSize[0] = tempTestGridSize[0];
								mGridDemo.mTestGridSize[1] = tempTestGridSize[1];
							} else {
								LOG_WARN(
									"New grid size of " << tempTestGridSize[0] << "x" << tempTestGridSize[1] <<
									" (" << tempTestGridSize[0] * tempTestGridSize[1] <<
									") is too large, max quad count is " << mGridDemo.mTestGridMaxQuadCount
								);
							}
						}
					}
					ImGui::DragFloat2("Quad Size", mGridDemo.mTestGridQuadSize, 0.001f, 0.01f, 0.5f);
					ImGui::DragFloat2("Quad Gap Size", mGridDemo.mTestGridQuadGapSize, 0.001f, 0.01f, 0.5f);
					//ImGui::DragFloat2("Test Grid Origin Pos", );
					//ImGui::Text("UI Quad Count: %i", renderer.getContext().getRenderer2d().getStorage().getUiQuadCount());

					//TOOD:: maybe have radio buttons for RenderStaticGrid or RenderDynamicGrid,
					//	static being the default values and dynamic being from the variables determined by ths menu
					// Maybe have the dynamic settings hidden unless dynamic is selected

					int tempTestTextureIndexOffset = mGridDemo.mTestTexturesIndexOffset;
					if (ImGui::InputInt("Test Texture Offset", &tempTestTextureIndexOffset)) {
						//Cycle through the 8 test texture indexes
						mGridDemo.mTestTexturesIndexOffset = tempTestTextureIndexOffset < 0 ? 0 : tempTestTextureIndexOffset % 8;
					}

					if (ImGui::Button("Reset Grid")) {
						mGridDemo.mTestGridSize[0] = 10;
						mGridDemo.mTestGridSize[1] = 10;
						mGridDemo.mTestGridQuadSize[0] = 0.1f;
						mGridDemo.mTestGridQuadSize[1] = 0.1f;
						mGridDemo.mTestGridQuadGapSize[0] = mGridDemo.mTestGridQuadSize[0] * 1.5f;
						mGridDemo.mTestGridQuadGapSize[1] = mGridDemo.mTestGridQuadSize[1] * 1.5f;
					}

					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("Bouncing Quads")) {
					ImGui::Checkbox("Render", &mBouncingQuadDemo.Render);
					ImGui::Checkbox("Update", &mBouncingQuadDemo.Update);
					ImGui::Text("Bouncing Quads Count: %i", mBouncingQuadDemo.mBouncingQuads.size());

					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("Custom")) {
					ImGui::Checkbox("Render Scene", &mCustomDemo.RenderScene);
					ImGui::Checkbox("Render UI", &mCustomDemo.RenderUi);
					ImGui::Checkbox("Update", &mCustomDemo.Update);

					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("Cube")) {
					ImGui::Checkbox("Render", &mCubeDemo.Render);
					ImGui::Checkbox("Update", &mCubeDemo.Update);

					ImGui::Checkbox("Auto Rotate", &mCubeDemo.AutoRotate);

					ImGui::DragFloat("Auto Rotate Speed", &mCubeDemo.AutoRotateSpeed, 0.1f, 1.0f, 60.0f);


					//Add temp array and set -1 > rotation < 361 to allow for "infinite" drag
					float tempRotation[3] = {
						mCubeDemo.Rotation[0],
						mCubeDemo.Rotation[1],
						mCubeDemo.Rotation[2]
					};
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

						mCubeDemo.Rotation[0] = tempRotation[0];
						mCubeDemo.Rotation[1] = tempRotation[1];
						mCubeDemo.Rotation[2] = tempRotation[2];
					}

					ImGui::DragFloat3("Position", mCubeDemo.Position, 0.05f, -10.0f, 10.0f);
					ImGui::DragFloat("Uniform Scale", &mCubeDemo.Scale, 0.01f, 0.1f, 5.0f);

					if (ImGui::Button("Reset Cube")) {
						mCubeDemo.AutoRotate = false;
						mCubeDemo.AutoRotateSpeed = 15.0f;
						mCubeDemo.Rotation[0] = 0.0f;
						mCubeDemo.Rotation[1] = 0.0f;
						mCubeDemo.Rotation[2] = 0.0f;
						mCubeDemo.Position[0] = 0.0f;
						mCubeDemo.Position[1] = 0.0f;
						mCubeDemo.Position[2] = 0.0f;
						mCubeDemo.Scale = 1.0f;
					}

					ImGui::EndTabItem();
				}
				ImGui::EndTabBar();

				mImGuiSettings.CurrentDemoCollapseMenuOpen = true;
			} else {
				mImGuiSettings.CurrentDemoCollapseMenuOpen = false;
			}

			//TODO:: Use different formats, currently looks bad
			ImGui::SetNextItemOpen(mImGuiSettings.CameraCollapseMenuOpen);
			if (ImGui::CollapsingHeader("Camera")) {
				const bool orthoCameraActive = mImGuiSettings.UseOrthographicCamera;
				const bool perspectiveCameraActive = !orthoCameraActive;
				if (ImGui::RadioButton("Orthographic Camera", orthoCameraActive)) {
					mImGuiSettings.UseOrthographicCamera = true;
				}
				ImGui::SameLine();
				if (ImGui::RadioButton("Perspective Camera", perspectiveCameraActive)) {
					mImGuiSettings.UseOrthographicCamera = false;
				}
				ImGui::Text("Current Camera: ");
				ImGui::SameLine();
				ImGui::Text(mImGuiSettings.UseOrthographicCamera ? "Orthographic" : "Perspective");
				if (mImGuiSettings.UseOrthographicCamera) {
					ImGui::Text("Orthographic Camera Controls");
					ImGui::BulletText("W, A, S, D: Move Camera");
					ImGui::BulletText("Hold Left Shift: Increase move speed");
					ImGui::BulletText("Z, X: Zoom Camera In & Out");
					ImGui::BulletText("Right Click Drag Camera");
					imGuiDisplayHelpTooltip("NOTE:: Drag doesn't work properly at all update rates");
					ImGui::Text("Camera Position");
					ImGui::Text("X: %f", TG_mOrthoCameraController->getPosition().x);
					ImGui::Text("Y: %f", TG_mOrthoCameraController->getPosition().y);
					ImGui::Text("Zoom: %f", TG_mOrthoCameraController->getZoomLevel());
					imGuiDisplayHelpTooltip("Higher is more \"zoomed in\" and lower is more \"zoomed out\"");
				} else {
					ImGui::Text("Perspective Camera Controls");
					ImGui::Text("Position");
					ImGui::Text("X: %f", mPerspectiveCameraController->getPosition().x);
					ImGui::Text("Y: %f", mPerspectiveCameraController->getPosition().y);
					ImGui::Text("Z: %f", mPerspectiveCameraController->getPosition().z);
					ImGui::Text("Direction");
					ImGui::Text("X: %f", mPerspectiveCameraController->getDirection().x);
					ImGui::Text("Y: %f", mPerspectiveCameraController->getDirection().y);
					ImGui::Text("Z: %f", mPerspectiveCameraController->getDirection().z);
				}
				if (ImGui::Button("Reset Orthographic Camera")) {
					TG_mOrthoCameraController->setPosition({ 0.0f, 0.0f, 1.0f });
					TG_mOrthoCameraController->setZoomLevel(1.0f);
				}
				if (ImGui::Button("Reset Perspective Camera")) {
					mPerspectiveCameraController->setPosition({ 0.0f, 0.0f, 5.0f });
					mPerspectiveCameraController->setDirection({ 0.0f, 0.0f, 0.0f });
				}

				mImGuiSettings.CameraCollapseMenuOpen = true;
			} else {
				mImGuiSettings.CameraCollapseMenuOpen = false;
			}

			ImGui::EndTabItem();
		}
		
		if (ImGui::BeginTabItem("UI Style")) {
			ImGui::Text("NOTE: Currently UI style selection is not persistent");
			ImGui::ShowStyleEditor();
			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();
		ImGui::End();
	}

	void TG_AppLogic::close() {		
		RendererVulkan& renderer = GET_RENDERER;
		
		//Custom demo
		renderer.closeGpuResource(mCustomDemo.mSceneShaderProgram);
		renderer.closeGpuResource(mCustomDemo.mSceneVertexArray);
		renderer.closeGpuResource(mCustomDemo.mUiShaderProgram);
		renderer.closeGpuResource(mCustomDemo.mUiVao);
		renderer.closeGpuResource(mCustomDemo.mTestTexture1);
		renderer.closeGpuResource(mCustomDemo.mTestTexture2);

		//Cube demo
		renderer.closeGpuResource(mCubeDemo.mCubeModel);
		renderer.closeGpuResource(mCubeDemo.mSceneShaderProgram);

		//for (std::shared_ptr<TextureVulkan> texture : mTestTextures) {
		//	renderer.closeGpuResource(texture);
		//}
	}

	void TG_AppLogic::onResize(float aspectRatio) {
		TG_mOrthoCameraController->onViewportResize(aspectRatio);
		setUiProjection(aspectRatio);
	}

	void TG_AppLogic::initDemos() {
		RenderingContextVulkan& context = GET_RENDERER.getContext();

		initBouncingQuadsDemo();
		initCustomDemo();
		initCubeDemo();

		//TODO::
		// Currently pipeline render order is determined by creation order as depth buffers are not yet supported.
		context.createPipeline(
			mCustomDemo.mScenePipelineName,
			SwapChainVulkan::ERenderPassType::SCENE,
			*mCustomDemo.mSceneShaderProgram,
			mCustomDemo.mSceneVertexType
		);
		context.createPipeline(
			mCustomDemo.mUiPipelineName,
			SwapChainVulkan::ERenderPassType::APP_UI,
			*mCustomDemo.mUiShaderProgram,
			mCustomDemo.mUiVertexType
		);
		context.createPipeline(
			mCubeDemo.mScenePipelineName,
			SwapChainVulkan::ERenderPassType::SCENE,
			*mCubeDemo.mSceneShaderProgram,
			mCubeDemo.mSceneVertexType
		);

		context.createPipelineUniformObjects();
	}

	void TG_AppLogic::populateTestGrid(int width, int height) {
		const std::vector<std::shared_ptr<TextureVulkan>>& testTextures =
			GET_RENDERER.getContext().getRenderer2d().getStorage().getTestTextures();

		mGridDemo.mTexturedTestGrid.clear();
		uint32_t texturesInUseCount = std::max(std::max(width, height), width * height);
		texturesInUseCount = std::min(texturesInUseCount, static_cast<uint32_t>(testTextures.size()));
		mGridDemo.mTexturedTestGrid.resize(texturesInUseCount);

		uint32_t index = 0;
		for (float y = 0.0f; y < height; y++) {
			for (float x = 0.0f; x < width; x++) {
				uint32_t textureSlot = static_cast<uint32_t>(x + y) % 8;
				mGridDemo.mTexturedTestGrid[textureSlot].push_back({
					{x * mGridDemo.mTestGridQuadGapSize[0], y * mGridDemo.mTestGridQuadGapSize[1], 0.5f},
					{mGridDemo.mTestGridQuadSize[0], mGridDemo.mTestGridQuadSize[1]},
					{0.0f, 1.0f, 1.0f, 1.0f},
					0.0f,
					testTextures[(static_cast<uint32_t>(x + y) + mGridDemo.mTestTexturesIndexOffset) % 8]
					});
				index++;
			}
		}
	}

	void TG_AppLogic::initBouncingQuadsDemo() {
		const std::vector<std::shared_ptr<TextureVulkan>>& testTextures =
			GET_RENDERER.getContext().getRenderer2d().getStorage().getTestTextures();
		for (size_t i = 0; i < mBouncingQuadDemo.mBouncingQuadCount; i++) {
			mBouncingQuadDemo.mBouncingQuads.push_back({
				//Semi-random values for starting position, velocitiy and assigned texture
				{((float)(rand() % 5000) / 500.0f) - 1.0f,((float)(rand() % 5000) / 500.0f) - 1.0f, 0.8f},
				{mGridDemo.mTestGridQuadSize[0], mGridDemo.mTestGridQuadSize[1]},
				{0.0f, 1.0f, 1.0f, 1.0f},
				0.0f,
				testTextures[rand() % 8]
				});
			mBouncingQuadDemo.mBouncingQuadVelocities.push_back({ (float)(rand() % 800) / 60.0f, (float)(rand() % 800) / 60.0f });
		}
	}

	void TG_AppLogic::initCustomDemo() {
		mCustomDemo.mTestTexture1 = ObjInit::texture(mCustomDemo.testTexturePath);
		mCustomDemo.mTestTexture2 = ObjInit::texture(mCustomDemo.testTexture2Path);

		//for (int i = 0; i < 8; i++) {
		//	std::string path = testTexturesPath + "texture" + std::to_string(i) + ".png";
		//	std::shared_ptr<TextureVulkan> testTexture = ObjInit::texture(path);
		//	mTestTextures.push_back(testTexture);
		//}

		mCustomDemo.mSceneShaderProgram = ObjInit::shaderProgram(
			ObjInit::shader(
				EShaderType::VERTEX,
				mCustomDemo.texturedShaderVertPath
				//flatColourShaderVertPath
			),
			ObjInit::shader(
				EShaderType::FRAGMENT,
				mCustomDemo.texturedShaderFragPath
				//flatColourShaderFragPath
			)
		);

		ShaderUniformLayout& customLayout = mCustomDemo.mSceneShaderProgram->getUniformLayout();
		customLayout.setValue(0, sizeof(CustomDemo::UniformBufferObject));
		customLayout.setTexture(1, { mCustomDemo.mTestTexture1->getImageView(), mCustomDemo.mTestTexture1->getSampler() });

		mCustomDemo.mSceneVertexArray = ObjInit::vertexArray();
		std::shared_ptr<VertexBufferVulkan> sceneVb = ObjInit::stagedVertexBuffer(
			mCustomDemo.mSceneVertexType,
			mCustomDemo.mSceneVertices.data(),
			(size_t) mCustomDemo.mSceneVertexType * mCustomDemo.mSceneVertices.size(),
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		);
		mCustomDemo.mSceneVertexArray->addVertexBuffer(sceneVb);
		std::shared_ptr<IndexBufferVulkan> sceneIb = ObjInit::stagedIndexBuffer(
			mCustomDemo.indices.data(),
			sizeof(uint32_t) * mCustomDemo.indices.size()
		);
		mCustomDemo.mSceneVertexArray->setDrawCount(static_cast<uint32_t>(mCustomDemo.indices.size()));
		mCustomDemo.mSceneVertexArray->setIndexBuffer(sceneIb);

		mCustomDemo.mUiShaderProgram = ObjInit::shaderProgram(
			ObjInit::shader(EShaderType::VERTEX, mCustomDemo.mUiShaderVertPath),
			ObjInit::shader(EShaderType::FRAGMENT, mCustomDemo.mUiShaderFragPath)
		);
		mCustomDemo.mUiShaderProgram->getUniformLayout().setValue(0, sizeof(CustomDemo::UniformBufferObject));

		mCustomDemo.mUiVao = ObjInit::vertexArray();
		std::shared_ptr<VertexBufferVulkan> appUiVb = ObjInit::stagedVertexBuffer(
			mCustomDemo.mUiVertexType,
			mCustomDemo.mUiVertices.data(),
			(size_t) mCustomDemo.mUiVertexType * mCustomDemo.mUiVertices.size(),
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		);
		mCustomDemo.mUiVao->addVertexBuffer(appUiVb);
		std::shared_ptr<IndexBufferVulkan> appUiIb = ObjInit::stagedIndexBuffer(
			mCustomDemo.mUiIndices.data(),
			sizeof(mCustomDemo.mUiIndices[0]) * mCustomDemo.mUiIndices.size()
		);
		mCustomDemo.mUiVao->setDrawCount(static_cast<uint32_t>(mCustomDemo.mUiIndices.size()));
		mCustomDemo.mUiVao->setIndexBuffer(appUiIb);
	}

	void TG_AppLogic::initCubeDemo() {
		mCubeDemo.mCubeModel = ModelVulkan::createModel(mCubeDemo.testCubeObjFilepath);
		mCubeDemo.mCubeModel->getVao().setPushConstantPtr(&mCubeDemo.mTranslation);

		mCubeDemo.mSceneShaderProgram = ObjInit::shaderProgram(
			ObjInit::shader(
				EShaderType::VERTEX,
				mCubeDemo.flatColourShaderVertPath
			),
			ObjInit::shader(
				EShaderType::FRAGMENT,
				mCubeDemo.flatColourShaderFragPath
			)
		);
		ShaderUniformLayout& cubeLayout = mCubeDemo.mSceneShaderProgram->getUniformLayout();
		cubeLayout.setValue(0, sizeof(CubeDemo::UniformBufferObject));
		//Push constant for transformation matrix
		cubeLayout.addPushConstant(VK_SHADER_STAGE_VERTEX_BIT, sizeof(glm::mat4x4));
	}

	void TG_AppLogic::imGuiDisplayHelpTooltip(const char* message) {
		ImGui::SameLine();
		ImGui::Text("(?)");
		if (ImGui::IsItemHovered()) {
			ImGui::BeginTooltip();
			ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
			ImGui::TextUnformatted(message);
			ImGui::PopTextWrapPos();
			ImGui::EndTooltip();
		}
	}

	void TG_AppLogic::imGuiPrintDrawCallTableColumn(const char* pipelineName, uint32_t drawCount) {
		//IMPORTANT:: Assumes already inside the Draw Call Count debug info table
		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		ImGui::Text(pipelineName);
		ImGui::TableNextColumn();
		ImGui::Text("%i", drawCount);
	}
}
