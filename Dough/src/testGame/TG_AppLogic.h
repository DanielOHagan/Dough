#pragma once

#include "dough/application/IApplicationLogic.h"
#include "dough/rendering/pipeline/shader/ShaderProgramVulkan.h"
#include "dough/rendering/buffer/VertexArrayVulkan.h"
#include "dough/rendering/TextureVulkan.h"

#include "testGame/TG_OrthoCameraController.h"

using namespace DOH;

namespace TG {

	class TG_AppLogic : public IApplicationLogic {

		//TODO:: learn strings properly
		const std::unique_ptr<std::string> texturedShaderVertPath = std::make_unique<std::string>("res/shaders/spv/Textured.vert.spv");
		const std::unique_ptr<std::string> texturedShaderFragPath = std::make_unique<std::string>("res/shaders/spv/Textured.frag.spv");
		const std::unique_ptr<std::string> flatColourShaderVertPath = std::make_unique<std::string>("res/shaders/spv/FlatColour.vert.spv");
		const std::unique_ptr<std::string> flatColourShaderFragPath = std::make_unique<std::string>("res/shaders/spv/FlatColour.frag.spv");
		const std::unique_ptr<std::string> testTexturePath = std::make_unique<std::string>("res/images/testTexture.jpg");
		const std::unique_ptr<std::string> testTexture2Path = std::make_unique<std::string>("res/images/testTexture2.jpg");

		const std::vector<Vertex> vertices{
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
			alignas(16) glm::mat4 projView;
		};

		std::shared_ptr<TG_OrthoCameraController> TG_mOrthoCameraController;
		std::shared_ptr<ShaderProgramVulkan> mShaderProgram;

		std::shared_ptr<VertexArrayVulkan> m_TestVAO_VertexArray;

		std::shared_ptr<TextureVulkan> mTestTexture1;
		std::shared_ptr<TextureVulkan> mTestTexture2;

	public:
		TG_AppLogic();
		TG_AppLogic(const TG_AppLogic& copy) = delete;
		TG_AppLogic operator=(const TG_AppLogic& assignment) = delete;

		virtual void init(float aspectRatio) override;
		virtual void update(float delta) override;
		virtual void close() override;

		virtual void onResize(float aspectRatio) override;

	};
}
