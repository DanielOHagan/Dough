#pragma once

#include "dough/application/IApplicationLogic.h"
#include "dough/Core.h"
#include "dough/Maths.h"
#include "dough/scene/geometry/collections/TextString.h"
#include "dough/rendering/RendererVulkan.h"
#include "dough/rendering/renderables/RenderableModel.h"
#include "dough/input/Input.h"
#include "dough/input/DefaultInputLayer.h"
#include "dough/physics/BoundingBox2d.h"
#include "dough/scene/geometry/collections/TileMap.h"

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

		class IDemo {
		public:
			virtual void init() = 0;
			virtual void close() = 0;
			virtual void renderImGuiMainTab() = 0;
			virtual void renderImGuiExtras() = 0;
		};

		class ObjModelsDemo : public IDemo {
		public:
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
			const StaticVertexInputLayout& ColouredVertexInputLayout = StaticVertexInputLayout::get(EVertexType::VERTEX_3D);
			const StaticVertexInputLayout& TexturedVertexInputLayout = StaticVertexInputLayout::get(EVertexType::VERTEX_3D_TEXTURED);
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
			bool RenderObjModelsList = false;
			bool RenderAllStandard = false; //Force all models to be rendered normally (ignores individual model Render bool)
			bool RenderAllWireframe = false; //Force all models to be rendered as wireframe (ignores individual model Wireframe bool)

			bool GpuResourcesLoaded = false;

			virtual void init() override;
			virtual void close() override;
			virtual void renderImGuiMainTab() override;
			virtual void renderImGuiExtras() override;

			void addObject(
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
			inline void addRandomisedObject() {
				addObject(
					rand() % LoadedModels.size(),
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

		class CustomDemo : public IDemo {
		public:
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
			const std::vector<uint32_t> Indices = {
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
			const std::vector<uint32_t> UiIndices = {
				0, 1, 2, 2, 3, 0
			};

			glm::mat4x4 UiProjMat = glm::mat4x4(1.0f);

			std::unique_ptr<GraphicsPipelineInstanceInfo> UiPipelineInfo;
			std::shared_ptr<ShaderProgramVulkan> UiShaderProgram;
			PipelineRenderableConveyor CustomUiConveyor;
			constexpr static const char* UiPipelineName = "CustomUi";

			//Renderables are for an easier API overall, they do not fully "own" an object,
			// therefore, they are not responsible for clearing up resources
			std::shared_ptr<SimpleRenderable> SceneRenderable;
			std::shared_ptr<VertexArrayVulkan> SceneVao;
			const StaticVertexInputLayout& SceneVertexInputLayout = StaticVertexInputLayout::get(SharedDemoResources::TexturedVertexType);
			std::shared_ptr<SimpleRenderable> UiRenderable;
			std::shared_ptr<VertexArrayVulkan> UiVao;
			const StaticVertexInputLayout& UiVertexInputLayout = StaticVertexInputLayout::get(EVertexType::VERTEX_2D);

			bool Update = false;
			bool RenderScene = false;
			bool RenderUi = false;
			bool GpuResourcesLoaded = false;

			virtual void init() override;
			virtual void close() override;
			virtual void renderImGuiMainTab() override;
			virtual void renderImGuiExtras() override;
		};

		class GridDemo : public IDemo {
		public:
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

			virtual void init() override;
			virtual void close() override;
			virtual void renderImGuiMainTab() override;
			virtual void renderImGuiExtras() override;
		};

		class BouncingQuadDemo : public IDemo {
		public:
			std::vector<Quad> BouncingQuads;
			std::vector<glm::vec2> BouncingQuadVelocities;
			glm::vec2 QuadSize = { 0.1f, 0.1f };
			size_t MaxBouncingQuadCount = BOUNCING_QUAD_COUNT;
			bool QuadDrawColour = false;
		
			int AddNewQuadCount = 0;
			int PopQuadCount = 0;
		
			bool Update = false;
			bool Render = false;
		
			virtual void init() override;
			virtual void close() override;
			virtual void renderImGuiMainTab() override;
			virtual void renderImGuiExtras() override;
		
			void addRandomQuads(size_t count);
			void popQuads(size_t count);
		};

		class TextDemo : public IDemo {
		public:
			static constexpr size_t StringLengthLimit = 1000; //Arbitrary limit
		
			std::unique_ptr<TextString> Text;
			std::unique_ptr<TextString> MsdfText;
			char StringBuffer[StringLengthLimit] = "This is the default text string. Use the Text Field to change me!";
			char MsdfStringBuffer[StringLengthLimit] = "Default MSDF text string, use the text field to change me!";
			glm::vec4 Colour = { 1.0f, 1.0f, 1.0f, 1.0f };
		
			bool Update = false;
			bool Render = false;

			virtual void init() override;
			virtual void close() override;
			virtual void renderImGuiMainTab() override;
			virtual void renderImGuiExtras() override;
		};

		class LineDemo : public IDemo {
		public:
			static constexpr uint32_t LINE_2D_INPUT_COMPONENT_COUNT = 8;
			static constexpr uint32_t LINE_3D_INPUT_COMPONENT_COUNT = 10; //3 for start, 3 for end, 4 for colour
			std::vector<float> LineData2d;
			std::vector<float> LineData3d;
			Quad UiQuadTest = { {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f}, { 0.0f, 1.0f, 0.0f, 1.0f } };
			float LineDataInput[LINE_3D_INPUT_COMPONENT_COUNT] = {};
			uint32_t LineCount2d = 0;
			uint32_t LineCount3d = 0;
			uint32_t LineDataIndex2d = 0;
			uint32_t LineDataIndex3d = 0;
			int LinePopCount = 1;

			bool Update = false;
			bool Render = false;

			bool RenderTextDemoOutlines = false;
			bool RenderUiQuad = false;

			virtual void init() override;
			virtual void close() override;
			virtual void renderImGuiMainTab() override;
			virtual void renderImGuiExtras() override;

			void addLine2d(glm::vec2 start, glm::vec2 end, glm::vec4 colour);
			void addLine3d(glm::vec3 start, glm::vec3 end, glm::vec4 colour);
			inline void popLine2d() {
				if (LineCount2d > 0) {
					LineData2d.resize(LineData2d.size() - static_cast<size_t>(LINE_2D_INPUT_COMPONENT_COUNT));
					LineCount2d--;
					LineDataIndex2d -= LINE_2D_INPUT_COMPONENT_COUNT;
				}
			};
			inline void popLine3d() {
				if (LineCount3d > 0) {
					LineData3d.resize(LineData3d.size() - static_cast<size_t>(LINE_3D_INPUT_COMPONENT_COUNT));
					LineCount3d--;
					LineDataIndex3d -= LINE_3D_INPUT_COMPONENT_COUNT;
				}
			};
			inline void popLines2d(const uint32_t count) {
				if (LineCount2d > 0 && count > 0) {
					if (count >= LineCount2d) {
						LineData2d.clear();
						LineCount2d = 0;
						LineDataIndex2d = 0;
					} else {
						LineData2d.resize(LineData2d.size() - static_cast<size_t>(count * LINE_2D_INPUT_COMPONENT_COUNT));
						LineCount2d -= count;
						LineDataIndex2d -= count * LINE_2D_INPUT_COMPONENT_COUNT;
					}
				}
			}
			inline void popLines3d(const uint32_t count) {
				if (LineCount3d > 0 && count > 0) {
					if (count >= LineCount3d) {
						LineData3d.clear();
						LineCount3d = 0;
						LineDataIndex3d = 0;
					} else {
						LineData3d.resize(LineData3d.size() - static_cast<size_t>(count * LINE_3D_INPUT_COMPONENT_COUNT));
						LineCount3d -= count;
						LineDataIndex3d -= count * LINE_3D_INPUT_COMPONENT_COUNT;
					}
				}
			}
		};

		class BoundingBoxDemo : public IDemo {
		public:
			std::vector<Quad> Quads;
			std::unique_ptr<BoundingBox2d> BoundingBox;

			Quad TempQuadToAdd = {};

			bool Update = false;
			bool Render = false;

			virtual void init() override;
			virtual void close() override;
			virtual void renderImGuiMainTab() override;
			virtual void renderImGuiExtras() override;
		};

		class TileMapDemo : public IDemo {
		public:
			std::unique_ptr<TileMap> SceneTileMap;
			std::unique_ptr<TextureAtlasAnimationController> PreviewAnimationController;
			Quad PreviewQuad;
			Quad AnimatedQuad;
			const char* PreviewedInnerTexture = "NONE";

			bool Update = false;
			bool Render = false;
			bool RenderPreviewQuad = false;

			virtual void init() override;
			virtual void close() override;
			virtual void renderImGuiMainTab() override;
			virtual void renderImGuiExtras() override;
		};

		//Resources used by more than one demo
		struct SharedDemoResources {
			static const EVertexType TexturedVertexType = EVertexType::VERTEX_3D_TEXTURED;

			static const std::string TexturedShaderVertPath;
			static const std::string TexturedShaderFragPath;

			constexpr static const char* TexturedPipelineName = "Textured";
			std::unique_ptr<GraphicsPipelineInstanceInfo> TexturedPipelineInfo;
			std::shared_ptr<ShaderProgramVulkan> TexturedShaderProgram;

			PipelineRenderableConveyor TexturedConveyor;

			const char* TestTexturePath = "Dough/res/images/testTexture.jpg";
			const char* TestTexture2Path = "Dough/res/images/testTexture2.jpg";
			std::shared_ptr<TextureVulkan> TestTexture1;
			std::shared_ptr<TextureVulkan> TestTexture2;

			bool GpuResourcesLoaded = false;
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
		std::unique_ptr<LineDemo> mLineDemo;
		std::unique_ptr<BoundingBoxDemo> mBoundingBoxDemo;
		std::unique_ptr<TileMapDemo> mTileMapDemo;

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
		void closeDemos();
		void initSharedResources();
		void closeSharedResources();
	};
}
