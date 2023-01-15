#pragma once

#include "dough/Maths.h"
#include "dough/Core.h"
#include "dough/rendering/textures/TextureVulkan.h"
#include "dough/ImGuiWrapper.h"

namespace DOH::EDITOR {

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

	class EditorGui {

	private:
		static std::unique_ptr<EditorGui> INSTANCE;

		EditorGui(const EditorGui& copy) = delete;
		EditorGui operator=(const EditorGui& assignment) = delete;

		//Map for texture windows, using the texture's ID as the key
		std::unordered_map<uint32_t, ImGuiTextureViewerWindow> mTextureViewerWindows;

	private:
		//-----ImGui Implementations-----
		void imGuiDrawTextureViewerWindowsImpl();
		void imGuiDrawTextureViewerWindowImpl(ImGuiTextureViewerWindow& textureWindow);
		void imGuiRemoveHiddenTextureViewerWindowsImpl();

		void imGuiDisplayHelpTooltipImpl(const char* message);
		void imGuiBulletTextWrappedImpl(const char* message);

		void imGuiPrintMat4x4Impl(const glm::mat4x4& mat, const char* name);
		//inline void imGuiPrintMat4x4Impl(const glm::mat4x4& mat, const std::string& name) { imGuiPrintMat4x4(mat, name.c_str()); }

		void imGuiPrintDrawCallTableColumnImpl(const char* pipelineName, uint32_t drawCount);


		//-----GUI Implementation Agnostic Functions-----
		void openTextureViewerWindowImpl(TextureVulkan& texture);
		void closeTextureViewerWindowImpl(uint32_t textureId);
		bool hasTextureViewerWindowImpl(uint32_t textureId);

	public:
		EditorGui() = default;
		~EditorGui() = default;

		//TODO:: more editor features

		/**
		* Initialise the data members for this singleton class.
		*/
		static void init();
		/**
		* Close the data members for this singleton class.
		*/
		static void close();

		/**
		* Convenience function to get a reference to the ImGuiWrapper.
		* 
		* @returns ImGuiWrapper that is stored in RenderingContextVulkan.
		*/
		static ImGuiWrapper& getGuiWrapper();

		//-----Texture Viewing-----
		/**
		* Cycle through current TextureViewerWindows and draw those enabled for rendering
		*/
		static void drawTextureViewerWindows();
		/**
		* If a texture viewer of given texture isn't already open then create a new texture viewer.
		* Then enable it for rendering.
		* 
		* @param texture The desired texture for use in creating a new texture viewer window.
		*/
		static void openTextureViewerWindow(TextureVulkan& texture);
		/**
		* Find texture viewer of given textureId and remove it from the texture viewer map.
		* 
		* @param textureId Unique id of the texture viewer that is to be closed.
		*/
		static void closeTextureViewerWindow(uint32_t textureId);
		/**
		* Cycle through all texture viewers and remove all that are currently hidden (rendering is disabled).
		*/
		static void removeHiddenTextureViewerWindows();
		/**
		* Search currently instantiated texture viewer windows to see if given textureId matches an instantiated one.
		* 
		* @param textureId Unique id of the texture viewer that is to be queried.
		* @returns If a texture viewer window with textureId is instantiated.
		*/
		static bool hasTextureViewerWindow(uint32_t textureId);

		//-----GUI Text-----
		/**
		* Display a hoverable tooltip that displays the given message.
		* Tooltip icon: (?)
		* 
		* @param message The given message that is to be displayed when hovering over the tooltip icon.
		*/
		static void displayHelpTooltip(const char* message);
		/**
		* Display a bullet pointed wrapped text of given message.
		* 
		* @param message The given message tat is to be displayed starting with a bullet point and wraps depending on container dimensions.
		*/
		static void bulletTextWrapped(const char* message);

		//-----Maths-----
		/**
		* Display a table of the current values of a given Matrix4x4.
		* 
		* @param mat The Matrix to be displayed.
		* @param name The name/label of the Matrix.
		*/
		static void printMat4x4(const glm::mat4x4& mat, const char* name);

		
		
		//IMPORTANT:: Calling this function outside of the table start/end is undefined behaviour.
		//TODO:: Some kind of way of creating custom tables easily in the GUI
		// It might be worth just keeping this function, in its current form, in EditorAppLogic
		static void printDrawCallTableColumn(const char* pipelineName, uint32_t drawCount);
	};
}
