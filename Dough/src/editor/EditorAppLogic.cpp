#include "editor/EditorAppLogic.h"

#include "dough/rendering/ObjInit.h"
#include "dough/application/Application.h"
#include "dough/input/InputCodes.h"
#include "dough/Logging.h"

#include <imgui/imgui.h>

#define GET_RENDERER Application::get().getRenderer()

namespace DOH::EDITOR {

	EditorAppLogic::EditorAppLogic()
	:	IApplicationLogic(),
		mInnerAppState(EInnerAppState::STOPPED)
	{
		mInnerAppTimer = std::make_unique<PausableTimer>();

		mObjModelsDemo = std::make_unique<ObjModelsDemo>();
		mCustomDemo = std::make_unique<CustomDemo>();
		mGridDemo = std::make_unique<GridDemo>();
		mBouncingQuadDemo = std::make_unique<BouncingQuadDemo>();

		mEditorSettings = std::make_unique<EditorSettings>();
	}

	void EditorAppLogic::init(float aspectRatio) {
		mGridDemo->TestGridMaxQuadCount = Renderer2dStorageVulkan::MAX_BATCH_COUNT_QUAD * Renderer2dStorageVulkan::BATCH_MAX_GEO_COUNT_QUAD;

		initDemos();

		setUiProjection(aspectRatio);

		mOrthoCameraController = std::make_shared<EditorOrthoCameraController>(aspectRatio);
		mPerspectiveCameraController = std::make_shared<EditorPerspectiveCameraController>(aspectRatio);

		mPerspectiveCameraController->setPosition({ 0.0f, 0.0f, 5.0f });

		mEditorSettings->UseOrthographicCamera = false;
		mEditorSettings->TextureViewerWindows = {};
	}

	void EditorAppLogic::update(float delta) {
		//Handle input first
		if (Input::isKeyPressed(DOH_KEY_F1)) {
			mEditorSettings->RenderDebugWindow = true;
		}

		if (mEditorSettings->UseOrthographicCamera) {
			mOrthoCameraController->onUpdate(delta);
		} else {
			mPerspectiveCameraController->onUpdate(delta);
		}

		//IMPORTANT NOTE:: Prevent updating anything after Editor specific parts are updated
		//TODO:: Have app-specific things in its own class so the code below is in an AppLogic::update()
		if (mInnerAppState != EInnerAppState::PLAYING) {
			return;
		}

		//TODO:: App specific updates in separate class
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

	void EditorAppLogic::render() {
		RendererVulkan& renderer = GET_RENDERER;
		RenderingContextVulkan& context = renderer.getContext();
		Renderer2dVulkan& renderer2d = context.getRenderer2d();

		renderer.beginScene(
			mEditorSettings->UseOrthographicCamera ?
				mOrthoCameraController->getCamera() : mPerspectiveCameraController->getCamera()
		);

		if (mCustomDemo->RenderScene) {
			//context.addRenderableToSceneDrawList(mCustomDemo->ScenePipelineName, mCustomDemo->SceneRenderable);
			mCustomDemo->CustomSceneConveyer.safeAddRenderable(mCustomDemo->SceneRenderable);
		}

		if (mObjModelsDemo->Render) {
			if (mObjModelsDemo->ScenePipelineConveyer.isValid() && mObjModelsDemo->WireframePipelineConveyer.isValid()) {
				for (auto& obj : mObjModelsDemo->RenderableObjects) {
					if (obj->Render || mObjModelsDemo->RenderAllStandard) {
						mObjModelsDemo->ScenePipelineConveyer.addRenderable(obj);
					}
					if (obj->RenderWireframe || mObjModelsDemo->RenderAllWireframe) {
						mObjModelsDemo->WireframePipelineConveyer.addRenderable(obj);
					}
				}
			}
		}

		if (mGridDemo->Render) {
			//Different ways of drawing quads (inserting into RenderBatches)
			//
			//for (Quad& quad : quadArray) {
			//	rrenderer2d.drawQuadScene(quad);
			//}
			//for (Quad& quad : sameTexturedQuads) {
			//	renderer2d.drawQuadTexturedScene(quad);
			//}
			//renderer2d.drawQuadArrayScene(sameTexturedQuads);
			//renderer2d.drawQuadArrayTexturedScene(sameTexturedQuads);
			//renderer2d.drawQuadArraySameTextureScene(sameTexturedQuads);

			if (mGridDemo->QuadDrawColour) {
				for (std::vector<Quad>& sameTexturedQuads : mGridDemo->TexturedTestGrid) {
					renderer2d.drawQuadArrayScene(sameTexturedQuads);
				}
			} else {
				for (std::vector<Quad>& sameTexturedQuads : mGridDemo->TexturedTestGrid) {
					renderer2d.drawQuadArraySameTextureScene(sameTexturedQuads);
				}
			}
		}

		if (mBouncingQuadDemo->Render) {
			//for (Quad& quad : mBouncingQuadDemo.BouncingQuads) {
			//	renderer2d.drawQuadScene(quad);
			//}
			//for (Quad& quad : mBouncingQuadDemo.BouncingQuads) {
			//	renderer2d.drawQuadTexturedScene(quad);
			//}
			//renderer2d.drawQuadArrayScene(mBouncingQuadDemo.BouncingQuads);
			//renderer2d.drawQuadArrayTexturedScene(mBouncingQuadDemo.BouncingQuads);

			if (mBouncingQuadDemo->QuadDrawColour) {
				for (Quad& quad : mBouncingQuadDemo->BouncingQuads) {
					renderer2d.drawQuadScene(quad);
				}
			} else {
				for (Quad& quad : mBouncingQuadDemo->BouncingQuads) {
					renderer2d.drawQuadTexturedScene(quad);
				}
			}
		}

		renderer.endScene();

		renderer.beginUi(mCustomDemo->UiProjMat);
		if (mCustomDemo->RenderUi) {
			//context.addRenderableToUiDrawList(mCustomDemo->UiPipelineName, mCustomDemo->UiRenderable);
			mCustomDemo->CustomUiConveyer.safeAddRenderable(mCustomDemo->UiRenderable);
		}
		renderer.endUi();
	}

	void EditorAppLogic::imGuiRender(float delta) {
		
		//NOTE:: ImGui Debug info windows
		//ImGui::ShowMetricsWindow();
		//ImGui::ShowStackToolWindow();
		//ImGui::ShowDemoWindow();

		if (mEditorSettings->RenderDebugWindow) {
			imGuiRenderDebugWindow(delta);
		}
	}

	void EditorAppLogic::imGuiRenderDebugWindow(float delta) {
		RendererVulkan& renderer = GET_RENDERER;
		Renderer2dVulkan& renderer2d = renderer.getContext().getRenderer2d();
		ImGuiWrapper& imGuiWrapper = renderer.getContext().getImGuiWrapper();

		ImGui::Begin("Dough Editor Window", &mEditorSettings->RenderDebugWindow);
		ImGui::BeginTabBar("Main Tab Bar");
		if (ImGui::BeginTabItem("Dough Editor")) {

			ImGui::SetNextItemOpen(mEditorSettings->EditorCollapseMenuOpen);
			if (mEditorSettings->EditorCollapseMenuOpen = ImGui::CollapsingHeader("Editor")) {
				ApplicationLoop& loop = Application::get().getLoop();
				Window& window = Application::get().getWindow();
				bool focused = Application::get().isFocused();
				
				ImGui::Text("Editor Runtime: %fs", Time::convertMillisToSeconds(Application::get().getAppInfoTimer().getCurrentTickingTimeMillis()));
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
				ImGui::Text(
					"FPS: %i \t(Fore: %i, Back: %i)",
					static_cast<int>(loop.getFps()),
					static_cast<int>(loop.getTargetFps()),
					static_cast<int>(loop.getTargetBackgroundFps())
				);
				imGuiDisplayHelpTooltip("Max: Current Target UPS or 360, whichever is lower. Min: 15\nFPS displayed is the count of frames in the last full second interval");
				ImGui::Text(
					"UPS: %i \t(Fore: %i, Back: %i)",
					static_cast<int>(loop.getUps()),
					static_cast<int>(loop.getTargetUps()),
					static_cast<int>(loop.getTargetBackgroundUps())
				);
				imGuiDisplayHelpTooltip("Max: 1000. Min: Current Target FPS or 15, whichever is higher.\nUPS displayed is the count of frames in the last full second interval");
				int tempTargetFps = static_cast<int>(loop.getTargetFps());
				if (ImGui::InputInt("Target FPS", &tempTargetFps)) {
					if (loop.isValidTargetFps(static_cast<float>(tempTargetFps))) {
						loop.setTargetFps(static_cast<float>(tempTargetFps));
					}
				}
				int tempTargetUps = static_cast<int>(loop.getTargetUps());
				if (ImGui::InputInt("Target UPS", &tempTargetUps)) {
					if (loop.isValidTargetUps(static_cast<float>(tempTargetUps))) {
						loop.setTargetUps(static_cast<float>(tempTargetUps));
					}
				}

				{
					const glm::vec2 cursorPos = Input::getCursorPos();
					ImGui::Text(
						"Cursor Position: X: %i, Y: %i",
						static_cast<int>(cursorPos.x),
						static_cast<int>(cursorPos.y)
					);
				}

				bool runInBackground = loop.isRunningInBackground();
				if (ImGui::Checkbox("Run In Background", &runInBackground)) {
					loop.setRunInBackground(runInBackground);
				}
				if (ImGui::Button("Stop Rendering ImGui Windows")) {
					mEditorSettings->RenderDebugWindow = false;
				}
				imGuiDisplayHelpTooltip("When hidden press F1 to start rendering ImGui again");

				if (ImGui::Button("Quit Application") || ImGui::IsItemActive()) {
					mEditorSettings->QuitButtonHoldTime += delta;

					//Close app when releasing mouse button after holding for required time
					if (mEditorSettings->QuitButtonHoldTime >= mEditorSettings->QuitHoldTimeRequired && ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
						Application::get().stop();
					}
				} else {
					mEditorSettings->QuitButtonHoldTime = 0.0f;
				}
				//Draw a rectangle to show how long to hold for
				if (ImGui::IsItemHovered() || ImGui::IsItemActive()) {
					ImGui::BeginTooltip();

					ImGui::PushStyleColor(ImGuiCol_PlotHistogram, { 1.0f, 0.0f, 0.0f, 1.0f });
					ImGui::ProgressBar((mEditorSettings->QuitButtonHoldTime / mEditorSettings->QuitHoldTimeRequired), { 200.0f, 20.0f });
					ImGui::PopStyleColor(1);

					ImGui::EndTooltip();
				}
			}

			ImGui::SetNextItemOpen(mEditorSettings->RenderingCollapseMenuOpen);
			if (mEditorSettings->RenderingCollapseMenuOpen = ImGui::CollapsingHeader("Rendering")) {
				AppDebugInfo& debugInfo = Application::get().getDebugInfo();
				MonoSpaceTextureAtlasVulkan& atlas = *renderer.getContext().getRenderer2d().getStorage().getTestTextureAtlas();

				const double frameTime = debugInfo.LastUpdateTimeMillis + debugInfo.LastRenderTimeMillis;

				if (debugInfo.FrameTimeIndex == AppDebugInfo::FrameTimesCount) {
					debugInfo.FrameTimeIndex = 0;
					debugInfo.FrameTimesArrayIsFull = true;
				}
				debugInfo.FrameTimesMillis[debugInfo.FrameTimeIndex] = static_cast<float>(frameTime);
				debugInfo.FrameTimeIndex++;

				ImGui::PlotLines(
					"Frame Times (ms)",
					debugInfo.FrameTimesMillis,
					debugInfo.FrameTimesArrayIsFull ? AppDebugInfo::FrameTimesCount : debugInfo.FrameTimeIndex
				);

				//Milliseconds
				ImGui::Text("Frame: %lfms", frameTime);
				ImGui::Text("Update: %lfms", debugInfo.LastUpdateTimeMillis);
				ImGui::Text("Render: %lfms", debugInfo.LastRenderTimeMillis);
				//Seconds
				//ImGui::Text("Frame: %fs", Time::convertMillisToSeconds(frameTime));
				//ImGui::Text("Update: %fs", Time::convertMillisToSeconds(debugInfo.LastUpdateTimeMillis));
				//ImGui::Text("Render: %fs", Time::convertMillisToSeconds(debugInfo.LastRenderTimeMillis));
				ImGui::BeginTable("Draw Call Info", 2);
				ImGui::TableSetupColumn("Pipeline");
				ImGui::TableSetupColumn("Draw Call Count");
				ImGui::TableHeadersRow();
				ImGui::TableSetColumnIndex(0);
				//Individual scene pipeline draw calls
				for (const auto& [name, pipeline] : renderer.getContext().getAppSceneGraphicsPipelines()) {
					imGuiPrintDrawCallTableColumn(name.c_str(), pipeline->getVaoDrawCount());
				}
				//Total scene pipeline draw calls
				imGuiPrintDrawCallTableColumn("Total Scene", debugInfo.SceneDrawCalls);
				ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg1, ImColor::ImColor(125, 125, 125, 90));
				//Individual UI pipeline draw calls
				for (const auto& [name, pipeline] : renderer.getContext().getAppUiGraphicsPipelines()) {
					imGuiPrintDrawCallTableColumn(name.c_str(), pipeline->getVaoDrawCount());
				}
				//Total UI pipeline draw calls
				imGuiPrintDrawCallTableColumn("Total UI", debugInfo.UiDrawCalls);
				ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg1, ImColor::ImColor(125, 125, 125, 90));
				imGuiPrintDrawCallTableColumn("Quad Batch", debugInfo.BatchRendererDrawCalls);
				imGuiPrintDrawCallTableColumn("Total", debugInfo.TotalDrawCalls);
				ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg1, ImColor::ImColor(128, 128, 255, 100));
				ImGui::EndTable();

				ImGui::Text("Quads Drawn: %i", renderer2d.getDrawnQuadCount());
				ImGui::Text("Quads Truncated: %i", renderer2d.getTruncatedQuadCount());
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
				if (ImGui::Button("View atlas texture")) {
					const auto& itr = mEditorSettings->TextureViewerWindows.find(atlas.getId());
					if (itr != mEditorSettings->TextureViewerWindows.end()) {
						itr->second.Display = true;
					} else {
						mEditorSettings->TextureViewerWindows.emplace(
							atlas.getId(),
							ImGuiTextureViewerWindow(
								atlas,
								true,
								false,
								1.0f
							)
						);
					}
				}
				imGuiDisplayHelpTooltip("Display Texture Altas texture inside a separate window.");

				if (ImGui::Button("Close All Empty Quad Batches")) {
					renderer.getContext().getRenderer2d().closeEmptyQuadBatches();
				}
				imGuiDisplayHelpTooltip("Close Empty Quad Batches. This can help clean-up when 1 or more batches have geo counts of 0");
			}

			ImGui::SetNextItemOpen(mEditorSettings->InnerAppCollapseMenu);
			if (mEditorSettings->InnerAppCollapseMenu = ImGui::CollapsingHeader("Inner App")) {
				if (mInnerAppState == EInnerAppState::PLAYING || mInnerAppState == EInnerAppState::PAUSED) {
					ImGui::Text("Inner App Runtime: %fs", Time::convertMillisToSeconds(mInnerAppTimer->getCurrentTickingTimeMillis()));
				} else {
					ImGui::Text("Inner App");
				}

				ImGui::SameLine();
				switch (mInnerAppState) {
					case EInnerAppState::PAUSED:
						ImGui::Text("Paused");

						if (ImGui::Button("Continue")) {
							mInnerAppState = EInnerAppState::PLAYING;

							mInnerAppTimer->unPause();
						}
						ImGui::SameLine();
						if (ImGui::Button("STOP")) {
							mInnerAppState = EInnerAppState::STOPPED;

							mInnerAppTimer->end();
							//TODO:: resetInnerApp();
						}
						break;

					case EInnerAppState::PLAYING:
						ImGui::Text("Playing");

						if (ImGui::Button("Pause")) {
							mInnerAppState = EInnerAppState::PAUSED;

							mInnerAppTimer->pause();
						}
						ImGui::SameLine();
						if (ImGui::Button("STOP")) {
							mInnerAppState = EInnerAppState::STOPPED;

							mInnerAppTimer->end();
							//TODO:: resetInnerApp();
						}
						break;

					case EInnerAppState::STOPPED:
						ImGui::Text("Stopped");

						if (ImGui::Button("Play")) {
							//TODO:: re-initialise/reset inner app 
							mInnerAppState = EInnerAppState::PLAYING;

							mInnerAppTimer->start();
						}
						break;

					case EInnerAppState::NONE:
					default:
						LOG_ERR("Inner App state Unknown or NONE");
						break;
				}

				ImGui::SetNextItemOpen(mEditorSettings->CurrentDemoCollapseMenuOpen);
				if (mEditorSettings->CurrentDemoCollapseMenuOpen = ImGui::CollapsingHeader("Demo")) {
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
					}

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

						MonoSpaceTextureAtlasVulkan& atlas = *renderer.getContext().getRenderer2d().getStorage().getTestTextureAtlas();

						int tempTestTextureRowOffset = mGridDemo->TestTexturesRowOffset;
						if (ImGui::InputInt("Test Texture Row Offset", &tempTestTextureRowOffset)) {
							mGridDemo->TestTexturesRowOffset = tempTestTextureRowOffset < 0 ? 0 : tempTestTextureRowOffset % atlas.getRowCount();
							mGridDemo->IsUpToDate = false;
						}
						int tempTestTextureColOffset = mGridDemo->TestTexturesColumnOffset;
						if (ImGui::InputInt("Test Texture Col Offset", &tempTestTextureColOffset)) {
							mGridDemo->TestTexturesColumnOffset = tempTestTextureColOffset < 0 ? 0 : tempTestTextureColOffset % atlas.getColCount();
							mGridDemo->IsUpToDate = false;
						}

						if (ImGui::ColorPicker4("Grid Colour", &mGridDemo->QuadColour.x)) {
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
						ImGui::Checkbox("Display Renderable Models List", &mEditorSettings->RenderObjModelsList);
						if (ImGui::Button("Clear Objects")) {
							renderables.clear();
						}
						ImGui::EndTabItem();
					}
					ImGui::EndTabBar();
				}
			}

			//TODO:: Use different formats, currently looks bad
			ImGui::SetNextItemOpen(mEditorSettings->CameraCollapseMenuOpen);
			if (mEditorSettings->CameraCollapseMenuOpen = ImGui::CollapsingHeader("Camera")) {
				const bool orthoCameraActive = mEditorSettings->UseOrthographicCamera;
				const bool perspectiveCameraActive = !orthoCameraActive;
				if (ImGui::RadioButton("Orthographic Camera", orthoCameraActive)) {
					mEditorSettings->UseOrthographicCamera = true;
				}
				ImGui::SameLine();
				if (ImGui::RadioButton("Perspective Camera", perspectiveCameraActive)) {
					mEditorSettings->UseOrthographicCamera = false;
				}
				ImGui::Text("Current Camera: ");
				ImGui::SameLine();
				ImGui::Text(mEditorSettings->UseOrthographicCamera ? "Orthographic" : "Perspective");
				if (mEditorSettings->UseOrthographicCamera) {
					ImGui::Text("Orthographic Camera Controls");
					ImGui::BulletText("W, A, S, D: Move Camera");
					ImGui::BulletText("Hold Left Shift: Increase move speed");
					ImGui::BulletText("Z, X: Zoom Camera In & Out");
					imGuiDisplayHelpTooltip("NOTE:: Changes top/bottom & left/right orthographic perspective, does not change the Z clipping range");
					ImGui::BulletText("Right Click Drag Camera");
					imGuiDisplayHelpTooltip("NOTE:: Drag doesn't work properly at all update rates");
					
					ImGui::Text("Camera Position");
					const auto& pos = mOrthoCameraController->getPosition();
					float tempPos[3] = {pos.x, pos.y, pos.z};
					if (ImGui::DragFloat3("Pos", tempPos)) {
						mOrthoCameraController->setPosition({ tempPos[0], tempPos[1], tempPos[2] });
					}
					ImGui::Text("Zoom: %f", mOrthoCameraController->getZoomLevel());
					imGuiDisplayHelpTooltip("Higher is more \"zoomed out\" and lower is more \"zoomed in\"");
				} else {
					ImGui::Text("Perspective Camera Controls");
					imGuiBulletTextWrapped("W, A, S, D, C, Space Bar: Move Camera");
					imGuiBulletTextWrapped("Arrow Left, Arrow Right, Arrow Up, Arrow Down, Z, X, Mouse Right Click & Drag: Change camera looking direction");
					imGuiBulletTextWrapped("Hold Left Shift: Increase translation speed");
					
					const auto pos = mPerspectiveCameraController->getPosition();
					const auto dir = mPerspectiveCameraController->getDirection();
					float tempPos[3] = { pos.x, pos.y, pos.z };
					if (ImGui::DragFloat3("Position", tempPos)) {
						mPerspectiveCameraController->setPosition({ tempPos[0], tempPos[1], tempPos[2] });
					}
					float tempDir[3] = { dir.x, dir.y, dir.z };
					if (ImGui::DragFloat3("Direction", tempDir)) {
						mPerspectiveCameraController->setDirection({ tempDir[0], tempDir[1], tempDir[2] });
					}
				}
				if (ImGui::Button("Reset Orthographic Camera")) {
					mOrthoCameraController->setPosition({ 0.0f, 0.0f, 1.0f });
					mOrthoCameraController->setZoomLevel(1.0f);
				}
				ImGui::SameLine();
				if (ImGui::Button("Reset Perspective Camera")) {
					mPerspectiveCameraController->setPosition({ 0.0f, 0.0f, 5.0f });
					mPerspectiveCameraController->setDirection({ 0.0f, 0.0f, 0.0f });
				}
			}

			ImGui::EndTabItem();
		}
		
		if (ImGui::BeginTabItem("UI Style")) {
			ImGui::Text("NOTE: Currently UI style selection is not persistent");
			ImGui::ShowStyleEditor();
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Rendering Device Info")) {
			const auto& deviceInfo = renderer.getContext().getRenderingDeviceInfo();
			ImGui::Text("Vulkan SDK Version: ");
			ImGui::SameLine();
			ImGui::Text(deviceInfo.ApiVersion.c_str());
			
			ImGui::Text("Rendering Device Name: ");
			ImGui::SameLine();
			ImGui::Text(deviceInfo.DeviceName.c_str());

			//TODO:: device driver version, these are vendor specific so need to implement a way of extracting it from AMD/NVIDIA/other

			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();
		ImGui::End();

		if (mEditorSettings->RenderObjModelsList) {
			const auto& renderables = mObjModelsDemo->RenderableObjects;
			if (ImGui::Begin("Renderable Models List")) {
				ImGui::Checkbox("Render All", &mObjModelsDemo->RenderAllStandard);
				ImGui::SameLine();
				ImGui::Checkbox("Render All Wireframe", &mObjModelsDemo->RenderAllWireframe);

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
							}
					
							ImGui::TreePop();
						}
						objIndex += objectsPerNode;
					}

					ImGui::TreePop();
				}
			}
			ImGui::End();
		}

		for (auto& textureViewer : mEditorSettings->TextureViewerWindows) {
			if (textureViewer.second.Display) {
				imGuiDrawTextureViewerWindow(textureViewer.second);
			}
		}
	}

	void EditorAppLogic::close() {
		RendererVulkan& renderer = GET_RENDERER;
		
		//Custom demo
		renderer.closeGpuResource(mCustomDemo->SceneShaderProgram);
		renderer.closeGpuResource(mCustomDemo->SceneVao);
		renderer.closeGpuResource(mCustomDemo->UiShaderProgram);
		renderer.closeGpuResource(mCustomDemo->UiVao);
		renderer.closeGpuResource(mCustomDemo->TestTexture1);
		renderer.closeGpuResource(mCustomDemo->TestTexture2);

		//Obj Models Demo
		for (const auto& model : mObjModelsDemo->LoadedModels) {
			renderer.closeGpuResource(model);
		}
		mObjModelsDemo->LoadedModels.clear();
		mObjModelsDemo->RenderableObjects.clear();
		renderer.closeGpuResource(mObjModelsDemo->SceneShaderProgram);

		//for (std::shared_ptr<TextureVulkan> texture : mTestTextures) {
		//	renderer.closeGpuResource(texture);
		//}
	}

	void EditorAppLogic::onResize(float aspectRatio) {
		mOrthoCameraController->onViewportResize(aspectRatio);
		setUiProjection(aspectRatio);
	}

	void EditorAppLogic::initDemos() {
		RenderingContextVulkan& context = GET_RENDERER.getContext();

		//Test grid is repopulated per update to apply changes from editor. To only populate once remove re-population from update and populate here.
		//populateTestGrid(static_cast<uint32_t>(mGridDemo.TestGridSize[0]), static_cast<uint32_t>(mGridDemo.TestGridSize[1]));
		
		initBouncingQuadsDemo();
		initCustomDemo();
		initObjModelsDemo();

		context.createPipelineUniformObjects();
	}

	void EditorAppLogic::populateTestGrid(uint32_t width, uint32_t height) {
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
					{mGridDemo->TestGridQuadSize[0], mGridDemo->TestGridQuadSize[1]},
					{mGridDemo->QuadColour.x, mGridDemo->QuadColour.y, mGridDemo->QuadColour.z, mGridDemo->QuadColour.w},
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

	void EditorAppLogic::initBouncingQuadsDemo() {
		bouncingQuadsDemoAddRandomQuads(5000);
	}

	void EditorAppLogic::initCustomDemo() {
		mCustomDemo->TestTexture1 = ObjInit::texture(mCustomDemo->TestTexturePath);
		mCustomDemo->TestTexture2 = ObjInit::texture(mCustomDemo->TestTexture2Path);

		//for (int i = 0; i < 8; i++) {
		//	std::string path = testTexturesPath + "texture" + std::to_string(i) + ".png";
		//	std::shared_ptr<TextureVulkan> testTexture = ObjInit::texture(path);
		//	mTestTextures.push_back(testTexture);
		//}

		mCustomDemo->SceneShaderProgram = ObjInit::shaderProgram(
			ObjInit::shader(
				EShaderType::VERTEX,
				mCustomDemo->TexturedShaderVertPath
				//flatColourShaderVertPath
			),
			ObjInit::shader(
				EShaderType::FRAGMENT,
				mCustomDemo->TexturedShaderFragPath
				//flatColourShaderFragPath
			)
		);

		ShaderUniformLayout& customLayout = mCustomDemo->SceneShaderProgram->getUniformLayout();
		customLayout.setValue(0, sizeof(CustomDemo::UniformBufferObject));
		customLayout.setTexture(1, { mCustomDemo->TestTexture1->getImageView(), mCustomDemo->TestTexture1->getSampler() });

		mCustomDemo->SceneVao = ObjInit::vertexArray();
		std::shared_ptr<VertexBufferVulkan> sceneVb = ObjInit::stagedVertexBuffer(
			mCustomDemo->SceneVertexType,
			mCustomDemo->SceneVertices.data(),
			(size_t) mCustomDemo->SceneVertexType * mCustomDemo->SceneVertices.size(),
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

		mCustomDemo->ScenePipelineInfo = std::make_unique<GraphicsPipelineInstanceInfo>(
			mCustomDemo->SceneVertexType,
			*mCustomDemo->SceneShaderProgram,
			ERenderPass::APP_SCENE
		);
		mCustomDemo->UiPipelineInfo = std::make_unique<GraphicsPipelineInstanceInfo>(
			mCustomDemo->UiVertexType,
			*mCustomDemo->UiShaderProgram,
			ERenderPass::APP_UI
		);

		auto& context = Application::get().getRenderer().getContext();
		mCustomDemo->CustomSceneConveyer = context.createPipeline(
			mCustomDemo->ScenePipelineName,
			*mCustomDemo->ScenePipelineInfo
		);
		mCustomDemo->CustomUiConveyer = context.createPipeline(
			mCustomDemo->UiPipelineName,
			*mCustomDemo->UiPipelineInfo
		);
	}

	void EditorAppLogic::initObjModelsDemo() {
		for (const auto& filePath : mObjModelsDemo->ObjModelFilePaths) {
			mObjModelsDemo->LoadedModels.emplace_back(ModelVulkan::createModel(filePath));
		}

		
		//mObjModelsDemo->LoadedModels.emplace_back(ModelVulkan::createModel(mObjModelsDemo->ObjModelFilePaths[0]));

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
		
				//objModelsDemoAddRandomisedObject();
			}
		}

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
			mObjModelsDemo->SceneVertexType,
			*mObjModelsDemo->SceneShaderProgram,
			ERenderPass::APP_SCENE
		);
		mObjModelsDemo->SceneWireframePipelineInfo = std::make_unique<GraphicsPipelineInstanceInfo>(
			mObjModelsDemo->SceneVertexType,
			*mObjModelsDemo->SceneShaderProgram,
			ERenderPass::APP_SCENE
		);

		mObjModelsDemo->SceneWireframePipelineInfo->CullMode = VK_CULL_MODE_NONE;
		mObjModelsDemo->SceneWireframePipelineInfo->PolygonMode = VK_POLYGON_MODE_LINE;

		auto& context = Application::get().getRenderer().getContext();
		mObjModelsDemo->ScenePipelineConveyer = context.createPipeline(
			mObjModelsDemo->ScenePipelineName,
			*mObjModelsDemo->ScenePipelineInfo
		);
		mObjModelsDemo->WireframePipelineConveyer = context.createPipeline(
			mObjModelsDemo->SceneWireframePipelineName,
			*mObjModelsDemo->SceneWireframePipelineInfo
		);
	}

	void EditorAppLogic::bouncingQuadsDemoAddRandomQuads(size_t count) {
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
					(static_cast<float>((rand() % 5000)) / 500.0f) - 1.0f, 0.8f
				},
				{mGridDemo->TestGridQuadSize[0], mGridDemo->TestGridQuadSize[1]},
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

	void EditorAppLogic::bouncingQaudsDemoPopQuads(size_t count) {
		const size_t size = mBouncingQuadDemo->BouncingQuads.size();
		if (count > size) {
			count = size;
		}

		for (int i = 0; i < count; i++) {
			mBouncingQuadDemo->BouncingQuads.pop_back();
			mBouncingQuadDemo->BouncingQuadVelocities.pop_back();
		}
	}

	void EditorAppLogic::objModelsDemoAddObject(
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

	void EditorAppLogic::imGuiDrawObjDemoItem(DOH::RenderableModelVulkan& model, const std::string& uniqueImGuiId) {
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

	void EditorAppLogic::imGuiDrawTextureViewerWindow(ImGuiTextureViewerWindow& textureViewer) {
		auto& imGuiWrapper = GET_RENDERER.getContext().getImGuiWrapper();

		const std::string title = "Texture ID: " + std::to_string(textureViewer.Texture.getId());
		const int paddingWidth = 5;
		const int paddingHeight = 5;

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(paddingWidth, paddingHeight));

		if (ImGui::Begin(
			title.c_str(),
			&textureViewer.Display,
			ImGuiWindowFlags_HorizontalScrollbar
		)) {
			ImGui::Text("Size: %i, %i", textureViewer.Texture.getWidth(), textureViewer.Texture.getHeight());
			ImGui::Checkbox("Match To Window Size", &textureViewer.MatchWindowSize);
			imGuiDisplayHelpTooltip("Match texture displayed width & height to window. Still affected by scale.");
			ImGui::DragFloat("Scale", &textureViewer.Scale, 0.005f, 0.01f, 2.0f);

			glm::vec2 displaySize = { 0.0f, 0.0f };

			if (textureViewer.MatchWindowSize) {
				ImVec2 regionAvail = ImGui::GetContentRegionAvail();

				displaySize.x = regionAvail.x * textureViewer.Scale;
				displaySize.y = regionAvail.y * textureViewer.Scale;
			} else {
				displaySize.x = textureViewer.Texture.getWidth() * textureViewer.Scale;
				displaySize.y = textureViewer.Texture.getHeight() * textureViewer.Scale;
			}

			if (displaySize.y > 0.0f) {
				imGuiWrapper.drawTexture(textureViewer.Texture, displaySize);
			}
		}

		ImGui::PopStyleVar(1);
		ImGui::End();
	}

	void EditorAppLogic::imGuiRemoveHiddenTextureViewerWindows() {
		for (auto& textureViewer : mEditorSettings->TextureViewerWindows) {
			if (!textureViewer.second.Display) {
				mEditorSettings->TextureViewerWindows.erase(textureViewer.first);
			}
		}
	}

	void EditorAppLogic::imGuiDisplayHelpTooltip(const char* message) {
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

	void EditorAppLogic::imGuiBulletTextWrapped(const char* message) {
		ImGui::Bullet();
		ImGui::SameLine();
		ImGui::TextWrapped(message);
	}

	void EditorAppLogic::imGuiPrintDrawCallTableColumn(const char* pipelineName, uint32_t drawCount) {
		//IMPORTANT:: Assumes already inside the Draw Call Count debug info table
		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		ImGui::Text(pipelineName);
		ImGui::TableNextColumn();
		ImGui::Text("%i", drawCount);
	}

	void EditorAppLogic::imGuiPrintMat4x4(const glm::mat4x4& mat, const char* name) {
		ImGui::Text(name);
		ImGui::BeginTable(name, 4);
		ImGui::TableSetupColumn("0");
		ImGui::TableSetupColumn("1");
		ImGui::TableSetupColumn("2");
		ImGui::TableSetupColumn("3");
		ImGui::TableHeadersRow();
		ImGui::TableSetColumnIndex(0);

		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		ImGui::Text("%f", mat[0][0]);
		ImGui::TableNextColumn();
		ImGui::Text("%f", mat[0][1]);
		ImGui::TableNextColumn();
		ImGui::Text("%f", mat[0][2]);
		ImGui::TableNextColumn();
		ImGui::Text("%f", mat[0][3]);

		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		ImGui::Text("%f", mat[1][0]);
		ImGui::TableNextColumn();
		ImGui::Text("%f", mat[1][1]);
		ImGui::TableNextColumn();
		ImGui::Text("%f", mat[1][2]);
		ImGui::TableNextColumn();
		ImGui::Text("%f", mat[1][3]);
		
		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		ImGui::Text("%f", mat[2][0]);
		ImGui::TableNextColumn();
		ImGui::Text("%f", mat[2][1]);
		ImGui::TableNextColumn();
		ImGui::Text("%f", mat[2][2]);
		ImGui::TableNextColumn();
		ImGui::Text("%f", mat[2][3]);

		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		ImGui::Text("%f", mat[3][0]);
		ImGui::TableNextColumn();
		ImGui::Text("%f", mat[3][1]);
		ImGui::TableNextColumn();
		ImGui::Text("%f", mat[3][2]);
		ImGui::TableNextColumn();
		ImGui::Text("%f", mat[3][3]);

		ImGui::EndTable();
	}
}
