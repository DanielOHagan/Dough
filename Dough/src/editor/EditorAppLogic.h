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

	class EditorAppLogic : public IApplicationLogic {

	private:
		//Cameras
		std::shared_ptr<EditorOrthoCameraController> mOrthoCameraController;
		std::shared_ptr<EditorPerspectiveCameraController> mPerspectiveCameraController;

		struct ObjModelsDemo {
			struct UniformBufferObject {
				glm::mat4x4 projView;
			};

			const std::array<std::string, 4> mObjModelFilePaths = {
				"res/models/testCube.obj",
				"res/models/spoon.obj",
				"res/models/teacup.obj",
				"res/models/teapot.obj"
			};
			const uint32_t mDefaultObjFilePathIndex = 0;

			const std::string flatColourShaderVertPath = "res/shaders/spv/FlatColour.vert.spv";
			const std::string flatColourShaderFragPath = "res/shaders/spv/FlatColour.frag.spv";
			const std::string mScenePipelineName = "ObjScene";
			const std::string mSceneWireframePipelineName = "ObjWireframe";
			const EVertexType mSceneVertexType = EVertexType::VERTEX_3D;
			std::shared_ptr<ShaderProgramVulkan> mSceneShaderProgram;

			std::unique_ptr<GraphicsPipelineInstanceInfo> mScenePipelineInfo;
			std::unique_ptr<GraphicsPipelineInstanceInfo> mSceneWireframePipelineInfo;

			std::vector<std::shared_ptr<ModelVulkan>> mLoadedModels;
			std::vector<std::shared_ptr<RenderableModelVulkan>> mRenderableObjects;

			int AddNewObjectsCount = 0;
			int PopObjectsCount = 0;

			bool Update = false;
			bool Render = false;
			bool RenderAllStandard = false; //Force all models to be rendered normally (ignores individual model Render bool)
			bool RenderAllWireframe = false; //Force all models to be rendered as wireframe (ignores individual model Wireframe bool)
		} mObjModelsDemo;

		struct CustomDemo {

			struct UniformBufferObject {
				glm::mat4 projView;
			};

			const std::string texturedShaderVertPath = "res/shaders/spv/Textured.vert.spv";
			const std::string texturedShaderFragPath = "res/shaders/spv/Textured.frag.spv";
			const std::string testTexturePath = "res/images/testTexture.jpg";
			const std::string testTexture2Path = "res/images/testTexture2.jpg";
			const std::vector<Vertex3dTextured> mSceneVertices = {
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
			const std::vector<uint32_t> indices{
				0, 1, 2, 2, 3, 0,
				4, 5, 6, 6, 7, 4
			};

			const std::string mUiShaderVertPath = "res/shaders/spv/SimpleUi.vert.spv";
			const std::string mUiShaderFragPath = "res/shaders/spv/SimpleUi.frag.spv";
			const std::vector<Vertex2d> mUiVertices = {
				//	x		y			r		g		b		a
				{{	-1.0f,	-0.90f},	{0.0f,	1.0f,	0.0f,	1.0f}}, //bot-left
				{{	-0.75f,	-0.90f},	{0.0f,	0.5f,	0.5f,	1.0f}}, //bot-right
				{{	-0.75f,	-0.65f},	{0.0f,	0.0f,	1.0f,	1.0f}}, //top-right
				{{	-1.0f,	-0.65f},	{0.0f,	0.5f,	0.5f,	1.0f}}  //top-left
			};
			const std::vector<uint32_t> mUiIndices{
				0, 1, 2, 2, 3, 0
			};

			const EVertexType mSceneVertexType = EVertexType::VERTEX_3D_TEXTURED;
			const EVertexType mUiVertexType = EVertexType::VERTEX_2D;

			glm::mat4x4 mUiProjMat = glm::mat4x4(1.0f);
			std::unique_ptr<GraphicsPipelineInstanceInfo> mScenePipelineInfo;
			std::shared_ptr<ShaderProgramVulkan> mSceneShaderProgram;
			std::unique_ptr<GraphicsPipelineInstanceInfo> mUiPipelineInfo;
			std::shared_ptr<ShaderProgramVulkan> mUiShaderProgram;
			
			std::shared_ptr<TextureVulkan> mTestTexture1;
			std::shared_ptr<TextureVulkan> mTestTexture2;

			//Renderables are for an easier API overall, they do not fully "own" an object,
			// therefore, they are not responsible for clearing up resources
			std::shared_ptr<SimpleRenderable> mSceneRenderable;
			std::shared_ptr<VertexArrayVulkan> mSceneVao;
			std::shared_ptr<SimpleRenderable> mUiRenderable;
			std::shared_ptr<VertexArrayVulkan> mUiVao;

			const std::string mScenePipelineName = "Custom";
			const std::string mUiPipelineName = "CustomUi";

			bool Update = false;
			bool RenderScene = false;
			bool RenderUi = false;
		} mCustomDemo;
		//const std::string testTexturesPath = "res/images/test textures/";
		//std::vector<std::shared_ptr<TextureVulkan>> mTestTextures;

		struct GridDemo {
			std::vector<std::vector<Quad>> mTexturedTestGrid;
			uint32_t mTestTexturesRowOffset = 0;
			uint32_t mTestTexturesColumnOffset = 0;
			int mTestGridMaxQuadCount = 0;
			int mTestGridSize[2] = { 10, 10 };
			float mTestGridQuadSize[2] = { 0.1f, 0.1f };
			float mTestGridQuadGapSize[2] = { mTestGridQuadSize[0] * 1.5f, mTestGridQuadSize[1] * 1.5f };

			bool Update = false;
			bool Render = false;
		} mGridDemo;

		struct BouncingQuadDemo {
			std::vector<Quad> mBouncingQuads;
			std::vector<glm::vec2> mBouncingQuadVelocities;
			size_t MaxBouncingQuadCount = BOUNCING_QUAD_COUNT;

			int AddNewQuadCount = 0;
			int PopQuadCount = 0;
			
			bool Update = false;
			bool Render = false;
		} mBouncingQuadDemo;

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

		struct ImGuiSettings {
			std::unordered_map<uint32_t, ImGuiTextureViewerWindow> TextureViewerWindows;
			
			//ImGui window rendering controls
			bool RenderDebugWindow = true;

			//ImGui menu
			bool ApplicationCollapseMenuOpen = true;
			bool RenderingCollapseMenuOpen = true;
			bool CameraCollapseMenuOpen = true;
			bool CurrentDemoCollapseMenuOpen = true;

			//Camera (switch between orthographic and perspective camera)
			bool UseOrthographicCamera = true;

			bool RenderObjModelsList = false;
		} mImGuiSettings;
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
			mCustomDemo.mUiProjMat = glm::ortho(-aspectRatio, aspectRatio, -1.0f, 1.0f, -1.0f, 1.0f);
			mCustomDemo.mUiProjMat[1][1] *= -1;
		}

		void populateTestGrid(int width, int height);

		void initDemos();
		//void initGridDemo();
		void initBouncingQuadsDemo();
		void initCustomDemo();
		void initObjModelsDemo();
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
				rand() % mObjModelsDemo.mLoadedModels.size(),
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
		//void imGuiDrawTextureAtlasViewerWindow(ImGuiTextureAtlasViewerWindow& textureAtlasWindow);
		void imGuiRemoveHiddenTextureViewerWindows();

		//ImGui convenience and separated functions, primarly used for debugging and easier ImGui functionality
		void imGuiDisplayHelpTooltip(const char* message);
		void imGuiBulletTextWrapped(const char* message);
		void imGuiRenderDebugWindow(float delta);
		void imGuiPrintDrawCallTableColumn(const char* pipelineName, uint32_t drawCount);
		void imGuiPrintMat4x4(const glm::mat4x4& mat, const char* name);
		inline void imGuiPrintMat4x4(const glm::mat4x4& mat, const std::string& name) { imGuiPrintMat4x4(mat, name.c_str()); }
	};
}
