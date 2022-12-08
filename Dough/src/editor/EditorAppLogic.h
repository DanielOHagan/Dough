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

		struct ImGuiTextureViewerWindow {
			const TextureVulkan& Texture;
			bool Display;
			bool MatchWindowSize;
			float Scale;
		
			ImGuiTextureViewerWindow(
				TextureVulkan& texture,
				const bool display,
				const bool matchWindowSize,
				const float scale
			) : Texture(texture),
				Display(display),
				MatchWindowSize(matchWindowSize),
				Scale(scale)
			{}
		};

		struct EditorSettings {
			//Map for texture windows, using the texture's ID as the key
			std::unordered_map<uint32_t, ImGuiTextureViewerWindow> TextureViewerWindows;

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
		//Draw a window displaying a texture
		void imGuiDrawTextureViewerWindow(ImGuiTextureViewerWindow& textureWindow);
		//TODO:: more editor features
		//void imGuiDrawTextureAtlasViewerWindow(ImGuiTextureAtlasViewerWindow& textureAtlasWindow);
		void imGuiRemoveHiddenTextureViewerWindows();

		//ImGui convenience and separated functions, primarly used for debugging and easier ImGui functionality
		void imGuiDisplayHelpTooltip(const char* message);
		void imGuiBulletTextWrapped(const char* message);
		void imGuiRenderDebugWindow(float delta); //This renders the Editor windows
		void imGuiPrintDrawCallTableColumn(const char* pipelineName, uint32_t drawCount);
		void imGuiPrintMat4x4(const glm::mat4x4& mat, const char* name);
		inline void imGuiPrintMat4x4(const glm::mat4x4& mat, const std::string& name) { imGuiPrintMat4x4(mat, name.c_str()); }
	};
}
