#include "editor/EditorAppLogic.h"

#include "dough/rendering/ObjInit.h"
#include "dough/application/Application.h"
#include "dough/input/InputCodes.h"
#include "dough/Logging.h"

#include <imgui/imgui.h>

#define GET_RENDERER Application::get().getRenderer()

namespace DOH::EDITOR {

	EditorAppLogic::EditorAppLogic(std::shared_ptr<IApplicationLogic> innerAppLogic)
	:	IApplicationLogic(),
		mInnerAppLogic(innerAppLogic),
		mInnerAppState(EInnerAppState::STOPPED)
	{
		if (mInnerAppLogic == nullptr) {
			THROW("EditorAppLogic: Inner app was nullptr");
		}

		mInnerAppTimer = std::make_unique<PausableTimer>();

		mEditorSettings = std::make_unique<EditorSettings>();
	}

	void EditorAppLogic::init(float aspectRatio) {
		mOrthoCameraController = std::make_shared<EditorOrthoCameraController>(aspectRatio);
		mPerspectiveCameraController = std::make_shared<EditorPerspectiveCameraController>(aspectRatio);

		mPerspectiveCameraController->setPosition({ 0.0f, 0.0f, 5.0f });

		mEditorSettings->UseOrthographicCamera = false;
		mEditorSettings->TextureViewerWindows = {};

		mInnerAppLogic->init(aspectRatio);
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
		if (mInnerAppState == EInnerAppState::PLAYING) {
			mInnerAppLogic->update(delta);
			return;
		}
	}

	void EditorAppLogic::render() {
		RendererVulkan& renderer = GET_RENDERER;

		//Begin scene sets the primary camera that is to be used to render the scene.
		// If one isn't set in the inner app then one of the Editor's cameras is used.
		// 
		//TODO:: When there is a substantial demo that has its own camera do:
		//	IF using editor camera THEN beginScene(editorCamera) ELSE do nothing and assume inner app will beginScene(innerAppCamera)
		renderer.beginScene(
			mEditorSettings->UseOrthographicCamera ?
				mOrthoCameraController->getCamera() : mPerspectiveCameraController->getCamera()
		);

		mInnerAppLogic->render();
	}

	void EditorAppLogic::imGuiRender(float delta) {
		//NOTE:: ImGui Debug info windows
		//ImGui::ShowMetricsWindow();
		//ImGui::ShowStackToolWindow();
		//ImGui::ShowDemoWindow();

		if (mEditorSettings->RenderDebugWindow) {
			imGuiRenderDebugWindow(delta);
		}

		if (mEditorSettings->InnerAppEditorWindowDisplay) {
			mInnerAppLogic->imGuiRender(delta);
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
				MonoSpaceTextureAtlas& atlas = *renderer.getContext().getRenderer2d().getStorage().getTestTextureAtlas();

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

				ImGui::Checkbox("Display Inner App Editor Window", &mEditorSettings->InnerAppEditorWindowDisplay);
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

			ImGui::Text("Vulkan Validation Layers Enabled: ");
			ImGui::SameLine();
			ImGui::Text(deviceInfo.ValidationLayersEnabled ? "TRUE" : "FALSE");

			//TODO:: list all currently used validation layers?

			//TODO:: vulkan extensions?

			
			ImGui::Text("Rendering Device Name: ");
			ImGui::SameLine();
			ImGui::Text(deviceInfo.DeviceName.c_str());

			//TODO:: device driver version, these are vendor specific so need to implement a way of extracting it from AMD/NVIDIA/other

			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();
		ImGui::End();

		for (auto& textureViewer : mEditorSettings->TextureViewerWindows) {
			if (textureViewer.second.Display) {
				imGuiDrawTextureViewerWindow(textureViewer.second);
			}
		}
	}

	void EditorAppLogic::close() {
		mInnerAppLogic->close();
	}

	void EditorAppLogic::onResize(float aspectRatio) {
		mOrthoCameraController->onViewportResize(aspectRatio);
		mInnerAppLogic->onResize(aspectRatio);
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
