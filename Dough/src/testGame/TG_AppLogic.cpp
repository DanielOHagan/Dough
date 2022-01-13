#include "testGame/TG_AppLogic.h"

#include "dough/Utils.h"
#include "dough/rendering/ObjInit.h"
#include "dough/application/Application.h"

#define GET_RENDERER Application::get().getRenderer()

namespace TG {

	TG_AppLogic::TG_AppLogic()
	:	IApplicationLogic() 
	{}

	void TG_AppLogic::init(float aspectRatio) {
		mTestTexture1 = ObjInit::texture(*testTexturePath);
		mTestTexture2 = ObjInit::texture(*testTexture2Path);

		mShaderProgram = ObjInit::shaderProgram(
			ObjInit::shader(
				EShaderType::VERTEX,
				*vertShaderPath
			),
			ObjInit::shader(
				EShaderType::FRAGMENT,
				*fragShaderPath
			)
		);

		ShaderUniformLayout& layout = mShaderProgram->getUniformLayout();
		layout.setValue(0, sizeof(UniformBufferObject));
		layout.setTexture(1, { mTestTexture1->getImageView(), mTestTexture1->getSampler() });

		TG_mOrthoCameraController = std::make_shared<TG::TG_OrthoCameraController>(aspectRatio);

		m_TestVAO_VertexArray = ObjInit::vertexArray();
		std::shared_ptr<VertexBufferVulkan> testVAO_VertexBuffer = ObjInit::stagedVertexBuffer(
			{
				{EDataType::FLOAT2, "mVertPos"},
				{EDataType::FLOAT3, "mColour"},
				{EDataType::FLOAT2, "mTexCoord"},
				{EDataType::FLOAT, "mTexIndex"}
			},
			vertices.data(),
			sizeof(vertices[0]) * vertices.size(),
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		);
		m_TestVAO_VertexArray->addVertexBuffer(testVAO_VertexBuffer);
		std::shared_ptr<IndexBufferVulkan> testVAO_IndexBuffer = ObjInit::stagedIndexBuffer(
			indices.data(),
			sizeof(indices[0]) * indices.size(),
			static_cast<uint32_t>(indices.size())
		);
		m_TestVAO_VertexArray->setIndexBuffer(testVAO_IndexBuffer);

		GET_RENDERER.openPipeline(*mShaderProgram);
	}

	void TG_AppLogic::update(float delta) {
		RendererVulkan& renderer = GET_RENDERER;

		TG_mOrthoCameraController->onUpdate(delta);

		renderer.beginScene(TG_mOrthoCameraController->getCamera());
		renderer.temp_addVaoDrawCommands(*m_TestVAO_VertexArray);
		renderer.endScene();
	}

	void TG_AppLogic::close() {
		RendererVulkan& renderer = GET_RENDERER;

		renderer.closeGpuResource(*mShaderProgram);
		renderer.closeGpuResource(*m_TestVAO_VertexArray);
		renderer.closeGpuResource(*mTestTexture1);
		renderer.closeGpuResource(*mTestTexture2);
	}

	void TG_AppLogic::onResize(float aspectRatio) {
		TG_mOrthoCameraController->onViewportResize(aspectRatio);
	}
}
