#pragma once

#include "dough/application/IApplicationLogic.h"
#include "dough/rendering/pipeline/shader/ShaderProgramVulkan.h"
#include "dough/rendering/buffer/VertexArrayVulkan.h"
#include "dough/rendering/TextureVulkan.h"

#include "testGame/TG_OrthoCameraController.h"

using namespace DOH;

namespace TG {

	class TG_AppLogic : public IApplicationLogic {

		//Scene Data
		//TODO:: learn strings properly
		const std::unique_ptr<std::string> texturedShaderVertPath = std::make_unique<std::string>("res/shaders/spv/Textured.vert.spv");
		const std::unique_ptr<std::string> texturedShaderFragPath = std::make_unique<std::string>("res/shaders/spv/Textured.frag.spv");
		const std::unique_ptr<std::string> flatColourShaderVertPath = std::make_unique<std::string>("res/shaders/spv/FlatColour.vert.spv");
		const std::unique_ptr<std::string> flatColourShaderFragPath = std::make_unique<std::string>("res/shaders/spv/FlatColour.frag.spv");
		const std::unique_ptr<std::string> testTexturePath = std::make_unique<std::string>("res/images/testTexture.jpg");
		const std::unique_ptr<std::string> testTexture2Path = std::make_unique<std::string>("res/images/testTexture2.jpg");
		const std::vector<Vertex3D> mSceneVertices{
			//	x		y		z		r		g		b		u		v		texIndex
			{{	-0.5f,	-0.5f,	1.0f},	{0.0f,	1.0f,	0.0f},	{0.0f,	1.0f},	{1.0f}},
			{{	 0.5f,	-0.5f,	1.0f},	{0.0f,	1.0f,	0.0f},	{1.0f,	1.0f},	{1.0f}},
			{{	 0.5f,	0.5f,	1.0f},	{0.0f,	1.0f,	0.0f},	{1.0f,	0.0f},	{1.0f}},
			{{	-0.5f,	0.5f,	1.0f},	{0.0f,	1.0f,	0.0f},	{0.0f,	0.0f},	{1.0f}},

			{{0.00f, 0.00f, 1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}, {0.0f}},
			{{1.00f, 0.00f, 1.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f}, {0.0f}},
			{{1.00f, 1.00f, 1.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}, {0.0f}},
			{{0.00f, 1.00f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {0.0f}}
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
		std::unique_ptr<std::string> mUiShaderVertPath = std::make_unique<std::string>("res/shaders/spv/SimpleUi.vert.spv");
		std::unique_ptr<std::string> mUiShaderFragPath = std::make_unique<std::string>("res/shaders/spv/SimpleUi.frag.spv");
		const std::vector<VertexUi2D> mUiVertices{
			//	x		y			r		g		b
			{{	-1.0f,	-0.90f},	{0.0f,	1.0f,	0.0f}}, //bot-left
			{{	-0.75f,	-0.90f},	{0.0f,	0.5f,	0.5f}}, //bot-right
			{{	-0.75f,	-0.65f},	{0.0f,	0.0f,	1.0f}}, //top-right
			{{	-1.0f,	-0.65f},	{0.0f,	0.5f,	0.5f}}  //top-left
		};
		const std::vector<uint16_t> mUiIndices{
			0, 1, 2, 2, 3, 0
		};
		//UI Objects
		std::shared_ptr<ShaderProgramVulkan> mUiShaderProgram;
		std::shared_ptr<VertexArrayVulkan> mUiVao;
		//NOTE:: in OpenGL space because glm
		glm::mat4x4 mUiProjMat;

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
	};
}
