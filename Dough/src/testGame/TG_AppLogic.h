#pragma once

#include "dough/application/IApplicationLogic.h"
#include "dough/rendering/pipeline/shader/ShaderProgramVulkan.h"
#include "dough/rendering/buffer/VertexArrayVulkan.h"
#include "dough/rendering/TextureVulkan.h"
#include "dough/rendering/Config.h"

#include "testGame/TG_OrthoCameraController.h"

using namespace DOH;

namespace TG {

	class TG_AppLogic : public IApplicationLogic {

		//Scene Data
		const std::string texturedShaderVertPath = "res/shaders/spv/Textured.vert.spv";
		const std::string texturedShaderFragPath = "res/shaders/spv/Textured.frag.spv";
		const std::string flatColourShaderVertPath = "res/shaders/spv/FlatColour.vert.spv";
		const std::string flatColourShaderFragPath = "res/shaders/spv/FlatColour.frag.spv";
		const std::string quadBatchShaderVertPath = "res/shaders/spv/QuadBatch.vert.spv";
		const std::string quadBatchShaderFragPath = "res/shaders/spv/QuadBatch.frag.spv";
		const std::string testTexturePath = "res/images/testTexture.jpg";
		const std::string testTexture2Path = "res/images/testTexture2.jpg";
		const std::vector<Vertex3dTextured> mSceneVertices = {
			//	x		y		z		r		g		b		a		u		v		texIndex
			{{	-0.5f,	-0.5f,	1.0f},	{1.0f,	0.0f,	0.0f,	1.0f},	{0.0f,	1.0f},	{0.0f}},
			{{	 0.5f,	-0.5f,	1.0f},	{0.0f,	0.0f,	0.0f,	1.0f},	{1.0f,	1.0f},	{0.0f}},
			{{	 0.5f,	0.5f,	1.0f},	{0.0f,	0.0f,	1.0f,	1.0f},	{1.0f,	0.0f},	{0.0f}},
			{{	-0.5f,	0.5f,	1.0f},	{0.0f,	0.0f,	0.0f,	1.0f},	{0.0f,	0.0f},	{0.0f}},

			{{0.00f,	0.00f,	1.0f},	{0.0f,	0.60f,	0.0f,	1.0f},	{0.0f,	1.0f},	{1.0f}},
			{{1.00f,	0.00f,	1.0f},	{0.0f,	0.60f,	0.0f,	1.0f},	{1.0f,	1.0f},	{1.0f}},
			{{1.00f,	1.00f,	1.0f},	{0.0f,	0.60f,	0.0f,	1.0f},	{1.0f,	0.0f},	{1.0f}},
			{{0.00f,	1.00f,	1.0f},	{0.0f,	0.60f,	0.0f,	1.0f},	{0.0f,	0.0f},	{1.0f}}
		};
		const std::vector<uint16_t> indices{
			0, 1, 2, 2, 3, 0,
			4, 5, 6, 6, 7, 4
		};
		struct UniformBufferObject {
			glm::mat4 projView;
		};
		//Scene Objects
		std::shared_ptr<TG_OrthoCameraController> TG_mOrthoCameraController;
		std::shared_ptr<ShaderProgramVulkan> mSceneShaderProgram;
		std::shared_ptr<VertexArrayVulkan> mSceneVertexArray;
		std::shared_ptr<TextureVulkan> mTestTexture1;
		std::shared_ptr<TextureVulkan> mTestTexture2;

		//UI Data
		const std::string mUiShaderVertPath = "res/shaders/spv/SimpleUi.vert.spv";
		const std::string mUiShaderFragPath = "res/shaders/spv/SimpleUi.frag.spv";
		const std::vector<Vertex2d> mUiVertices = {
			//	x		y			r		g		b		a
			{{	-1.0f,	-0.90f},	{0.0f,	1.0f,	0.0f,	1.0f}}, //bot-left
			{{	-0.75f,	-0.90f},	{0.0f,	0.5f,	0.5f,	1.0f}}, //bot-right
			{{	-0.75f,	-0.65f},	{0.0f,	0.0f,	1.0f,	1.0f}}, //top-right
			{{	-1.0f,	-0.65f},	{0.0f,	0.5f,	0.5f,	1.0f}}  //top-left
		};
		const std::vector<uint16_t> mUiIndices{
			0, 1, 2, 2, 3, 0
		};
		//UI Objects
		std::shared_ptr<ShaderProgramVulkan> mUiShaderProgram;
		std::shared_ptr<VertexArrayVulkan> mUiVao;
		//NOTE:: in OpenGL space because glm
		glm::mat4x4 mUiProjMat;

		struct ImGuiSettings {
			//ImGui menu
			bool mApplicationCollapseMenuOpen = true;
			bool mRenderingCollapseMenuOpen = true;
			bool mCameraCollapseMenuOpen = false;
			bool mSceneDataCollapseMenuOpen = true;
			//Rendering
			bool mRenderScene = true;
			bool mRenderUi = true;
			bool mRenderBatchQuadScene = true;
			bool mRenderBatchQuadUi = true;
		} mImGuiSettings;
	public:
		TG_AppLogic();
		TG_AppLogic(const TG_AppLogic& copy) = delete;
		TG_AppLogic operator=(const TG_AppLogic& assignment) = delete;

		virtual void init(float aspectRatio) override;
		virtual void update(float delta) override;
		virtual void imGuiRender() override;
		virtual void close() override;

		virtual void onResize(float aspectRatio) override;

		void initScene(float aspectRatio);
		void initUi();

	private:
		inline void setUiProjection(float aspectRatio) {
			mUiProjMat = glm::ortho(-aspectRatio, aspectRatio, -1.0f, 1.0f, -1.0f, 1.0f);
			mUiProjMat[1][1] *= -1;
		}

		//ImGui convenience functions
		void imguiDisplayHelpTooltip(const char* message);
	};
}
