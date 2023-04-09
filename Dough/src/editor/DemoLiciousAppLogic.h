#pragma once

#include "dough/application/IApplicationLogic.h"
#include "dough/Core.h"
#include "dough/Maths.h"
#include "dough/scene/geometry/collections/TextString.h"
#include "dough/rendering/RendererVulkan.h"
#include "dough/rendering/renderables/RenderableModel.h"
#include "dough/input/Input.h"
#include "dough/input/DefaultInputLayer.h"

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

	class DemoLiciousAppLogic : public IApplicationLogic {

	private:

		struct ObjModelsDemo {
			struct UniformBufferObject {
				glm::mat4x4 ProjView;
			};

			//Models using EVertexType::VERTEX_3D
			const std::array<const char*, 4> ObjModelFilePaths = {
				"Dough/res/models/testCube.obj",
				"Dough/res/models/spoon.obj",
				"Dough/res/models/teacup.obj",
				"Dough/res/models/teapot.obj"
			};
			const uint32_t DefaultObjFilePathIndex = 0;

			//TODO:: Loading shaders with const char* crashes, strings work, FIX
			const std::string FlatColourShaderVertPath = "Dough/res/shaders/spv/FlatColour.vert.spv";
			const std::string FlatColourShaderFragPath = "Dough/res/shaders/spv/FlatColour.frag.spv";
			const char* ScenePipelineName = "ObjScene";
			const char* SceneWireframePipelineName = "ObjWireframe";
			std::shared_ptr<StaticVertexInputLayout> ColouredVertexInputLayout;
			std::shared_ptr<StaticVertexInputLayout> TexturedVertexInputLayout;
			std::shared_ptr<ShaderProgramVulkan> SceneShaderProgram;

			std::unique_ptr<GraphicsPipelineInstanceInfo> ScenePipelineInfo;
			std::unique_ptr<GraphicsPipelineInstanceInfo> SceneWireframePipelineInfo;

			std::vector<std::shared_ptr<ModelVulkan>> LoadedModels;
			std::vector<std::shared_ptr<RenderableModelVulkan>> RenderableObjects;

			std::shared_ptr<ModelVulkan> TexturedModel;
			std::shared_ptr<RenderableModelVulkan> RenderableTexturedModel;

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

			const std::vector<Vertex3dTextured> SceneVertices = {
				//	x		y		z		r		g		b		a		u		v		
				{{	-0.5f,	-0.5f,	0.0f},	{1.0f,	0.0f,	0.0f,	1.0f},	{0.0f,	1.0f} },
				{{	 0.5f,	-0.5f,	0.0f},	{0.0f,	0.0f,	0.0f,	1.0f},	{1.0f,	1.0f} },
				{{	 0.5f,	0.5f,	0.0f},	{0.0f,	0.0f,	1.0f,	1.0f},	{1.0f,	0.0f} },
				{{	-0.5f,	0.5f,	0.0f},	{0.0f,	0.0f,	0.0f,	1.0f},	{0.0f,	0.0f} },

				{{	0.00f,	0.00f,	0.0f},	{0.0f,	0.60f,	0.0f,	1.0f},	{0.0f,	1.0f} },
				{{	1.00f,	0.00f,	0.0f},	{0.0f,	0.60f,	0.0f,	1.0f},	{1.0f,	1.0f} },
				{{	1.00f,	1.00f,	0.0f},	{0.0f,	0.60f,	0.0f,	1.0f},	{1.0f,	0.0f} },
				{{	0.00f,	1.00f,	0.0f},	{0.0f,	0.60f,	0.0f,	1.0f},	{0.0f,	0.0f} }
			};
			const std::vector<uint32_t> Indices{
				0, 1, 2, 2, 3, 0,
				4, 5, 6, 6, 7, 4
			};

			const std::string UiShaderVertPath = "Dough/res/shaders/spv/SimpleUi.vert.spv";
			const std::string UiShaderFragPath = "Dough/res/shaders/spv/SimpleUi.frag.spv";
			const std::vector<Vertex2d> UiVertices = {
				//	x		y			r		g		b		a
				{{	-1.0f,	-0.90f},	{0.0f,	1.0f,	0.0f,	1.0f}},	//bot-left
				{{	-0.75f,	-0.90f},	{0.0f,	0.5f,	0.5f,	1.0f}},	//bot-right
				{{	-0.75f,	-0.65f},	{0.0f,	0.0f,	1.0f,	1.0f}},	//top-right
				{{	-1.0f,	-0.65f},	{0.0f,	0.5f,	0.5f,	1.0f}}	//top-left
			};
			const std::vector<uint32_t> UiIndices{
				0, 1, 2, 2, 3, 0
			};

			glm::mat4x4 UiProjMat = glm::mat4x4(1.0f);

			std::unique_ptr<GraphicsPipelineInstanceInfo> UiPipelineInfo;
			std::shared_ptr<ShaderProgramVulkan> UiShaderProgram;
			PipelineRenderableConveyor CustomUiConveyor;
			const char* UiPipelineName = "CustomUi";

			//Renderables are for an easier API overall, they do not fully "own" an object,
			// therefore, they are not responsible for clearing up resources
			std::shared_ptr<SimpleRenderable> SceneRenderable;
			std::shared_ptr<VertexArrayVulkan> SceneVao;
			std::shared_ptr<StaticVertexInputLayout> SceneVertexInputLayout;
			std::shared_ptr<SimpleRenderable> UiRenderable;
			std::shared_ptr<VertexArrayVulkan> UiVao;
			std::shared_ptr<StaticVertexInputLayout> UiVertexInputLayout;

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
			static constexpr size_t StringLengthLimit = 1000; //Arbitrary limit
		
			std::unique_ptr<TextString> TextString;
			char StringBuffer[StringLengthLimit] = "This is the default text string. Use the Text Field to change me!";
			glm::vec4 Colour = { 1.0f, 1.0f, 1.0f, 1.0f };
		
			bool Update = false;
			bool Render = false;
		};

		//Resources used by more than one demo
		struct SharedDemoResources {
			static const EVertexType TexturedVertexType = EVertexType::VERTEX_3D_TEXTURED;

			static const std::string TexturedShaderVertPath;
			static const std::string TexturedShaderFragPath;

			const char* TexturedPipelineName = "Textured";
			std::unique_ptr<GraphicsPipelineInstanceInfo> TexturedPipelineInfo;
			std::shared_ptr<ShaderProgramVulkan> TexturedShaderProgram;

			PipelineRenderableConveyor TexturedConveyor;

			const char* TestTexturePath = "Dough/res/images/testTexture.jpg";
			const char* TestTexture2Path = "Dough/res/images/testTexture2.jpg";
			std::shared_ptr<TextureVulkan> TestTexture1;
			std::shared_ptr<TextureVulkan> TestTexture2;
		};

		struct ImGuiSettings {
			bool CurrentDemoCollapseMenuOpen = true;
			bool RenderObjModelsList = false;
		};

		//Demos
		std::unique_ptr<SharedDemoResources> mSharedDemoResources;
		std::unique_ptr<ObjModelsDemo> mObjModelsDemo;
		std::unique_ptr<CustomDemo> mCustomDemo;
		std::unique_ptr<GridDemo> mGridDemo;
		std::unique_ptr<BouncingQuadDemo> mBouncingQuadDemo;
		std::unique_ptr<TextDemo> mTextDemo;

		std::unique_ptr<ImGuiSettings> mImGuiSettings;
		std::shared_ptr<DefaultInputLayer> mInputLayer;

		//TODO:: allow for apps to pass in textures to batch renderer so their lifetime is controlled by the app and not the engine
		//const char* testTexturesPath = "Dough/res/images/test textures/";
		//std::shared_ptr<MonoSpaceTextureAtlas> mTestTexturesAtlas;

	public:
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
		void initSharedResources();
		//void initGridDemo(); //Doesn't currently need any form of initialisation, commented-out declaration just here in case it ever needs it
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
				static_cast<float>(rand() % 5) * (rand() % 2 > 0 ? 1.0f : -1.0f),
				static_cast<float>(rand() % 5) * (rand() % 2 > 0 ? 1.0f : -1.0f),
				static_cast<float>(rand() % 5) * (rand() % 2 > 0 ? 1.0f : -1.0f),
				0.5f,
				static_cast<float>(rand() % 360),
				static_cast<float>(rand() % 360),
				static_cast<float>(rand() % 360),
				1.0f
			);
		}

		void imGuiDrawObjDemoItem(DOH::RenderableModelVulkan& model, const std::string& uniqueImGuiId);
	};
}
