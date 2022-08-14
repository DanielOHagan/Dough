#include "testGame/TG_AppLogic.h"

#include "dough/rendering/ObjInit.h"
#include "dough/application/Application.h"
#include "dough/input/InputCodes.h"
#include "dough/Logging.h"

#include <imgui/imgui.h>

#define GET_RENDERER Application::get().getRenderer()

namespace TG {

	TG_AppLogic::TG_AppLogic()
	:	IApplicationLogic()
	{}

	void TG_AppLogic::init(float aspectRatio) {
		mGridDemo.mTestGridMaxQuadCount = Renderer2dStorageVulkan::MAX_BATCH_COUNT_QUAD * Renderer2dStorageVulkan::BATCH_MAX_GEO_COUNT_QUAD;

		initDemos();

		setUiProjection(aspectRatio);

		TG_mOrthoCameraController = std::make_shared<TG_OrthoCameraController>(aspectRatio);
		mPerspectiveCameraController = std::make_shared<TG_PerspectiveCameraController>(aspectRatio);

		mPerspectiveCameraController->setPosition({ 0.0f, 0.0f, 5.0f });

		mImGuiSettings.UseOrthographicCamera = false;
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

	void TG_AppLogic::render() {
		RendererVulkan& renderer = GET_RENDERER;
		RenderingContextVulkan& context = renderer.getContext();
		Renderer2dVulkan& renderer2d = context.getRenderer2d();

		renderer.beginScene(
			mImGuiSettings.UseOrthographicCamera ?
				TG_mOrthoCameraController->getCamera() : mPerspectiveCameraController->getCamera()
		);

		if (mCustomDemo.RenderScene) {
			context.addRenderableToSceneDrawList(mCustomDemo.mScenePipelineName, mCustomDemo.mSceneRenderable);
		}

		if (mObjModelsDemo.Render) {
			PipelineRenderableConveyer objModelConveyer = context.createPipelineConveyer(
				SwapChainVulkan::ERenderPassType::SCENE,
				mObjModelsDemo.mScenePipelineName
			);
			PipelineRenderableConveyer objWireframeConveyer = context.createPipelineConveyer(
				SwapChainVulkan::ERenderPassType::SCENE,
				mObjModelsDemo.mSceneWireframePipelineName
			);

			if (objModelConveyer.isValid()) {
				for (auto& obj : mObjModelsDemo.mRenderableObjects) {
					if (obj->Render || mObjModelsDemo.RenderAllStandard) {
						objModelConveyer.addRenderable(obj);
					}
					if (obj->RenderWireframe || mObjModelsDemo.RenderAllWireframe) {
						objWireframeConveyer.addRenderable(obj);
					}
				}
			}
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
			context.addRenderableToUiDrawList(mCustomDemo.mUiPipelineName, mCustomDemo.mUiRenderable);
		}
		renderer.endUi();
	}

	void TG_AppLogic::imGuiRender(float delta) {
		if (mImGuiSettings.RenderDebugWindow) {
			imGuiRenderDebugWindow(delta);
		}
	}

	void TG_AppLogic::imGuiRenderDebugWindow(float delta) {
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

				const static float quitHoldTimeRequired = 1.5f;
				static float quitButtonHoldTime = 0.0f;
				if (ImGui::Button("Quit Application") || ImGui::IsItemActive()) {
					quitButtonHoldTime += delta;

					//Close app when releasing mouse button after holding for required time
					if (quitButtonHoldTime >= quitHoldTimeRequired && ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
						Application::get().stop();
					}
				} else {
					quitButtonHoldTime = 0.0f;
				}
				//Draw a rectangle to show how long to hold for
				if (ImGui::IsItemHovered() || ImGui::IsItemActive()) {
					ImGui::BeginTooltip();

					ImGui::PushStyleColor(ImGuiCol_PlotHistogram, { 1.0f, 0.0f, 0.0f, 1.0f });
					ImGui::ProgressBar((quitButtonHoldTime / quitHoldTimeRequired), {200.0f, 20.0f});
					ImGui::PopStyleColor(1);

					ImGui::EndTooltip();
				}

				mImGuiSettings.ApplicationCollapseMenuOpen = true;
			} else {
				mImGuiSettings.ApplicationCollapseMenuOpen = false;
			}

			ImGui::SetNextItemOpen(mImGuiSettings.RenderingCollapseMenuOpen);
			if (ImGui::CollapsingHeader("Rendering")) {
				AppDebugInfo& debugInfo = Application::get().getDebugInfo();

				const double frameTime = debugInfo.LastUpdateTimeMillis + debugInfo.LastRenderTimeMillis;

				if (debugInfo.FrameTimeIndex == AppDebugInfo::FrameTimesCount) {
					debugInfo.FrameTimeIndex = 0;
					debugInfo.FrameTimesFullArray = true;
				}
				debugInfo.FrameTimesMillis[debugInfo.FrameTimeIndex] = (float) frameTime;
				debugInfo.FrameTimeIndex++;

				ImGui::PlotLines(
					"Frame Times (ms)",
					debugInfo.FrameTimesMillis,
					debugInfo.FrameTimesFullArray ? AppDebugInfo::FrameTimesCount : debugInfo.FrameTimeIndex
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
				for (const auto& [name, pipeline] : renderer.getContext().getSceneGraphicsPipelines()) {
					imGuiPrintDrawCallTableColumn(name.c_str(), pipeline->getVaoDrawCount());
				}
				//Total scene pipeline draw calls
				imGuiPrintDrawCallTableColumn("Total Scene", debugInfo.SceneDrawCalls);
				ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg1, ImColor::ImColor(125, 125, 125, 90));
				//Individual UI pipeline draw calls
				for (const auto& [name, pipeline] : renderer.getContext().getUiGraphicsPipelines()) {
					imGuiPrintDrawCallTableColumn(name.c_str(), pipeline->getVaoDrawCount());
				}
				//Total UI pipeline draw calls
				imGuiPrintDrawCallTableColumn("Total UI", debugInfo.UiDrawCalls);
				ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg1, ImColor::ImColor(125, 125, 125, 90));
				imGuiPrintDrawCallTableColumn("Quad Batch", debugInfo.BatchRendererDrawCalls);
				imGuiPrintDrawCallTableColumn("Total", debugInfo.TotalDrawCalls);
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
				if (ImGui::Button("Disable all demos")) {
					mGridDemo.Render = false;
					mGridDemo.Update = false;

					mBouncingQuadDemo.Render = false;
					mBouncingQuadDemo.Update = false;

					mCustomDemo.RenderScene = false;
					mCustomDemo.RenderUi = false;
					mCustomDemo.Update = false;

					mObjModelsDemo.Render = false;
					mObjModelsDemo.Update = false;
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

					const auto& atlas = renderer.getContext().getRenderer2d().getStorage().getTestTextureAtlas();

					int tempTestTextureRowOffset = mGridDemo.mTestTexturesRowOffset;
					if (ImGui::InputInt("Test Texture Row Offset", &tempTestTextureRowOffset)) {
						mGridDemo.mTestTexturesRowOffset = tempTestTextureRowOffset < 0 ? 0 : tempTestTextureRowOffset % atlas->getRowCount();
					}
					int tempTestTextureColOffset = mGridDemo.mTestTexturesColumnOffset;
					if (ImGui::InputInt("Test Texture Col Offset", &tempTestTextureColOffset)) {
						mGridDemo.mTestTexturesColumnOffset = tempTestTextureColOffset < 0 ? 0 : tempTestTextureColOffset % atlas->getColCount();
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
					auto& demo = mBouncingQuadDemo;
					if (ImGui::InputInt((std::string(ImGuiWrapper::EMPTY_LABEL) + "Add").c_str(), &demo.AddNewQuadCount, 5, 5)) {
						if (demo.AddNewQuadCount < 0) {
							demo.AddNewQuadCount = 0;
						} else if (demo.AddNewQuadCount > 1000) {
							demo.AddNewQuadCount = 1000;
						}
					}
					ImGui::SameLine();
					if (ImGui::Button("Add Quads")) {
						bouncingQuadsDemoAddRandomQuads(demo.AddNewQuadCount);
					}
					if (ImGui::InputInt((std::string(ImGuiWrapper::EMPTY_LABEL) + "Pop").c_str(), &demo.PopQuadCount, 5, 5)) {
						if (demo.PopQuadCount < 0) {
							demo.PopQuadCount = 0;
						}
					}
					ImGui::SameLine();
					if (ImGui::Button("Pop Quads")) {
						bouncingQaudsDemoPopQuads(demo.PopQuadCount);
					}
					if (ImGui::Button("Clear Quads")) {
						bouncingQaudsDemoPopQuads(static_cast<int>(demo.mBouncingQuads.size()));
					}

					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("Custom")) {
					ImGui::Checkbox("Render Scene", &mCustomDemo.RenderScene);
					ImGui::Checkbox("Render UI", &mCustomDemo.RenderUi);
					ImGui::Checkbox("Update", &mCustomDemo.Update);

					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("Obj Models")) {
					auto& renderables = mObjModelsDemo.mRenderableObjects;
					auto& demo = mObjModelsDemo;

					ImGui::Checkbox("Render", &demo.Render);
					ImGui::Checkbox("Update", &demo.Update);
					ImGui::Text("Object Count: %i", renderables.size());
					if (ImGui::InputInt((std::string(ImGuiWrapper::EMPTY_LABEL) + "Add").c_str(), &demo.AddNewObjectsCount, 5, 5)) {
						if (demo.AddNewObjectsCount < 0) {
							demo.AddNewObjectsCount = 0;
						} else if (demo.AddNewObjectsCount > 1000) {
							demo.AddNewObjectsCount = 1000;
						}
					}
					ImGui::SameLine();
					if (ImGui::Button("Add Object")) {
						for (int i = 0; i < demo.AddNewObjectsCount; i++) {
							objModelsDemoAddRandomisedObject();
						}
					}
					if (ImGui::InputInt((std::string(ImGuiWrapper::EMPTY_LABEL) + "Pop").c_str(), &demo.PopObjectsCount, 5, 5)) {
						if (demo.PopObjectsCount < 0) {
							demo.PopObjectsCount = 0;
						}
					}
					ImGui::SameLine();
					if (ImGui::Button("Pop Object")) {
						//Max out pop count to renderables list size
						const int size = static_cast<int>(demo.mRenderableObjects.size());
						const int popCount = demo.PopObjectsCount > size ? size : demo.PopObjectsCount;

						for (int i = 0; i < popCount; i++) {
							demo.mRenderableObjects.pop_back();
						}
					}
					ImGui::Checkbox("Display Renderable Models List", &mImGuiSettings.RenderObjModelsList);
					if (ImGui::Button("Clear Objects")) {
						renderables.clear();
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
					imGuiDisplayHelpTooltip("NOTE:: Changes top/bottom & left/right orthographic perspective, does not change the Z clipping range");
					ImGui::BulletText("Right Click Drag Camera");
					imGuiDisplayHelpTooltip("NOTE:: Drag doesn't work properly at all update rates");
					
					ImGui::Text("Camera Position");
					const auto& pos = TG_mOrthoCameraController->getPosition();
					float tempPos[3] = {pos.x, pos.y, pos.z};
					if (ImGui::DragFloat3("Pos", tempPos)) {
						TG_mOrthoCameraController->setPosition({ tempPos[0], tempPos[1], tempPos[2] });
					}
					ImGui::Text("Zoom: %f", TG_mOrthoCameraController->getZoomLevel());
					imGuiDisplayHelpTooltip("Higher is more \"zoomed in\" and lower is more \"zoomed out\"");
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
					TG_mOrthoCameraController->setPosition({ 0.0f, 0.0f, 1.0f });
					TG_mOrthoCameraController->setZoomLevel(1.0f);
				}
				ImGui::SameLine();
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

		if (ImGui::BeginTabItem("Rendering Device Info")) {
			const auto& deviceInfo = renderer.getContext().getRenderingDeviceInfo();
			ImGui::Text("Vulkan API Version: ");
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

		if (mImGuiSettings.RenderObjModelsList) {
			const auto& renderables = mObjModelsDemo.mRenderableObjects;
			if (ImGui::Begin("Renderable Models List")) {
				ImGui::Checkbox("Render All", &mObjModelsDemo.RenderAllStandard);
				ImGui::SameLine();
				ImGui::Checkbox("Render All Wireframe", &mObjModelsDemo.RenderAllWireframe);

				uint32_t objectIndex = 0;
				for (auto objItr = renderables.begin(); objItr != renderables.end(); objItr++) {
					const std::string uniqueImGuiId = "##" + std::to_string(objectIndex);
					if (ImGui::BeginCombo(("Obj Model" + uniqueImGuiId).c_str(), objItr->get()->getName().c_str())) {
						int modelFilePathIndex = -1;
						for (const auto& filePath : mObjModelsDemo.mObjModelFilePaths) {
							bool selected = false;
							modelFilePathIndex++;
							if (ImGui::Selectable((filePath.c_str() + uniqueImGuiId).c_str(), &selected)) {
								objItr->get()->Model = mObjModelsDemo.mLoadedModels[modelFilePathIndex];
								objItr->get()->Name = mObjModelsDemo.mObjModelFilePaths[modelFilePathIndex];
								break;
							}
						}
						ImGui::EndCombo();
					}
					ImGui::Checkbox(("Render" + uniqueImGuiId).c_str(), &objItr->get()->Render);
					ImGui::SameLine();
					ImGui::Checkbox(("Wireframe" + uniqueImGuiId).c_str(), &objItr->get()->RenderWireframe);

					//Add temp array and set -1 > rotation < 361 to allow for "infinite" drag
					TransformationData& transformation = *objItr->get()->Transformation;
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

					if (transformed && mObjModelsDemo.Update) {
						transformation.updateTranslationMatrix();
					}

					objectIndex++;
				}
			}
			ImGui::End();
		}

		//DEBUG:: show ImGui stack info, can be helpful when debugging ImGui menus
		//ImGui::ShowStackToolWindow();
	}

	void TG_AppLogic::close() {
		RendererVulkan& renderer = GET_RENDERER;
		
		//Custom demo
		renderer.closeGpuResource(mCustomDemo.mSceneShaderProgram);
		renderer.closeGpuResource(mCustomDemo.mSceneVao);
		renderer.closeGpuResource(mCustomDemo.mUiShaderProgram);
		renderer.closeGpuResource(mCustomDemo.mUiVao);
		renderer.closeGpuResource(mCustomDemo.mTestTexture1);
		renderer.closeGpuResource(mCustomDemo.mTestTexture2);

		//Obj Models Demo
		for (const auto& model : mObjModelsDemo.mLoadedModels) {
			renderer.closeGpuResource(model);
		}
		renderer.closeGpuResource(mObjModelsDemo.mSceneShaderProgram);

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
		initObjModelsDemo();

		context.createPipeline(
			mCustomDemo.mScenePipelineName,
			*mCustomDemo.mScenePipelineInfo
		);
		context.createPipeline(
			mCustomDemo.mUiPipelineName,
			*mCustomDemo.mUiPipelineInfo
		);
		context.createPipeline(
			mObjModelsDemo.mScenePipelineName,
			*mObjModelsDemo.mScenePipelineInfo
		);
		context.createPipeline(
			mObjModelsDemo.mSceneWireframePipelineName,
			*mObjModelsDemo.mSceneWireframePipelineInfo
		);

		context.createPipelineUniformObjects();
	}

	void TG_AppLogic::populateTestGrid(int width, int height) {
		const auto& storage = GET_RENDERER.getContext().getRenderer2d().getStorage();
		const auto& atlas = storage.getTestTextureAtlas();

		mGridDemo.mTexturedTestGrid.clear();
		mGridDemo.mTexturedTestGrid.resize(storage.getQuadBatchTextureArray().getTextureSlots().size());
		const uint32_t textureSlot = storage.getQuadBatchTextureArray().getTextureSlotIndex(storage.getTestTextureAtlas()->getId());

		uint32_t index = 0;
		for (float y = 0.0f; y < height; y++) {
			for (float x = 0.0f; x < width; x++) {
				mGridDemo.mTexturedTestGrid[textureSlot].push_back({
					{x * mGridDemo.mTestGridQuadGapSize[0], y * mGridDemo.mTestGridQuadGapSize[1], 0.5f},
					{mGridDemo.mTestGridQuadSize[0], mGridDemo.mTestGridQuadSize[1]},
					{0.0f, 1.0f, 1.0f, 1.0f},
					0.0f,
					atlas,
					atlas->getInnerTextureCoords(
						(uint32_t) x + mGridDemo.mTestTexturesRowOffset,
						(uint32_t) y + mGridDemo.mTestTexturesColumnOffset
					)
				});
				index++;
			}
		}
	}

	void TG_AppLogic::initBouncingQuadsDemo() {
		bouncingQuadsDemoAddRandomQuads(1000);
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

		mCustomDemo.mSceneVao = ObjInit::vertexArray();
		std::shared_ptr<VertexBufferVulkan> sceneVb = ObjInit::stagedVertexBuffer(
			mCustomDemo.mSceneVertexType,
			mCustomDemo.mSceneVertices.data(),
			(size_t) mCustomDemo.mSceneVertexType * mCustomDemo.mSceneVertices.size(),
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		);
		mCustomDemo.mSceneVao->addVertexBuffer(sceneVb);
		std::shared_ptr<IndexBufferVulkan> sceneIb = ObjInit::stagedIndexBuffer(
			mCustomDemo.indices.data(),
			sizeof(uint32_t) * mCustomDemo.indices.size()
		);
		mCustomDemo.mSceneVao->setDrawCount(static_cast<uint32_t>(mCustomDemo.indices.size()));
		mCustomDemo.mSceneVao->setIndexBuffer(sceneIb);
		mCustomDemo.mSceneRenderable = std::make_shared<SimpleRenderable>(mCustomDemo.mSceneVao);

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
		mCustomDemo.mUiRenderable = std::make_shared<SimpleRenderable>(mCustomDemo.mUiVao);

		mCustomDemo.mScenePipelineInfo = std::make_unique<GraphicsPipelineInstanceInfo>(
			mCustomDemo.mSceneVertexType,
			*mCustomDemo.mSceneShaderProgram,
			SwapChainVulkan::ERenderPassType::SCENE
		);
		mCustomDemo.mUiPipelineInfo = std::make_unique<GraphicsPipelineInstanceInfo>(
			mCustomDemo.mUiVertexType,
			*mCustomDemo.mUiShaderProgram,
			SwapChainVulkan::ERenderPassType::APP_UI
		);
	}

	void TG_AppLogic::initObjModelsDemo() {
		for (const auto& filePath : mObjModelsDemo.mObjModelFilePaths) {
			mObjModelsDemo.mLoadedModels.push_back(ModelVulkan::createModel(filePath));
		}

		const float padding = 0.5f;
		for (uint32_t x = 0; x < 10; x++) {
			for (uint32_t y = 0; y < 10; y++) {
				//Add an object with a position based off of x & y value from loop, creates a grid like result
				const uint32_t modelIndex = rand() % mObjModelsDemo.mLoadedModels.size();
				std::shared_ptr<TransformationData> transform = std::make_shared<TransformationData>(
					glm::vec3(
						((float) x + padding) * 3.0f,
						((float) y + padding) * 3.0f,
						(float) (rand() % 10)
					),
					glm::vec3(
						(float) (rand() % 360),
						(float) (rand() % 360),
						(float) (rand() % 360)
					),
					1.0f
				);
				transform->updateTranslationMatrix();
				
				mObjModelsDemo.mRenderableObjects.push_back(std::make_shared<RenderableModelVulkan>(
					mObjModelsDemo.mObjModelFilePaths[modelIndex],
					mObjModelsDemo.mLoadedModels[modelIndex],
					transform
				));

				//objModelsDemoAddRandomisedObject();
			}
		}

		mObjModelsDemo.mSceneShaderProgram = ObjInit::shaderProgram(
			ObjInit::shader(
				EShaderType::VERTEX,
				mObjModelsDemo.flatColourShaderVertPath
			),
			ObjInit::shader(
				EShaderType::FRAGMENT,
				mObjModelsDemo.flatColourShaderFragPath
			)
		);
		ShaderUniformLayout& cubeLayout = mObjModelsDemo.mSceneShaderProgram->getUniformLayout();
		cubeLayout.setValue(0, sizeof(ObjModelsDemo::UniformBufferObject));
		//Push constant for transformation matrix
		cubeLayout.addPushConstant(VK_SHADER_STAGE_VERTEX_BIT, sizeof(glm::mat4x4));

		mObjModelsDemo.mScenePipelineInfo = std::make_unique<GraphicsPipelineInstanceInfo>(
			mObjModelsDemo.mSceneVertexType,
			*mObjModelsDemo.mSceneShaderProgram,
			SwapChainVulkan::ERenderPassType::SCENE
		);
		mObjModelsDemo.mSceneWireframePipelineInfo = std::make_unique<GraphicsPipelineInstanceInfo>(
			mObjModelsDemo.mSceneVertexType,
			*mObjModelsDemo.mSceneShaderProgram,
			SwapChainVulkan::ERenderPassType::SCENE
		);

		mObjModelsDemo.mSceneWireframePipelineInfo->CullMode = VK_CULL_MODE_NONE;
		mObjModelsDemo.mSceneWireframePipelineInfo->PolygonMode = VK_POLYGON_MODE_LINE;
	}

	void TG_AppLogic::bouncingQuadsDemoAddRandomQuads(size_t count) {
		const auto& atlas = GET_RENDERER.getContext().getRenderer2d().getStorage().getTestTextureAtlas();

		//Stop quad count going over MaxCount
		if (count + mBouncingQuadDemo.mBouncingQuads.size() > mBouncingQuadDemo.MaxBouncingQuadCount) {
			count = mBouncingQuadDemo.MaxBouncingQuadCount - mBouncingQuadDemo.mBouncingQuads.size();
		}

		for (int i = 0; i < count; i++) {
			mBouncingQuadDemo.mBouncingQuads.push_back({
				//Semi-random values for starting position, velocitiy and assigned texture
				{((float)(rand() % 5000) / 500.0f) - 1.0f,((float)(rand() % 5000) / 500.0f) - 1.0f, 0.8f},
				{mGridDemo.mTestGridQuadSize[0], mGridDemo.mTestGridQuadSize[1]},
				{0.0f, 1.0f, 1.0f, 1.0f},
				0.0f,
				//Texture atlas
				atlas,
				
				//Single inner texture
				atlas->getInnerTextureCoords(
					rand() % atlas->getRowCount(),
					rand() % atlas->getColCount()
				)
				//atlas->getInnerTextureCoords(4, 4)
				
				//Display mutliple inner rows/columns (may result in two 0 values passed which returns all 0 coords)
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
			mBouncingQuadDemo.mBouncingQuadVelocities.push_back({ (float)(rand() % 800) / 60.0f, (float)(rand() % 800) / 60.0f });
		}
	}

	void TG_AppLogic::bouncingQaudsDemoPopQuads(size_t count) {
		const size_t size = mBouncingQuadDemo.mBouncingQuads.size();
		if (count > size) {
			count = size;
		}

		for (int i = 0; i < count; i++) {
			mBouncingQuadDemo.mBouncingQuads.pop_back();
			mBouncingQuadDemo.mBouncingQuadVelocities.pop_back();
		}
	}

	void TG_AppLogic::objModelsDemoAddObject(
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

		mObjModelsDemo.mRenderableObjects.push_back(std::make_shared<RenderableModelVulkan>(
			mObjModelsDemo.mObjModelFilePaths[modelIndex],
			mObjModelsDemo.mLoadedModels[modelIndex],
			transform
		));
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

	void TG_AppLogic::imGuiBulletTextWrapped(const char* message) {
		ImGui::Bullet();
		ImGui::SameLine();
		ImGui::TextWrapped(message);
	}

	void TG_AppLogic::imGuiPrintDrawCallTableColumn(const char* pipelineName, uint32_t drawCount) {
		//IMPORTANT:: Assumes already inside the Draw Call Count debug info table
		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		ImGui::Text(pipelineName);
		ImGui::TableNextColumn();
		ImGui::Text("%i", drawCount);
	}

	void TG_AppLogic::imGuiPrintMat4x4(const glm::mat4x4& mat, const char* name) {
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
