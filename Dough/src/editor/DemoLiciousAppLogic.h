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
#include "dough/scene/geometry/primitives/Circle.h"
#include "editor/EditorPerspectiveCameraController.h"
#include "editor/EditorOrthoCameraController.h"

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
		//Resources used by more than one demo
		struct SharedDemoResources {
			static const EVertexType TexturedVertexType = EVertexType::VERTEX_3D_TEXTURED;

			static const char* TexturedShaderVertPath;
			static const char* TexturedShaderFragPath;

			static const char* PERSP_SCENE_CAM_NAME;
			static const char* ORTHO_UI_CAM_NAME;
			//Using editor camera controllers to save making a new class
			std::unique_ptr<EDITOR::EditorPerspectiveCameraController> PerspectiveSceneCameraController;
			std::unique_ptr<EDITOR::EditorOrthoCameraController> OrthoUiCameraController;

			std::shared_ptr<DefaultInputLayer> InputLayer;

			constexpr static const char* TexturedPipelineName = "Textured";
			std::unique_ptr<GraphicsPipelineInstanceInfo> TexturedPipelineInfo;
			std::shared_ptr<ShaderProgram> TexturedShaderProgram;
			std::shared_ptr<ShaderVulkan> TexturedVertexShader;
			std::shared_ptr<ShaderVulkan> TexturedFragmentShader;

			PipelineRenderableConveyor TexturedConveyor;

			const char* TestTexturePath = "Dough/Dough/res/images/testTexture.jpg";
			const char* TestTexture2Path = "Dough/Dough/res/images/testTexture2.jpg";
			std::shared_ptr<TextureVulkan> TestTexture1;
			std::shared_ptr<TextureVulkan> TestTexture2;
			VkDescriptorSet TestTexture1DescSet = VK_NULL_HANDLE;
			VkDescriptorSet TestTexture2DescSet = VK_NULL_HANDLE;

			float AspectRatio = 1.0f;

			bool GpuResourcesLoaded = false;
		};

		class ADemo {
		protected:
			SharedDemoResources& SharedResources;
		public:
			ADemo(SharedDemoResources& sharedResources)
			:	SharedResources(sharedResources)
			{}
			virtual void init() = 0;
			virtual void close() = 0;
			virtual void update(float delta) = 0;
			virtual void render() = 0;
			virtual void renderImGuiMainTab() = 0;
			virtual void renderImGuiExtras() = 0;
			virtual const char* getName() = 0;
		};

		class ObjModelsDemo : public ADemo {
		public:
			struct UniformBufferObject {
				glm::mat4x4 ProjView;
			};

			//Models using EVertexType::VERTEX_3D
			const std::array<const char*, 4> ObjModelFilePaths = {
				"Dough/Dough/res/models/testCube.obj",
				"Dough/Dough/res/models/spoon.obj",
				"Dough/Dough/res/models/teacup.obj",
				"Dough/Dough/res/models/teapot.obj"
			};
			const uint32_t DefaultObjFilePathIndex = 0;

			const char* FlatColourShaderVertPath = "Dough/Dough/res/shaders/spv/FlatColour.vert.spv";
			const char* FlatColourShaderFragPath = "Dough/Dough/res/shaders/spv/FlatColour.frag.spv";
			const char* ScenePipelineName = "ObjScene";
			const char* SceneWireframePipelineName = "ObjWireframe";
			const StaticVertexInputLayout& ColouredVertexInputLayout = StaticVertexInputLayout::get(EVertexType::VERTEX_3D);
			const StaticVertexInputLayout& TexturedVertexInputLayout = StaticVertexInputLayout::get(EVertexType::VERTEX_3D_TEXTURED);
			std::shared_ptr<ShaderProgram> SceneShaderProgram;
			std::shared_ptr<ShaderVulkan> SceneVertexShader;
			std::shared_ptr<ShaderVulkan> SceneFragmentShader;
			std::shared_ptr<DescriptorSetsInstanceVulkan> SceneDescSetsInstance;

			std::unique_ptr<GraphicsPipelineInstanceInfo> ScenePipelineInfo;
			std::unique_ptr<GraphicsPipelineInstanceInfo> SceneWireframePipelineInfo;

			std::vector<std::shared_ptr<ModelVulkan>> LoadedModels;
			std::vector<std::shared_ptr<RenderableModelVulkan>> RenderableObjects;

			std::shared_ptr<ModelVulkan> TexturedModel;
			std::shared_ptr<DescriptorSetsInstanceVulkan> TexturedModelDescriptorSets;
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

			ObjModelsDemo(SharedDemoResources& sharedResources)
			:	ADemo(sharedResources)
			{}
			virtual void init() override;
			virtual void close() override;
			virtual void update(float delta) override;
			virtual void render() override;
			virtual void renderImGuiMainTab() override;
			virtual void renderImGuiExtras() override;
			virtual const char* getName() override { return "ObjModels"; }

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

		class CustomDemo : public ADemo {
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

			const char* UiShaderVertPath = "Dough/Dough/res/shaders/spv/SimpleUi.vert.spv";
			const char* UiShaderFragPath = "Dough/Dough/res/shaders/spv/SimpleUi.frag.spv";
			const std::vector<Vertex2d> UiVertices = {
				//	x		y			r		g		b		a
				{{	-1.0f,	-0.90f},	{0.0f,	1.0f,	0.0f,	1.0f}},	//bot-left
				{{	-0.75f,	-0.90f},	{0.0f,	0.5f,	0.5f,	1.0f}},	//bot-right
				{{	-0.75f,	-0.65f},	{0.0f,	0.0f,	1.0f,	1.0f}},	//top-right
				{{	-1.0f,	-0.65f},	{0.0f,	0.5f,	0.5f,	1.0f}}	//top-left
			};
			const std::vector<uint32_t> UiIndices = { 0, 1, 2, 2, 3, 0 };

			std::unique_ptr<GraphicsPipelineInstanceInfo> UiPipelineInfo;
			std::shared_ptr<ShaderProgram> UiShaderProgram;
			std::shared_ptr<ShaderVulkan> UiVertexShader;
			std::shared_ptr<ShaderVulkan> UiFragmentShader;
			PipelineRenderableConveyor CustomUiConveyor;
			constexpr static const char* UiPipelineName = "CustomUi";

			//Renderables are for an easier API overall, they do not fully "own" an object,
			// therefore, they are not responsible for clearing up resources
			std::shared_ptr<SimpleRenderable> SceneRenderable;
			std::shared_ptr<DescriptorSetsInstanceVulkan> SceneDescSetsInstance;
			std::shared_ptr<VertexArrayVulkan> SceneVao;
			const StaticVertexInputLayout& SceneVertexInputLayout = StaticVertexInputLayout::get(SharedDemoResources::TexturedVertexType);
			std::shared_ptr<SimpleRenderable> UiRenderable;
			std::shared_ptr<VertexArrayVulkan> UiVao;
			const StaticVertexInputLayout& UiVertexInputLayout = StaticVertexInputLayout::get(EVertexType::VERTEX_2D);

			bool Update = false;
			bool RenderScene = false;
			bool RenderUi = false;
			bool GpuResourcesLoaded = false;

			CustomDemo(SharedDemoResources& sharedResources)
			:	ADemo(sharedResources)
			{}
			virtual void init() override;
			virtual void close() override;
			virtual void update(float delta) override;
			virtual void render() override;
			virtual void renderImGuiMainTab() override;
			virtual void renderImGuiExtras() override;
			virtual const char* getName() override { return "Custom"; }
		};
		
		class ShapesDemo : public ADemo {
		private:
			class GridDemo : public ADemo {
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

				void populateTestGrid(uint32_t width, uint32_t height);

				GridDemo(SharedDemoResources& sharedResources)
				:	ADemo(sharedResources)
				{}
				virtual void init() override;
				virtual void close() override;
				virtual void update(float delta) override;
				virtual void render() override;
				virtual void renderImGuiMainTab() override;
				virtual void renderImGuiExtras() override;
				virtual const char* getName() override { return "Shapes::Grid"; }
			};

			class BouncingQuadDemo : public ADemo {
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

				BouncingQuadDemo(SharedDemoResources& sharedResources)
				:	ADemo(sharedResources)
				{}
				virtual void init() override;
				virtual void close() override;
				virtual void update(float delta) override;
				virtual void render() override;
				virtual void renderImGuiMainTab() override;
				virtual void renderImGuiExtras() override;
				virtual const char* getName() override { return "Shapes::BouncingQuad"; }

				void addRandomQuads(size_t count);
				void popQuads(size_t count);
			};

			class CircleDemo : public ADemo {
			public:
				#if defined (_DEBUG)
				static const uint32_t MaxCirclesCount = 40000;
				#else
				static const uint32_t MaxCirclesCount = 100000;
				#endif
				std::vector<Circle> CirclesScene;
				//std::vector<Circle> CirclesUi;
				Circle TestCircleScene;
				Circle TestCircleUi;

				bool Update = false;
				bool Render = false;
				bool DrawColour = false;
				bool RenderTestCircles = false;
				int AddNewCount = 0;
				int PopCount = 0;

				CircleDemo(SharedDemoResources& sharedResources)
				:	ADemo(sharedResources)
				{}
				virtual void init() override;
				virtual void close() override;
				virtual void update(float delta) override;
				virtual void render() override;
				virtual void renderImGuiMainTab() override;
				virtual void renderImGuiExtras() override;
				virtual const char* getName() override { return "Shapes::Circle"; }

				void addRandomCirclesScene(size_t count);
				void popScene(size_t count);
			};
		public:
			std::unique_ptr<GridDemo> mGridDemo;
			std::unique_ptr<BouncingQuadDemo> mBouncingQuadDemo;
			std::unique_ptr<CircleDemo> mCircleDemo;

			ShapesDemo(SharedDemoResources& sharedResources)
			:	ADemo(sharedResources)
			{}
			virtual void init() override;
			virtual void close() override;
			virtual void update(float delta) override;
			virtual void render() override;
			virtual void renderImGuiMainTab() override;
			virtual void renderImGuiExtras() override;
			virtual const char* getName() override { return "Shapes"; }
		};

		class TextDemo : public ADemo {
		public:
			static constexpr size_t StringLengthLimit = 1000; //Arbitrary limit
		
			std::unique_ptr<TextString> SoftMaskScene;
			std::unique_ptr<TextString> MsdfTextScene;
			std::unique_ptr<TextString> SoftMaskTextUi;
			std::unique_ptr<TextString> MsdfTextUi;
			char SoftMaskSceneStringBuffer[StringLengthLimit] = "This is the default text string. Use the Text Field to change me!";
			char MsdfSceneStringBuffer[StringLengthLimit] = "Default MSDF text string, use the text field to change me!";
			char SoftMaskUiStringBuffer[StringLengthLimit] = "This is the default text string. Use the Text Field to change me!";
			char MsdfUiStringBuffer[StringLengthLimit] = "Default MSDF text string, use the text field to change me!";
			glm::vec4 Colour = { 1.0f, 1.0f, 1.0f, 1.0f };
		
			bool Update = false;
			bool Render = false;

			TextDemo(SharedDemoResources& sharedResources)
			:	ADemo(sharedResources)
			{}
			virtual void init() override;
			virtual void close() override;
			virtual void update(float delta) override;
			virtual void render() override;
			virtual void renderImGuiMainTab() override;
			virtual void renderImGuiExtras() override;
			virtual const char* getName() override { return "Text"; }
		};

		class LineDemo : public ADemo {
		public:
			static constexpr uint32_t LINE_2D_INPUT_COMPONENT_COUNT = 8;
			static constexpr uint32_t LINE_3D_INPUT_COMPONENT_COUNT = 10; //3 for start, 3 for end, 4 for colour
			std::vector<float> LineData2d;
			std::vector<float> LineData3d;
			std::reference_wrapper<TextDemo> TextDemoRef;
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

			LineDemo(SharedDemoResources& sharedResources, TextDemo& textDemo)
			:	ADemo(sharedResources),
				TextDemoRef(textDemo)
			{}
			virtual void init() override;
			virtual void close() override;
			virtual void update(float delta) override;
			virtual void render() override;
			virtual void renderImGuiMainTab() override;
			virtual void renderImGuiExtras() override;
			virtual const char* getName() override { return "Line"; }

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

		class BoundingBoxDemo : public ADemo {
		public:
			std::vector<Quad> Quads;
			std::unique_ptr<BoundingBox2d> BoundingBox;

			Quad TempQuadToAdd = {};

			bool Update = false;
			bool Render = false;

			BoundingBoxDemo(SharedDemoResources& sharedResources)
			:	ADemo(sharedResources)
			{}
			virtual void init() override;
			virtual void close() override;
			virtual void update(float delta) override;
			virtual void render() override;
			virtual void renderImGuiMainTab() override;
			virtual void renderImGuiExtras() override;
			virtual const char* getName() override { return "BoundingBox"; }
		};

		class TileMapDemo : public ADemo {
		public:
			std::unique_ptr<TileMap> SceneTileMap;
			std::unique_ptr<TextureAtlasAnimationController> PreviewAnimationController;
			Quad PreviewQuad;
			Quad AnimatedQuad;
			const char* PreviewedInnerTexture = "NONE";

			bool Update = false;
			bool Render = false;
			bool RenderPreviewQuad = false;

			TileMapDemo(SharedDemoResources& sharedResources)
			:	ADemo(sharedResources)
			{}
			virtual void init() override;
			virtual void close() override;
			virtual void update(float delta) override;
			virtual void render() override;
			virtual void renderImGuiMainTab() override;
			virtual void renderImGuiExtras() override;
			virtual const char* getName() override { return "TileMap"; }
		};

		//To show the use of multiple cameras used in the same render pass
		class MultiCameraPassDemo : public ADemo {
		public:
			//Some cameras and textured shaders are owned by mSharedDemoResources

			static const char* ORTHO_SCENE_CAM_NAME;
			static const char* PERSP_UI_CAM_NAME;

			//Ui3D Pipeline
			const char* FlatColourShaderVertPath = "Dough/Dough/res/shaders/spv/FlatColour.vert.spv";
			const char* FlatColourShaderFragPath = "Dough/Dough/res/shaders/spv/FlatColour.frag.spv";
			const char* Ui3dPipelineName = "MultiCamera_3dUi";
			const StaticVertexInputLayout& ColouredVertexInputLayout = StaticVertexInputLayout::get(EVertexType::VERTEX_3D);
			std::shared_ptr<ShaderProgram> Ui3dShaderProgram;
			std::shared_ptr<ShaderVulkan> Ui3dVertexShader;
			std::shared_ptr<ShaderVulkan> Ui3dFragmentShader;
			std::shared_ptr<DescriptorSetsInstanceVulkan> Ui3dDescSetsInstance;
			std::unique_ptr<GraphicsPipelineInstanceInfo> Ui3dPipelineInfo;
			PipelineRenderableConveyor Ui3dPipelineConveyor;

			std::unique_ptr<EDITOR::EditorOrthoCameraController> OrthoSceneCameraController;
			std::unique_ptr<EDITOR::EditorPerspectiveCameraController> PerspectiveUiCameraController;

			//Basic quad to show normal 2d UI
			Quad UiQuad;

			//3d Model rendered in the UI
			std::string UiCubeModelName;
			std::shared_ptr<TransformationData> CubeUiTransformation;
			std::shared_ptr<ModelVulkan> Ui3dModel; //Copied model from Obj demo but is it's own instance so ownership is kept here
			std::shared_ptr<RenderableModelVulkan> Ui3dModelInstance;

			bool Update = false;
			bool Render = false;
			bool RenderUiQuad = false;
			bool RenderUiModel = false;

			MultiCameraPassDemo(SharedDemoResources& sharedResources)
			:	ADemo(sharedResources)
			{}
			virtual void init() override;
			virtual void close() override;
			virtual void update(float delta) override;
			virtual void render() override;
			virtual void renderImGuiMainTab() override;
			virtual void renderImGuiExtras() override;
			virtual const char* getName() override { return "MultiCameraPass"; }
		};

		struct ImGuiSettings {
			bool CurrentDemoCollapseMenuOpen = true;
			bool RenderObjModelsList = false;
		};

		//Demos
		std::unique_ptr<ObjModelsDemo> mObjModelsDemo;
		std::unique_ptr<CustomDemo> mCustomDemo;
		std::unique_ptr<ShapesDemo> mShapesDemo;
		std::unique_ptr<TextDemo> mTextDemo;
		std::unique_ptr<LineDemo> mLineDemo;
		std::unique_ptr<BoundingBoxDemo> mBoundingBoxDemo;
		std::unique_ptr<TileMapDemo> mTileMapDemo;
		std::unique_ptr<MultiCameraPassDemo> mMultiCameraPassDemo;
		//References to usable demos (mShapesDemo is just a wrapper of "inner" demos so it's not needed here).
		// This is primarily used for convenience for Editor GUI. 
		// IMPORTANT:: NOT TO BE USED FOR OBJECT LIFETIME!
		std::vector<std::reference_wrapper<ADemo>> mDemos;
		uint32_t mSelectedDemoIndex = UINT32_MAX;

		std::unique_ptr<SharedDemoResources> mSharedDemoResources;
		std::unique_ptr<ImGuiSettings> mImGuiSettings;

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
		void initDemos(float aspectRatio);
		void closeDemos();
		void initSharedResources(float aspectRatio);
		void closeSharedResources();
		void updateCamerasAspectRatio(float aspectRatio);
	};
}
