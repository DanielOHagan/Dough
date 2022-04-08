#include "testGame/TG_AppLogic.h"

#include "dough/Utils.h"
#include "dough/rendering/ObjInit.h"
#include "dough/application/Application.h"
#include "dough/input/InputCodes.h"
#include "dough/Logging.h"

#include <imgui/imgui.h>

#define GET_RENDERER Application::get().getRenderer()

namespace TG {

	TG_AppLogic::TG_AppLogic()
	:	IApplicationLogic(),
		mUiProjMat(1.0f)
	{}

	void TG_AppLogic::init(float aspectRatio) {
		setUiProjection(aspectRatio);
		initScene(aspectRatio);
		initUi();
	}

	void TG_AppLogic::update(float delta) {
		RendererVulkan& renderer = GET_RENDERER;

		TG_mOrthoCameraController->onUpdate(delta);

		renderer.beginScene(TG_mOrthoCameraController->getCamera());
		if (mImGuiSettings.mRenderScene) {
			renderer.getContext().addVaoToSceneDrawList(*mSceneVertexArray);
		}

		if (mImGuiSettings.mRenderBatchQuadScene) {
			renderer.getContext().getRenderer2d().drawQuadArrayScene(mTestGrid);
		}
		renderer.endScene();

		renderer.beginUi(mUiProjMat);
		if (mImGuiSettings.mRenderUi) {
			renderer.getContext().addVaoToUiDrawList(*mUiVao);
		}
		renderer.endUi();
	}

	void TG_AppLogic::imGuiRender() {
		//TODO:: Clean up basic layout and styling, maybe add a few imgui helper functions

		RendererVulkan& renderer = GET_RENDERER;

		ImGui::Begin("Debug Window");


		ImGui::SetNextItemOpen(mImGuiSettings.mApplicationCollapseMenuOpen);
		if (ImGui::CollapsingHeader("Application")) {
			ApplicationLoop& loop = Application::get().getLoop();
			Window& window = Application::get().getWindow();
			bool focused = Application::get().isFocused();

			ImGui::Text("Runtime: %fs", Time::convertMillisToSeconds(Application::get().getAppInfoTimer().getCurrentTickingTimeMillis()));
			ImGui::Text("Window Size: X: %i \tY: %i", window.getWidth(), window.getHeight());
			ImGui::Text("Is Focused: ");
			ImGui::SameLine();
			ImGui::TextColored(focused ? ImVec4(0, 1, 0, 1) : ImVec4(1, 0, 0, 1), focused ? "FOCUSED" : "NOT FOCUSED");
			ImGui::Text("Current and Target FPS/UPS");
			ImGui::Text("FPS: %i \t(Fore: %i, Back: %i)", (int)loop.getFps(), (int)loop.getTargetFps(), (int)loop.getTargetBackgroundFps());
			imguiDisplayHelpTooltip("FPS displayed is the count of frames in the last full second interval");
			ImGui::Text("UPS: %i \t(Fore: %i, Back: %i)", (int)loop.getUps(), (int)loop.getTargetUps(), (int)loop.getTargetBackgroundUps());
			imguiDisplayHelpTooltip("UPS displayed is the count of frames in the last full second interval");

			bool runInBackground = loop.isRunningInBackground();
			if (ImGui::Checkbox("Run In Background", &runInBackground)) {
				loop.setRunInBackground(runInBackground);
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

			mImGuiSettings.mRenderingCollapseMenuOpen = true;
		} else {
			mImGuiSettings.mRenderingCollapseMenuOpen = false;
		}

		ImGui::SetNextItemOpen(mImGuiSettings.mSceneDataCollapseMenuOpen);
		if (ImGui::CollapsingHeader("Quad Test Grid")) {
			ImGui::Text("Test Grid Quad Count: %i", mTestGridSize[0] * mTestGridSize[1]);
			int tempTestGridSize[2] = { mTestGridSize[0], mTestGridSize[1] };
			ImGui::InputInt2("Test Grid Size", tempTestGridSize);
			if (tempTestGridSize[0] > 0 && tempTestGridSize[1] > 0) {
				int tempGridQuadCount = tempTestGridSize[0] * tempTestGridSize[1];
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

			ImGui::DragFloat2("Test Grid Quad Size", mTestGridQuadSize, 0.001f, 0.01f, 0.5f);
			ImGui::DragFloat2("Test Grid Quad Gap Size", mTestGridQuadGapSize, 0.001f, 0.01f, 0.5f);
			//ImGui::DragFloat2("Test Grid Origin Pos", );
			//ImGui::Text("UI Quad Count: %i", renderer.getContext().getRenderer2d().getStorage().getUiQuadCount());

			//TOOD:: maybe have radio buttons for RenderStaticGrid or RenderDynamicGrid,
			//	static being the default values and dynamic being from the variables determined by ths menu
			// Maybe have the dynamic settings hidden unless dynamic is selected

			if (ImGui::Button("Reset Grid")) {
				//TODO:: Set values to default
				LOG_WARN("'Reset Grid' button not implemented yet");
			}

			mImGuiSettings.mSceneDataCollapseMenuOpen = true;
		} else {
			mImGuiSettings.mSceneDataCollapseMenuOpen = false;
		}

		//TODO:: Use different formats, currently looks bad
		ImGui::SetNextItemOpen(mImGuiSettings.mCameraCollapseMenuOpen);
		if (ImGui::CollapsingHeader("Scene Camera")) {
			ImGui::BeginListBox("Camera Controls");
			ImGui::BulletText("W, A, S, D: Move Camera");
			ImGui::BulletText("Z, X: Zoom Camera In & Out");
			ImGui::EndListBox();
			ImGui::BeginListBox("Camera Position");
			ImGui::Text("X: %f", TG_mOrthoCameraController->getPosition().x);
			ImGui::Text("Y: %f", TG_mOrthoCameraController->getPosition().y);
			ImGui::Text("Zoom: %f", TG_mOrthoCameraController->getZoomLevel());
			imguiDisplayHelpTooltip("Higher is more \"zoomed in\" and lower is more \"zoomed out\"");
			ImGui::EndListBox();
			if (ImGui::Button("Reset Camera")) {
				TG_mOrthoCameraController->setPosition({0.0f, 0.0f, 0.0f});
				TG_mOrthoCameraController->setZoomLevel(1.0f);
			}

			mImGuiSettings.mCameraCollapseMenuOpen = true;
		} else {
			mImGuiSettings.mCameraCollapseMenuOpen = false;
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
	}

	void TG_AppLogic::onResize(float aspectRatio) {
		TG_mOrthoCameraController->onViewportResize(aspectRatio);
		setUiProjection(aspectRatio);
	}

	void TG_AppLogic::initScene(float aspectRatio) {
		mTestTexture1 = ObjInit::texture(testTexturePath);
		mTestTexture2 = ObjInit::texture(testTexture2Path);

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
		std::shared_ptr<IndexBufferVulkan> sceneIb = ObjInit::stagedIndexBuffer(
			indices.data(),
			sizeof(indices[0]) * indices.size(),
			static_cast<uint32_t>(indices.size())
		);
		mSceneVertexArray->setIndexBuffer(sceneIb);

		GET_RENDERER.prepareScenePipeline(*mSceneShaderProgram);

		TG_mOrthoCameraController = std::make_shared<TG::TG_OrthoCameraController>(aspectRatio);

		populateTestGrid(mTestGridSize[0], mTestGridSize[0]);
	}

	void TG_AppLogic::populateTestGrid(int width, int height) {
		for (float x = 0.0f; x < width; x++) {
			for (float y = 0.0f; y < height; y++) {
				Quad quad = {
					{x * mTestGridQuadGapSize[0], y * mTestGridQuadGapSize[1], 1.0f},
					{mTestGridQuadSize[0], mTestGridQuadSize[1]},
					{0.0f, 1.0f, 1.0f, 1.0f}
				};

				//TOOD:: texture slot to be determined by batch by texture that is attatched to quad
				//uint32_t textureSlot = static_cast<uint32_t>(x) % 8;

				mTestGrid.push_back({
					{x * mTestGridQuadGapSize[0], y * mTestGridQuadGapSize[1], 1.0f},
					{mTestGridQuadSize[0], mTestGridQuadSize[1]},
					{0.0f, 1.0f, 1.0f, 1.0f}
				});
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
		std::shared_ptr<IndexBufferVulkan> appUiIb = ObjInit::stagedIndexBuffer(
			mUiIndices.data(),
			sizeof(mUiIndices[0]) * mUiIndices.size(),
			static_cast<uint32_t>(mUiIndices.size())
		);
		mUiVao->setIndexBuffer(appUiIb);

		GET_RENDERER.prepareUiPipeline(*mUiShaderProgram);
	}

	void TG_AppLogic::imguiDisplayHelpTooltip(const char* message) {
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
