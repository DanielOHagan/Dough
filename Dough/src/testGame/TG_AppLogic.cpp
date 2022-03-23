#include "testGame/TG_AppLogic.h"

#include "dough/Utils.h"
#include "dough/rendering/ObjInit.h"
#include "dough/application/Application.h"
#include "dough/input/InputCodes.h"
#include "dough/Logging.h"

#include <imgui.h>

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
		renderer.getContext().addVaoToSceneDrawList(*mSceneVertexArray);
		renderer.endScene();

		renderer.beginUi(mUiProjMat);
		renderer.getContext().addVaoToUiDrawList(*mUiVao);
		renderer.endUi();
	}

	void TG_AppLogic::imGuiRender() {
		ImGui::ShowDemoWindow();

		ImGui::Begin("Test Window");
		if (ImGui::Button("Log text")) {
			LOGLN("Logged some text");
		}
		ImGui::End();
	}

	void TG_AppLogic::close() {
		RendererVulkan& renderer = GET_RENDERER;

		renderer.closeGpuResource(*mSceneShaderProgram);
		renderer.closeGpuResource(*mSceneVertexArray);
		renderer.closeGpuResource(*mUiShaderProgram);
		renderer.closeGpuResource(*mUiVao);
		renderer.closeGpuResource(*mTestTexture1);
		renderer.closeGpuResource(*mTestTexture2);
	}

	void TG_AppLogic::onResize(float aspectRatio) {
		TG_mOrthoCameraController->onViewportResize(aspectRatio);
		setUiProjection(aspectRatio);
	}

	void TG_AppLogic::initScene(float aspectRatio) {
		mTestTexture1 = ObjInit::texture(*testTexturePath);
		mTestTexture2 = ObjInit::texture(*testTexture2Path);

		mSceneShaderProgram = ObjInit::shaderProgram(
			ObjInit::shader(
				EShaderType::VERTEX,
				*texturedShaderVertPath
				//*flatColourShaderVertPath
			),
			ObjInit::shader(
				EShaderType::FRAGMENT,
				*texturedShaderFragPath
				//*flatColourShaderFragPath
			)
		);

		ShaderUniformLayout& layout = mSceneShaderProgram->getUniformLayout();
		layout.setValue(0, sizeof(UniformBufferObject));
		layout.setTexture(1, { mTestTexture1->getImageView(), mTestTexture1->getSampler() });

		TG_mOrthoCameraController = std::make_shared<TG::TG_OrthoCameraController>(aspectRatio);

		mSceneVertexArray = ObjInit::vertexArray();
		std::shared_ptr<VertexBufferVulkan> testVAO_VertexBuffer = ObjInit::stagedVertexBuffer(

			//TODO:: I think buffer layouts might be pointless as Vertex structs provide the data (e.g. stride, internal offsets)

			{
				//{EDataType::FLOAT2, "mVertPos"},
				{EDataType::FLOAT3, "mVertPos"},
				{EDataType::FLOAT3, "mColour"},
				{EDataType::FLOAT2, "mTexCoord"},
				{EDataType::FLOAT, "mTexIndex"}
			},
			mSceneVertices.data(),
			sizeof(Vertex3D) * mSceneVertices.size(),
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		);
		mSceneVertexArray->addVertexBuffer(testVAO_VertexBuffer);
		std::shared_ptr<IndexBufferVulkan> testVAO_IndexBuffer = ObjInit::stagedIndexBuffer(
			indices.data(),
			sizeof(indices[0]) * indices.size(),
			static_cast<uint32_t>(indices.size())
		);
		mSceneVertexArray->setIndexBuffer(testVAO_IndexBuffer);

		GET_RENDERER.prepareScenePipeline(*mSceneShaderProgram);
	}

	void TG_AppLogic::initUi() {
		mUiShaderProgram = ObjInit::shaderProgram(
			ObjInit::shader(EShaderType::VERTEX, *mUiShaderVertPath),
			ObjInit::shader(EShaderType::FRAGMENT, *mUiShaderFragPath)
		);
		mUiShaderProgram->getUniformLayout().setValue(0, sizeof(UniformBufferObject));

		mUiVao = ObjInit::vertexArray();
		std::shared_ptr<VertexBufferVulkan> appUiVb = ObjInit::stagedVertexBuffer(
			{
				{EDataType::FLOAT2, "mVertPos"},
				{EDataType::FLOAT3, "mColour"}
			},
			mUiVertices.data(),
			sizeof(VertexUi2D) * mUiVertices.size(),
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
}
