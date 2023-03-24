#pragma once

#include "dough/application/IApplicationLogic.h"
#include "dough/rendering/pipeline/shader/ShaderProgramVulkan.h"
#include "dough/rendering/buffer/VertexArrayVulkan.h"
#include "dough/rendering/textures/TextureVulkan.h"
#include "dough/rendering/Config.h"
#include "dough/scene/geometry/primitives/Quad.h"
#include "dough/rendering/renderables/RenderableModel.h"
#include "dough/rendering/renderables/SimpleRenderable.h"
#include "dough/rendering/pipeline/GraphicsPipelineVulkan.h"
#include "dough/time/PausableTimer.h"
#include "dough/input/Input.h"
#include "dough/input/AInputLayer.h"

#include "editor/EditorOrthoCameraController.h"
#include "editor/EditorPerspectiveCameraController.h"
#include "editor/ui/EditorGui.h"

using namespace DOH;

namespace DOH::EDITOR {

	//TODO:: Allow for stopping which resets the scene (re-init everything app-specific)
	enum class EInnerAppState {
		NONE = 0,

		PAUSED,
		PLAYING,
		STOPPED
	};

	class EditorInputLayer : public AInputLayer {
	
	public:
		EditorInputLayer()
		:	AInputLayer("Editor")
		{}

		//EditorInputLayer isn't manually handling events so all functions are empty, return false, or return 
		virtual bool handleKeyPressed(int keyCode, bool pressed) override { return EditorGui::isGuiHandlingKeyboardInput(); }
		virtual bool handleMouseButtonPressed(int button, bool pressed) override { return EditorGui::isGuiHandlingMouseInput(); }
		virtual bool handleMouseMoved(float x, float y) override { return EditorGui::isGuiHandlingMouseInput(); }
		virtual bool handleMouseScroll(float offsetX, float offsetY) override { return EditorGui::isGuiHandlingMouseInput(); }
		virtual void resetCycleData() override {};
		virtual void reset() override {};

		//NOTE:: Device Input is handled by EditorUI, no need to handle it manually so no need to override original function
		//virtual inline bool hasDeviceInput() const override { return false; }

		virtual bool isKeyPressed(int keyCode) const override { return false; }
		virtual bool isMouseButtonPressed(int button) const override { return false; }
		virtual inline bool isMouseScrollingUp() const override { return false; }
		virtual inline bool isMouseScrollingDown() const override { return false; }
		virtual inline const glm::vec2 getCursorPos() const override { return { 0.0f, 0.0f }; }
	};

	class EditorAppLogic : public IApplicationLogic {

	private:
		struct EditorSettings {
			//ImGui window rendering controls
			bool RenderDebugWindow = true;

			//ImGui menu
			bool EditorCollapseMenuOpen = true;
			bool InnerAppCollapseMenu = true;
			bool RenderingCollapseMenuOpen = true;
			bool CameraCollapseMenuOpen = true;

			bool InnerAppEditorWindowDisplay = true;

			bool RenderPerformancePlotLines = false;
			const std::array<ETimeUnit, 2> RenderingTimeUnitsAvailable = {
				ETimeUnit::SECOND,
				ETimeUnit::MILLISECOND
			};
			ETimeUnit RenderingTimeUnit = ETimeUnit::MILLISECOND;

			//-----Editor Camera(s)-----
			bool UseOrthographicCamera = true;

			//-----Close App Functionality-----
			const float QuitHoldTimeRequired = 1.5f;
			float QuitButtonHoldTime = 0.0f;
		};

		std::shared_ptr<IApplicationLogic> mInnerAppLogic;

		std::shared_ptr<EditorInputLayer> mEditorInputLayer;
		std::optional<std::shared_ptr<AInputLayer>> mInnerAppInputLayer;

		//Editor Cameras
		std::shared_ptr<EditorOrthoCameraController> mOrthoCameraController;
		std::shared_ptr<EditorPerspectiveCameraController> mPerspectiveCameraController;

		//Editor settings
		std::unique_ptr<EditorSettings> mEditorSettings;
		std::unique_ptr<PausableTimer> mInnerAppTimer;
		EInnerAppState mInnerAppState;
		bool mEditorGuiFocused;

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
