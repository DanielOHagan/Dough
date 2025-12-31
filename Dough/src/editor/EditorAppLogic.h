#pragma once

#include "dough/application/IApplicationLogic.h"
#include "dough/rendering/pipeline/ShaderProgram.h"
#include "dough/rendering/buffer/VertexArrayVulkan.h"
#include "dough/rendering/textures/TextureVulkan.h"
#include "dough/rendering/Config.h"
#include "dough/scene/geometry/primitives/Quad.h"
#include "dough/rendering/renderables/RenderableModel.h"
#include "dough/rendering/renderables/SimpleRenderable.h"
#include "dough/rendering/pipeline/GraphicsPipelineVulkan.h"
#include "dough/time/PausableTimer.h"
#include "dough/input/Input.h"
#include "dough/input/DeviceInput.h"
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
	private:
		static std::array<int, 14> EDITOR_DEFAULT_KEY_CODES;
		static std::array<int, 1> EDITOR_DEFAULT_MOUSE_CODES;

		std::shared_ptr<DeviceInputKeyboardMouse> mKeyboardMouseInput;

	public:
		constexpr static const char* EDITOR_INPUT_LAYER_NAME = "DOH Editor";

		EditorInputLayer();

		//EditorInputLayer isn't manually handling events so all functions are empty, return false, or return 
		virtual bool handleKeyPressed(int keyCode, bool pressed) override {
			return mKeyboardMouseInput->setKeyPressed(keyCode, pressed) || EditorGui::isGuiHandlingKeyboardInput();
		}
		virtual bool handleMouseButtonPressed(int button, bool pressed) override {
			return mKeyboardMouseInput->setMouseButtonPressed(button, pressed) || EditorGui::isGuiHandlingMouseInput();
		}
		virtual bool handleMouseMoved(float x, float y) override {
			//return EditorGui::isGuiHandlingMouseInput();
			return false;
		}
		virtual bool handleMouseScroll(float offsetX, float offsetY) override { return EditorGui::isGuiHandlingMouseInput(); }
		virtual void resetCycleData() override {}
		virtual void reset() override {}

		//virtual inline bool hasDeviceInput() const override { return false; }

		virtual inline bool isKeyPressed(int keyCode) const override { return !EditorGui::isGuiFocused() && mKeyboardMouseInput->isKeyPressed(keyCode); }
		virtual inline bool isMouseButtonPressed(int button) const override { return !EditorGui::isGuiFocused() && mKeyboardMouseInput->isMouseButtonPressed(button); }
		virtual inline bool isMouseScrollingUp() const override { return false; }
		virtual inline bool isMouseScrollingDown() const override { return false; }
		virtual inline const glm::vec2 getCursorPos() const override { return { 0.0f, 0.0f }; }
		virtual bool isActionActiveAND(const char* action) const override { return false; }

		virtual inline bool isKeyPressedConsume(int keyCode) override { return !EditorGui::isGuiFocused() && mKeyboardMouseInput->isKeyPressedConsume(keyCode); }
		virtual inline bool isMouseButtonPressedConsume(int button) override { return !EditorGui::isGuiFocused() && mKeyboardMouseInput->isMouseButtonPressed(button); }
		virtual inline bool isMouseScrollingUpConsume() override { return false; }
		virtual inline bool isMouseScrollingDownConsume() override { return false; }
		virtual inline bool isActionActiveANDConsume(const char* action) override { return false; }

		virtual void consumeAction(InputAction& action) override;

		void enableCameraInput();
		void disableCameraInput();
	};

	class EditorAppLogic : public IApplicationLogic {
	private:
		static constexpr std::array<const char*, 4> EEditorCameraStrings = {
			"NONE",
			"EDITOR_PERSPECTIVE",
			"EDITOR_ORTHOGRAPHIC",
			"INNER_APP_CHOSEN"
		};

		enum class EEditorCamera {
			NONE,

			EDITOR_PERSPECTIVE,
			EDITOR_ORTHOGRAPHIC,

			//The currently chosen camera for the Inner App
			INNER_APP_CHOSEN
		};

		struct EditorSettings {
			//ImGui window rendering controls
			bool RenderDebugWindow = true;

			//ImGui menu
			bool EditorCollapseMenuOpen = true;
			bool InnerAppCollapseMenu = true;
			bool RenderingCollapseMenuOpen = true;
			bool CameraCollapseMenuOpen = true;
			bool InputCollapseMenuOpen = true;
			EImGuiContainerType EngineSysContainerType = EImGuiContainerType::TAB;

			bool InnerAppEditorWindowDisplay = true;

			bool RenderPerformancePlotLines = false;
			const std::array<ETimeUnit, 2> RenderingTimeUnitsAvailable = {
				ETimeUnit::SECOND,
				ETimeUnit::MILLISECOND
			};
			ETimeUnit RenderingTimeUnit = ETimeUnit::MILLISECOND;

			//-----Editor Camera(s)-----
			//Current camera in which the scene/UI is to be viewed
			EEditorCamera mCurrentCamera = EEditorCamera::EDITOR_ORTHOGRAPHIC;

			//-----Close App Functionality-----
			const float QuitHoldTimeRequired = 1.5f;
			float QuitButtonHoldTime = 0.0f;
		};

		std::shared_ptr<IApplicationLogic> mInnerAppLogic;

		std::shared_ptr<EditorInputLayer> mEditorInputLayer;
		//std::optional<std::shared_ptr<AInputLayer>> mInnerAppInputLayer;

		//Editor Cameras
		std::unique_ptr<DOH::OrthographicCamera> mOrthoCamera;
		std::unique_ptr<DOH::PerspectiveCamera> mPerspectiveCamera;
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
