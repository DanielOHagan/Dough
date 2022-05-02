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
		mUiProjMat(1.0f),
		mTestGridMaxQuadCount(
			Renderer2dStorageVulkan::MAX_BATCH_COUNT_QUAD *
			Renderer2dStorageVulkan::BATCH_MAX_GEO_COUNT_QUAD
		) //Set to max supported by batch renderer as excess quads are truncated
	{}

	void TG_AppLogic::init(float aspectRatio) {
		setUiProjection(aspectRatio);
		initScene(aspectRatio);
		initUi();

		TG_mOrthoCameraController = std::make_shared<TG::TG_OrthoCameraController>(aspectRatio);
		mPerspectiveCameraController = std::make_shared<TG_PerspectiveCameraController>(aspectRatio);
	}

	void TG_AppLogic::update(float delta) {
		if (Input::isKeyPressed(DOH_KEY_F1)) {
			mImGuiSettings.mRenderDebugWindow = true;
		}

		if (mImGuiSettings.mUseOrthographicCamera) {
			TG_mOrthoCameraController->onUpdate(delta);
		} else {
			mPerspectiveCameraController->onUpdate(delta);
		}

		if (mImGuiSettings.mRenderBatchQuadScene) {
			populateTestGrid(mTestGridSize[0], mTestGridSize[1]);
		}
	}

	void TG_AppLogic::render() {
		RendererVulkan& renderer = GET_RENDERER;

		renderer.beginScene(
			mImGuiSettings.mUseOrthographicCamera ?
				TG_mOrthoCameraController->getCamera() : mPerspectiveCameraController->getCamera()
		);
		if (mImGuiSettings.mRenderScene) {
			renderer.getContext().addVaoToSceneDrawList(*mSceneVertexArray);
		}

		if (mImGuiSettings.mRenderBatchQuadScene) {
			for (std::vector<Quad>& sameTexturedQuads : mTexturedTestGrid) {
				//Different ways of drawing quads (inserting into RenderBatches),
				// shouldn't cause too much of a performance difference (quick testing showed 0-3 fps drop max)
				
				//for (Quad& quad : sameTexturedQuads) {
				//	renderer.getContext().getRenderer2d().drawQuadScene(quad);
				//}
				//for (Quad& quad : sameTexturedQuads) {
				//	renderer.getContext().getRenderer2d().drawQuadTexturedScene(quad);
				//}
				//renderer.getContext().getRenderer2d().drawQuadArrayScene(sameTexturedQuads);
				//renderer.getContext().getRenderer2d().drawQuadArrayTexturedScene(sameTexturedQuads);
				renderer.getContext().getRenderer2d().drawQuadArraySameTextureScene(sameTexturedQuads);
			}
		}
		renderer.endScene();

		renderer.beginUi(mUiProjMat);
		if (mImGuiSettings.mRenderUi) {
			renderer.getContext().addVaoToUiDrawList(*mUiVao);
		}
		renderer.endUi();
	}

	void TG_AppLogic::imGuiRender() {
		if (mImGuiSettings.mRenderDebugWindow) {
			imGuiRenderDebugWindow();
		}

		if (mImGuiSettings.mRenderToDoListWindow) {
			imGuiRenderToDoListWindow();
		}
	}

	void TG_AppLogic::imGuiRenderDebugWindow() {
		//TODO:: Clean up basic layout and styling, maybe add a few imgui helper functions

		RendererVulkan& renderer = GET_RENDERER;

		ImGui::Begin("Debug Window");

		ImGui::SetNextItemOpen(mImGuiSettings.mApplicationCollapseMenuOpen);
		if (ImGui::CollapsingHeader("Application")) {
			ApplicationLoop& loop = Application::get().getLoop();
			Window& window = Application::get().getWindow();
			bool focused = Application::get().isFocused();

			ImGui::Text("Runtime: %fs", Time::convertMillisToSeconds(Application::get().getAppInfoTimer().getCurrentTickingTimeMillis()));
			ImGui::Text("Window Size: (%i, %i)", window.getWidth(), window.getHeight());
			bool displayModeWindowActive = window.getDisplayMode() == WindowDisplayMode::WINDOWED;
			if (
				ImGui::RadioButton("Windowed", displayModeWindowActive) &&
				window.getDisplayMode() != WindowDisplayMode::WINDOWED
			) {
				window.selectDisplayMode(WindowDisplayMode::WINDOWED);
			}
			bool displayModeBorderlessWindowActive = window.getDisplayMode() == WindowDisplayMode::BORDERLESS_FULLSCREEN;
			if (
				ImGui::RadioButton("Borderless Windowed", displayModeBorderlessWindowActive) &&
				window.getDisplayMode() != WindowDisplayMode::BORDERLESS_FULLSCREEN
			) {
				window.selectDisplayMode(WindowDisplayMode::BORDERLESS_FULLSCREEN);
			}
			bool displayModeFullscreenActive = window.getDisplayMode() == WindowDisplayMode::FULLSCREEN;
			if (
				ImGui::RadioButton("Fullscreen", displayModeFullscreenActive) &&
				window.getDisplayMode() != WindowDisplayMode::FULLSCREEN
			) {
				window.selectDisplayMode(WindowDisplayMode::FULLSCREEN);
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

			bool runInBackground = loop.isRunningInBackground();
			if (ImGui::Checkbox("Run In Background", &runInBackground)) {
				loop.setRunInBackground(runInBackground);
			}

			ImGui::Checkbox("Render ToDo List Window", &mImGuiSettings.mRenderToDoListWindow);
			if (ImGui::Button("Stop Rendering All ImGui Windows")) {
				mImGuiSettings.mRenderDebugWindow = false;
				mImGuiSettings.mRenderToDoListWindow = false;
			}
			imGuiDisplayHelpTooltip("Once the debug window has stopped rendering, you can press F1 to start rendering again");

			if (ImGui::Button("Quit Application")) {
				Application::get().stop();
			}

			mImGuiSettings.mApplicationCollapseMenuOpen = true;
		} else {
			mImGuiSettings.mApplicationCollapseMenuOpen = false;
		}

		ImGui::SetNextItemOpen(mImGuiSettings.mRenderingCollapseMenuOpen);
		if (ImGui::CollapsingHeader("Rendering")) {
			ImGui::Checkbox("Render Scene", &mImGuiSettings.mRenderScene);
			ImGui::Checkbox("Render UI", &mImGuiSettings.mRenderUi);
			ImGui::Checkbox("Render Batch Quad Scene", &mImGuiSettings.mRenderBatchQuadScene);
			ImGui::Checkbox("Render Batch Quad UI", &mImGuiSettings.mRenderBatchQuadUi);
			//TODO:: ImGui::Checkbox("Render Wireframes", &mRenderWireframes);
			ImGui::Text("Quad Batch Max Size: %i", Renderer2dStorageVulkan::BatchSizeLimits::BATCH_MAX_GEO_COUNT_QUAD);
			ImGui::Text(
				"Quad Batch Count: %i of Max %i",
				renderer.getContext().getRenderer2d().getQuadBatchCount(),
				Renderer2dStorageVulkan::BatchSizeLimits::MAX_BATCH_COUNT_QUAD
			);
			uint32_t quadBatchIndex = 0;
			for (RenderBatchQuad& batch : renderer.getContext().getRenderer2d().getStorage().getQuadRenderBatches()) {
				//ImGui::Text("Batch: %i Geo Count: %i Texture Count: %i", index, batch.getGeometryCount(), batch.getTextureSlots().size());
				ImGui::Text("Batch: %i Geo Count: %i", quadBatchIndex, batch.getGeometryCount());
				quadBatchIndex++;
			}
			//TODO:: debug info when multiple texture arrays are supported
			//uint32_t texArrIndex = 0;
			//for (TextureArray& texArr : renderer.getContext().getRenderer2d().getStorage().getTextureArrays()) {
			//	ImGui::Text("Texture Array: %i Texture Count: %i", texArrIndex, texArr.getTextureSlots().size());
			//	texArrIndex++;
			//}
			ImGui::Text("Texture Array: %i Texture Count: %i", 1, renderer.getContext().getRenderer2d().getStorage().getTextureArray().getTextureSlots().size());
			if (ImGui::Button("Close All Empty Quad Batches")) {
				renderer.getContext().getRenderer2d().closeEmptyQuadBatches();
			}
			imGuiDisplayHelpTooltip("Close Empty Quad Batches. This can help clean-up when 1 or more batches have geo counts of 0");

			mImGuiSettings.mRenderingCollapseMenuOpen = true;
		} else {
			mImGuiSettings.mRenderingCollapseMenuOpen = false;
		}

		ImGui::SetNextItemOpen(mImGuiSettings.mSceneDataCollapseMenuOpen);
		if (ImGui::CollapsingHeader("Quad Test Grid")) {
			//Size of grid can be adjusted but it limited to a certain number of quads,
			// ImGui values are taken and checked to see if they are valid
			ImGui::Text("Test Grid Quad Count: %i of Max %i", mTestGridSize[0] * mTestGridSize[1], mTestGridMaxQuadCount);
			int tempTestGridSize[2] = { mTestGridSize[0], mTestGridSize[1] };
			if (ImGui::InputInt2("Test Grid Size", tempTestGridSize)) {
				if (tempTestGridSize[0] > 0 && tempTestGridSize[1] > 0) {
					const int tempGridQuadCount = tempTestGridSize[0] * tempTestGridSize[1];
					if (tempGridQuadCount <= mTestGridMaxQuadCount) {
						mTestGridSize[0] = tempTestGridSize[0];
						mTestGridSize[1] = tempTestGridSize[1];
					} else {
						LOG_WARN(
							"New grid size of " << tempTestGridSize[0] << "x" << tempTestGridSize[1] <<
							" (" << tempTestGridSize[0] * tempTestGridSize[1] <<
							") is too large, max quad count is " << mTestGridMaxQuadCount
						);
					}
				}
			}

			ImGui::DragFloat2("Test Grid Quad Size", mTestGridQuadSize, 0.001f, 0.01f, 0.5f);
			ImGui::DragFloat2("Test Grid Quad Gap Size", mTestGridQuadGapSize, 0.001f, 0.01f, 0.5f);
			//ImGui::DragFloat2("Test Grid Origin Pos", );
			//ImGui::Text("UI Quad Count: %i", renderer.getContext().getRenderer2d().getStorage().getUiQuadCount());

			//TOOD:: maybe have radio buttons for RenderStaticGrid or RenderDynamicGrid,
			//	static being the default values and dynamic being from the variables determined by ths menu
			// Maybe have the dynamic settings hidden unless dynamic is selected

			if (ImGui::Button("Reset Grid")) {
				mTestGridSize[0] = 10;
				mTestGridSize[1] = 10;
				mTestGridQuadSize[0] = 0.1f;
				mTestGridQuadSize[1] = 0.1f;
				mTestGridQuadGapSize[0] = mTestGridQuadSize[0] * 1.5f;
				mTestGridQuadGapSize[1] = mTestGridQuadSize[1] * 1.5f;
			}

			mImGuiSettings.mSceneDataCollapseMenuOpen = true;
		} else {
			mImGuiSettings.mSceneDataCollapseMenuOpen = false;
		}

		//TODO:: Use different formats, currently looks bad
		ImGui::SetNextItemOpen(mImGuiSettings.mCameraCollapseMenuOpen);
		if (ImGui::CollapsingHeader("Camera")) {
			const bool orthoCameraActive = mImGuiSettings.mUseOrthographicCamera;
			const bool perspectiveCameraActive = !orthoCameraActive;
			if (ImGui::RadioButton("Orthographic Camera", orthoCameraActive)) {
				mImGuiSettings.mUseOrthographicCamera = true;
			}
			if (ImGui::RadioButton("Perspective Camera", perspectiveCameraActive)) {
				mImGuiSettings.mUseOrthographicCamera = false;
			}
			ImGui::Text("Current Camera: ");
			ImGui::SameLine();
			ImGui::Text(mImGuiSettings.mUseOrthographicCamera ? "Orthographic" : "Perspective");
			if (mImGuiSettings.mUseOrthographicCamera) {
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
				mPerspectiveCameraController->setPosition({ 0.0f, 0.0f, 1.0f });
				mPerspectiveCameraController->setDirection({ 0.0f, 0.0f, 0.0f });
			}

			mImGuiSettings.mCameraCollapseMenuOpen = true;
		} else {
			mImGuiSettings.mCameraCollapseMenuOpen = false;
		}

		ImGui::End();
	}

	void TG_AppLogic::imGuiRenderToDoListWindow() {
		ImGui::Begin("Todo List");
		ImGui::Text("Things to do/fix in no particular order");

		if (ImGui::CollapsingHeader("Top Tier")) {
			ImGui::Bullet();
			ImGui::TextWrapped("Multiple texture support for dynamic batches");
			ImGui::Bullet();
			ImGui::TextWrapped("Some kind of text rendering");

			mImGuiSettings.mToDoListTopTierCollapseMenuOpen = true;
		} else {
			mImGuiSettings.mToDoListTopTierCollapseMenuOpen = false;
		}

		if (ImGui::CollapsingHeader("Mid Tier")) {
			ImGui::Bullet();
			ImGui::TextWrapped("Fix texture artifacts caused, I think, by casting float to int in shader for texture array index");
			ImGui::Bullet();
			ImGui::TextWrapped(
				"Have some way of batching textures so it's easier for multiple batches that use the same textures. Maybe start/end texture batch calls, this is because multiple batches will use the same textures so it's probably better to limit the number of uniform updates per frame. Currently it is being changed per batch."
			);
			ImGui::Bullet();
			ImGui::TextWrapped("Maybe have a \"Master Texture Batch\" or a \"Static\" and a \"Dynamic\" Texture Batch to manage how batches and textures are sorted for uniform and vertex input");

			mImGuiSettings.mToDoListMidTierCollapseMenuOpen = true;
		} else {
			mImGuiSettings.mToDoListMidTierCollapseMenuOpen = false;
		}

		if (ImGui::CollapsingHeader("Bottom Tier")) {
			ImGui::Bullet();
			ImGui::TextWrapped("Better Perspective camera and camera controller");
			ImGui::Bullet();
			ImGui::TextWrapped("Texture atlas");


			mImGuiSettings.mToDoListBottomTierCollapseMenuOpen = true;
		} else {
			mImGuiSettings.mToDoListBottomTierCollapseMenuOpen = false;
		}

		ImGui::End();
	}

	void TG_AppLogic::close() {
		RendererVulkan& renderer = GET_RENDERER;

		renderer.closeGpuResource(mSceneShaderProgram);
		renderer.closeGpuResource(mSceneVertexArray);
		renderer.closeGpuResource(mUiShaderProgram);
		renderer.closeGpuResource(mUiVao);
		renderer.closeGpuResource(mTestTexture1);
		renderer.closeGpuResource(mTestTexture2);

		//for (std::shared_ptr<TextureVulkan> texture : mTestTextures) {
		//	renderer.closeGpuResource(texture);
		//}
	}

	void TG_AppLogic::onResize(float aspectRatio) {
		TG_mOrthoCameraController->onViewportResize(aspectRatio);
		setUiProjection(aspectRatio);
	}

	void TG_AppLogic::initScene(float aspectRatio) {
		mTestTexture1 = ObjInit::texture(testTexturePath);
		mTestTexture2 = ObjInit::texture(testTexture2Path);

		//for (int i = 0; i < 8; i++) {
		//	std::string path = testTexturesPath + "texture" + std::to_string(i) + ".png";
		//	std::shared_ptr<TextureVulkan> testTexture = ObjInit::texture(path);
		//	mTestTextures.push_back(testTexture);
		//}

		mSceneShaderProgram = ObjInit::shaderProgram(
			ObjInit::shader(
				EShaderType::VERTEX,
				texturedShaderVertPath
				//flatColourShaderVertPath
			),
			ObjInit::shader(
				EShaderType::FRAGMENT,
				texturedShaderFragPath
				//flatColourShaderFragPath
			)
		);

		ShaderUniformLayout& layout = mSceneShaderProgram->getUniformLayout();
		layout.setValue(0, sizeof(UniformBufferObject));
		layout.setTexture(1, { mTestTexture1->getImageView(), mTestTexture1->getSampler() });
		

		mSceneVertexArray = ObjInit::vertexArray();
		std::shared_ptr<VertexBufferVulkan> sceneVb = ObjInit::stagedVertexBuffer(
			{
				{EDataType::FLOAT3},
				{EDataType::FLOAT4},
				{EDataType::FLOAT2},
				{EDataType::FLOAT}
			},
			mSceneVertices.data(),
			sizeof(Vertex3dTextured) * mSceneVertices.size(),
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		);
		mSceneVertexArray->addVertexBuffer(sceneVb);
		//std::shared_ptr<IndexBufferVulkan> sceneIb = ObjInit::stagedIndexBuffer(
		//	indices.data(),
		//	sizeof(uint16_t) * indices.size(),
		//	static_cast<uint32_t>(indices.size())
		//);
		std::shared_ptr<IndexBufferVulkan> sceneIb = ObjInit::stagedIndexBuffer(
			indices.data(),
			sizeof(uint16_t) * indices.size()
		);
		mSceneVertexArray->setDrawCount(static_cast<uint32_t>(indices.size()));
		mSceneVertexArray->setIndexBuffer(sceneIb);

		GET_RENDERER.prepareScenePipeline(*mSceneShaderProgram);

		//populateTestGrid(mTestGridSize[0], mTestGridSize[1]);
	}

	void TG_AppLogic::populateTestGrid(int width, int height) {
		const std::vector<std::shared_ptr<TextureVulkan>>& testTextures = GET_RENDERER.getContext().getRenderer2d().getStorage().getTestTextures();
		
		mTexturedTestGrid.clear();
		mTexturedTestGrid.resize(testTextures.size());

		uint32_t index = 0;
		for (float y = 0.0f; y < height; y++) {
			for (float x = 0.0f; x < width; x++) {
				uint32_t textureSlot = static_cast<uint32_t>(x + y) % 8;
				mTexturedTestGrid[textureSlot].push_back({
					{x * mTestGridQuadGapSize[0], y * mTestGridQuadGapSize[1], 0.5f},
					{mTestGridQuadSize[0], mTestGridQuadSize[1]},
					{0.0f, 1.0f, 1.0f, 1.0f},
					0.0f,
					testTextures[static_cast<uint32_t>(x + y) % 8]
				});
				index++;
			}
		}
	}

	void TG_AppLogic::initUi() {
		mUiShaderProgram = ObjInit::shaderProgram(
			ObjInit::shader(EShaderType::VERTEX, mUiShaderVertPath),
			ObjInit::shader(EShaderType::FRAGMENT, mUiShaderFragPath)
		);
		mUiShaderProgram->getUniformLayout().setValue(0, sizeof(UniformBufferObject));

		mUiVao = ObjInit::vertexArray();
		std::shared_ptr<VertexBufferVulkan> appUiVb = ObjInit::stagedVertexBuffer(
			{
				{EDataType::FLOAT2},
				{EDataType::FLOAT4}
			},
			mUiVertices.data(),
			sizeof(Vertex2d) * mUiVertices.size(),
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		);
		mUiVao->addVertexBuffer(appUiVb);
		//std::shared_ptr<IndexBufferVulkan> appUiIb = ObjInit::stagedIndexBuffer(
		//	mUiIndices.data(),
		//	sizeof(mUiIndices[0]) * mUiIndices.size(),
		//	static_cast<uint32_t>(mUiIndices.size())
		//);
		std::shared_ptr<IndexBufferVulkan> appUiIb = ObjInit::stagedIndexBuffer(
			mUiIndices.data(),
			sizeof(mUiIndices[0]) * mUiIndices.size()
		);
		mUiVao->setDrawCount(static_cast<uint32_t>(mUiIndices.size()));
		mUiVao->setIndexBuffer(appUiIb);

		GET_RENDERER.prepareUiPipeline(*mUiShaderProgram);
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
}
