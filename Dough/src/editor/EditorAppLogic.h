#pragma once

#include "dough/application/IApplicationLogic.h"
#include "dough/rendering/pipeline/shader/ShaderProgramVulkan.h"
#include "dough/rendering/buffer/VertexArrayVulkan.h"
#include "dough/rendering/textures/TextureVulkan.h"
#include "dough/rendering/Config.h"
#include "dough/scene/geometry/Quad.h"
#include "dough/rendering/renderables/RenderableModel.h"
#include "dough/rendering/renderables/SimpleRenderable.h"
#include "dough/rendering/pipeline/GraphicsPipelineVulkan.h"
#include "dough/time/PausableTimer.h"

#include "editor/EditorOrthoCameraController.h"
#include "editor/EditorPerspectiveCameraController.h"

#define BOUNCING_QUAD_COUNT
#if defined (_DEBUG)
	//Debug config
	#undef BOUNCING_QUAD_COUNT
	#define BOUNCING_QUAD_COUNT 40000
#else
	//Release config
	#undef BOUNCING_QUAD_COUNT
	#define BOUNCING_QUAD_COUNT 100000
#endif

using namespace DOH;

namespace DOH::EDITOR {

	//TODO:: Allow for stopping which resets the scene (re-init everything app-specific)
	enum class EInnerAppState {
		NONE = 0,

		PAUSED,
		PLAYING,
		STOPPED
	};

	class EditorAppLogic : public IApplicationLogic {

	private:
		struct ObjModelsDemo {
			struct UniformBufferObject {
				glm::mat4x4 ProjView;
			};

			const std::array<const char*, 4> ObjModelFilePaths = {
				"res/models/testCube.obj",
				"res/models/spoon.obj",
				"res/models/teacup.obj",
				"res/models/teapot.obj"
			};
			const uint32_t DefaultObjFilePathIndex = 0;

			//TODO:: Loading shaders with const char* crashes, strings work, FIX
			const std::string FlatColourShaderVertPath = "res/shaders/spv/FlatColour.vert.spv";
			const std::string FlatColourShaderFragPath = "res/shaders/spv/FlatColour.frag.spv";
			const char* ScenePipelineName = "ObjScene";
			const char* SceneWireframePipelineName = "ObjWireframe";
			const EVertexType SceneVertexType = EVertexType::VERTEX_3D;
			std::shared_ptr<ShaderProgramVulkan> SceneShaderProgram;

			std::unique_ptr<GraphicsPipelineInstanceInfo> ScenePipelineInfo;
			std::unique_ptr<GraphicsPipelineInstanceInfo> SceneWireframePipelineInfo;

			std::vector<std::shared_ptr<ModelVulkan>> LoadedModels;
			std::vector<std::shared_ptr<RenderableModelVulkan>> RenderableObjects;

			PipelineRenderableConveyor ScenePipelineConveyor;
			PipelineRenderableConveyor WireframePipelineConveyor;

			int AddNewObjectsCount = 0;
			int PopObjectsCount = 0;

			bool Update = false;
			bool Render = false;
			bool RenderAllStandard = false; //Force all models to be rendered normally (ignores individual model Render bool)
			bool RenderAllWireframe = false; //Force all models to be rendered as wireframe (ignores individual model Wireframe bool)
		};

		struct CustomDemo {
			struct UniformBufferObject {
				glm::mat4 ProjView;
			};

			const std::string TexturedShaderVertPath = "res/shaders/spv/Textured.vert.spv";
			const std::string TexturedShaderFragPath = "res/shaders/spv/Textured.frag.spv";
			const char* TestTexturePath = "res/images/testTexture.jpg";
			const char* TestTexture2Path = "res/images/testTexture2.jpg";
			const std::vector<Vertex3dTextured> SceneVertices = {
				//	x		y		z		r		g		b		a		u		v		texIndex
				{{	-0.5f,	-0.5f,	0.0f},	{1.0f,	0.0f,	0.0f,	1.0f},	{0.0f,	1.0f},	{0.0f}},
				{{	 0.5f,	-0.5f,	0.0f},	{0.0f,	0.0f,	0.0f,	1.0f},	{1.0f,	1.0f},	{0.0f}},
				{{	 0.5f,	0.5f,	0.0f},	{0.0f,	0.0f,	1.0f,	1.0f},	{1.0f,	0.0f},	{0.0f}},
				{{	-0.5f,	0.5f,	0.0f},	{0.0f,	0.0f,	0.0f,	1.0f},	{0.0f,	0.0f},	{0.0f}},

				{{0.00f,	0.00f,	0.0f},	{0.0f,	0.60f,	0.0f,	1.0f},	{0.0f,	1.0f},	{1.0f}},
				{{1.00f,	0.00f,	0.0f},	{0.0f,	0.60f,	0.0f,	1.0f},	{1.0f,	1.0f},	{1.0f}},
				{{1.00f,	1.00f,	0.0f},	{0.0f,	0.60f,	0.0f,	1.0f},	{1.0f,	0.0f},	{1.0f}},
				{{0.00f,	1.00f,	0.0f},	{0.0f,	0.60f,	0.0f,	1.0f},	{0.0f,	0.0f},	{1.0f}}
			};
			const std::vector<uint32_t> Indices{
				0, 1, 2, 2, 3, 0,
				4, 5, 6, 6, 7, 4
			};

			const std::string UiShaderVertPath = "res/shaders/spv/SimpleUi.vert.spv";
			const std::string UiShaderFragPath = "res/shaders/spv/SimpleUi.frag.spv";
			const std::vector<Vertex2d> UiVertices = {
				//	x		y			r		g		b		a
				{{	-1.0f,	-0.90f},	{0.0f,	1.0f,	0.0f,	1.0f}}, //bot-left
				{{	-0.75f,	-0.90f},	{0.0f,	0.5f,	0.5f,	1.0f}}, //bot-right
				{{	-0.75f,	-0.65f},	{0.0f,	0.0f,	1.0f,	1.0f}}, //top-right
				{{	-1.0f,	-0.65f},	{0.0f,	0.5f,	0.5f,	1.0f}}  //top-left
			};
			const std::vector<uint32_t> UiIndices{
				0, 1, 2, 2, 3, 0
			};

			const EVertexType SceneVertexType = EVertexType::VERTEX_3D_TEXTURED;
			const EVertexType UiVertexType = EVertexType::VERTEX_2D;

			glm::mat4x4 UiProjMat = glm::mat4x4(1.0f);
			std::unique_ptr<GraphicsPipelineInstanceInfo> ScenePipelineInfo;
			std::shared_ptr<ShaderProgramVulkan> SceneShaderProgram;
			std::unique_ptr<GraphicsPipelineInstanceInfo> UiPipelineInfo;
			std::shared_ptr<ShaderProgramVulkan> UiShaderProgram;

			std::shared_ptr<TextureVulkan> TestTexture1;
			std::shared_ptr<TextureVulkan> TestTexture2;

			//Renderables are for an easier API overall, they do not fully "own" an object,
			// therefore, they are not responsible for clearing up resources
			std::shared_ptr<SimpleRenderable> SceneRenderable;
			std::shared_ptr<VertexArrayVulkan> SceneVao;
			std::shared_ptr<SimpleRenderable> UiRenderable;
			std::shared_ptr<VertexArrayVulkan> UiVao;

			PipelineRenderableConveyor CustomSceneConveyor;
			PipelineRenderableConveyor CustomUiConveyor;

			const char* ScenePipelineName = "Custom";
			const char* UiPipelineName = "CustomUi";

			bool Update = false;
			bool RenderScene = false;
			bool RenderUi = false;
		};

		struct GridDemo {
			std::vector<std::vector<Quad>> TexturedTestGrid;
			glm::vec4 QuadColour = { 1.0f, 1.0f, 1.0f, 1.0f };
			bool QuadDrawColour = false;
			uint32_t TestTexturesRowOffset = 0;
			uint32_t TestTexturesColumnOffset = 0;
			int TestGridMaxQuadCount = 0;
			int TestGridSize[2] = { 10, 10 };
			float TestGridQuadSize[2] = { 0.1f, 0.1f };
			float TestGridQuadGapSize[2] = { TestGridQuadSize[0] * 1.5f, TestGridQuadSize[1] * 1.5f };

			bool Update = false;
			bool Render = false;

			bool IsUpToDate = false;
		};

		struct BouncingQuadDemo {
			std::vector<Quad> BouncingQuads;
			std::vector<glm::vec2> BouncingQuadVelocities;
			size_t MaxBouncingQuadCount = BOUNCING_QUAD_COUNT;
			bool QuadDrawColour = false;

			int AddNewQuadCount = 0;
			int PopQuadCount = 0;

			bool Update = false;
			bool Render = false;
		};

		struct TextDemo {
			static const size_t StringLengthLimit = 1024;

			std::vector<Quad> TextQuads;
			char String[StringLengthLimit]; //Arbitrary limit
			float Colour[4] = {1.0f, 1.0f, 1.0f, 1.0f};

			bool Update = false;
			bool Render = false;
		};

		struct ImGuiTextureViewerWindow {
			const TextureVulkan& Texture;
			bool Display;
			bool MatchWindowSize;
			float Scale;

			ImGuiTextureViewerWindow(
				TextureVulkan& texture,
				const bool display,
				const bool matchWindowSize,
				const float scale
			) : Texture(texture),
				Display(display),
				MatchWindowSize(matchWindowSize),
				Scale(scale)
			{}
		};

		struct EditorSettings {
			//Map for texture windows, using the texture's ID as the key
			std::unordered_map<uint32_t, ImGuiTextureViewerWindow> TextureViewerWindows;

			//ImGui window rendering controls
			bool RenderDebugWindow = true;

			//ImGui menu
			bool EditorCollapseMenuOpen = true;
			bool InnerAppCollapseMenu = true;
			bool RenderingCollapseMenuOpen = true;
			bool CameraCollapseMenuOpen = true;
			bool CurrentDemoCollapseMenuOpen = true;

			//Camera (switch between orthographic and perspective camera)
			bool UseOrthographicCamera = true;

			bool RenderObjModelsList = false;

			//Close App
			const float QuitHoldTimeRequired = 1.5f;
			float QuitButtonHoldTime = 0.0f;
		};

		//Cameras
		std::shared_ptr<EditorOrthoCameraController> mOrthoCameraController;
		std::shared_ptr<EditorPerspectiveCameraController> mPerspectiveCameraController;

		//Demos
		std::unique_ptr<ObjModelsDemo> mObjModelsDemo;
		std::unique_ptr<CustomDemo> mCustomDemo;
		std::unique_ptr<GridDemo> mGridDemo;
		std::unique_ptr<BouncingQuadDemo> mBouncingQuadDemo;
		std::unique_ptr<TextDemo> mTextDemo;

		//Editor settings
		std::unique_ptr<EditorSettings> mEditorSettings;
		std::unique_ptr<PausableTimer> mInnerAppTimer;
		EInnerAppState mInnerAppState;

		//const char* testTexturesPath = "res/images/test textures/";
		//std::vector<std::shared_ptr<TextureVulkan>> mTestTextures;

	public:
		EditorAppLogic();
		EditorAppLogic(const EditorAppLogic& copy) = delete;
		EditorAppLogic operator=(const EditorAppLogic& assignment) = delete;

		virtual void init(float aspectRatio) override;
		virtual void update(float delta) override;
		virtual void render() override;
		virtual void imGuiRender(float delta) override;
		virtual void close() override;

		virtual void onResize(float aspectRatio) override;

	private:
		inline void setUiProjection(float aspectRatio) {
			mCustomDemo->UiProjMat = glm::ortho(-aspectRatio, aspectRatio, -1.0f, 1.0f, -1.0f, 1.0f);
			mCustomDemo->UiProjMat[1][1] *= -1;
		}

		void populateTestGrid(uint32_t width, uint32_t height);

		void initDemos();
		//void initGridDemo();
		void initBouncingQuadsDemo();
		void initCustomDemo();
		void initObjModelsDemo();
		void initTextDemo();
		void bouncingQuadsDemoAddRandomQuads(size_t count);
		void bouncingQaudsDemoPopQuads(size_t count);
		void objModelsDemoAddObject(
			const uint32_t modelIndex = 0,
			const float x = 0.0f,
			const float y = 0.0f,
			const float z = 0.0f,
			const float posPadding = 0.5f,
			const float yaw = 0.0f,
			const float pitch = 0.0f,
			const float roll = 0.0f,
			const float scale = 1.0f
		);
		inline void objModelsDemoAddRandomisedObject() {
			objModelsDemoAddObject(
				rand() % mObjModelsDemo->LoadedModels.size(),
				static_cast<float>(rand() % 25),
				static_cast<float>(rand() % 25),
				static_cast<float>(rand() % 15) * (rand() % 2 > 0 ? 1.0f : -1.0f),
				0.5f,
				static_cast<float>(rand() % 360),
				static_cast<float>(rand() % 360),
				static_cast<float>(rand() % 360),
				1.0f
			);
		}
		void imGuiDrawObjDemoItem(DOH::RenderableModelVulkan& model, const std::string& uniqueImGuiId);

		//Draw a window displaying a texture
		void imGuiDrawTextureViewerWindow(ImGuiTextureViewerWindow& textureWindow);
		//TODO:: more editor features
		//void imGuiDrawTextureAtlasViewerWindow(ImGuiTextureAtlasViewerWindow& textureAtlasWindow);
		void imGuiRemoveHiddenTextureViewerWindows();

		//ImGui convenience and separated functions, primarly used for debugging and easier ImGui functionality
		void imGuiDisplayHelpTooltip(const char* message);
		void imGuiBulletTextWrapped(const char* message);
		void imGuiRenderDebugWindow(float delta); //This renders the Editor windows
		void imGuiPrintDrawCallTableColumn(const char* pipelineName, uint32_t drawCount);
		void imGuiPrintMat4x4(const glm::mat4x4& mat, const char* name);
		inline void imGuiPrintMat4x4(const glm::mat4x4& mat, const std::string& name) { imGuiPrintMat4x4(mat, name.c_str()); }
	};
}
