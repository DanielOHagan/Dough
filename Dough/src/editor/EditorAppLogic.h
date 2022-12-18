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
#include "editor/EditorGui.h"

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
		std::shared_ptr<IApplicationLogic> mInnerAppLogic;

		struct EditorSettings {
			//ImGui window rendering controls
			bool RenderDebugWindow = true;

			//ImGui menu
			bool EditorCollapseMenuOpen = true;
			bool InnerAppCollapseMenu = true;
			bool RenderingCollapseMenuOpen = true;
			bool CameraCollapseMenuOpen = true;

			bool InnerAppEditorWindowDisplay = true;

			//-----Editor Camera(s)-----
			bool UseOrthographicCamera = true;

			//-----Close App Functionality-----
			const float QuitHoldTimeRequired = 1.5f;
			float QuitButtonHoldTime = 0.0f;
		};

		//Editor Cameras
		std::shared_ptr<EditorOrthoCameraController> mOrthoCameraController;
		std::shared_ptr<EditorPerspectiveCameraController> mPerspectiveCameraController;

		//Editor settings
		std::unique_ptr<EditorSettings> mEditorSettings;
		std::unique_ptr<PausableTimer> mInnerAppTimer;
		EInnerAppState mInnerAppState;

	public:
		EditorAppLogic(std::shared_ptr<IApplicationLogic> innerApp);
		EditorAppLogic(const EditorAppLogic& copy) = delete;
		EditorAppLogic operator=(const EditorAppLogic& assignment) = delete;

		virtual void init(float aspectRatio) override;
		virtual void update(float delta) override;
		virtual void render() override;
		virtual void imGuiRender(float delta) override;
		virtual void close() override;

		virtual void onResize(float aspectRatio) override;

	private:
		//This renders the Editor windows
		void imGuiRenderDebugWindow(float delta);
	};
}
