#pragma once

#include "dough/Maths.h"
#include "dough/Core.h"
#include "dough/rendering/textures/TextureVulkan.h"
#include "dough/ImGuiWrapper.h"

#include "editor/ui/EditorResourceUiViewer.h"

namespace DOH::EDITOR {

	class EditorGui {

	private:
		static std::unique_ptr<EditorGui> INSTANCE;

		EditorGui(const EditorGui& copy) = delete;
		EditorGui operator=(const EditorGui& assignment) = delete;

		//Map for texture windows, using the texture's ID as the key
		std::unordered_map<uint32_t, ResourceViewerUiTexture> mTextureViewerWindows;
		std::unordered_map<uint32_t, ResourceViewerUiMonoSpaceTextureAtlas> mMonoSpaceTextureAtlasViewerWindows;
		std::unordered_map<std::string, ResourceViewerUiTextureArray> mTextureArrayViewerWindows;

	private:
		//-----ImGui Implementations-----
		void imGuiDrawTextureViewerWindowsImpl();
		void imGuiRemoveHiddenTextureViewerWindowsImpl();

		void imGuiDisplayHelpTooltipImpl(const char* message);
		void imGuiBulletTextWrappedImpl(const char* message);

		void imGuiPrintMat4x4Impl(const glm::mat4x4& mat, const char* name);
		//inline void imGuiPrintMat4x4Impl(const glm::mat4x4& mat, const std::string& name) { imGuiPrintMat4x4(mat, name.c_str()); }

		void imGuiPrintDrawCallTableColumnImpl(const char* pipelineName, const uint32_t drawCount, const char* renderPass);


		//-----GUI Implementation Agnostic Functions-----
		void openTextureViewerWindowImpl(const TextureVulkan& texture);
		void openMonoSpaceTextureAtlasViewerWindowImpl(const MonoSpaceTextureAtlas& textureAtlas);
		void openTextureArrayViewerWindowImpl(const char* title, const TextureArray& texArray);

		void closeTextureViewerWindowImpl(const uint32_t textureId);
		void closeMonoSpaceTextureAtlasViewerWindowImpl(const uint32_t textureId);
		void closeTextureArrayViewerWindowImpl(const char* title);
		
		bool hasTextureViewerWindowImpl(const uint32_t textureId);
		bool hasMonoSpaceTextureAtlasViewerWindowImpl(const uint32_t textureId);
		bool hasTextureArrayViewerWindowImpl(const char* title);

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
		static void openTextureViewerWindow(const TextureVulkan& texture);
		/**
		* If a texture viewer of given mono space texture atlas isn't already open then create a new texture viewer.
		* Then enable it for rendering.
		*
		* @param textureAtlas The desired texture atlas for use in creating a new texture viewer window.
		*/
		static void openMonoSpaceTextureAtlasViewerWindow(const MonoSpaceTextureAtlas& textureAtlas);
		/**
		* If a texture viewer of given texture array isn't already open then create a new texture viewer.
		* Then enable it for rendering.
		*
		* @param title The title of the texture array, to be used as a UUID for further reference/use.
		* @param texArray The desired texture array for use in creating a new texture viewer window.
		*/
		static void openTextureArrayViewerWindow(const char* title, const TextureArray& texArray);

		/**
		* Find texture viewer of given textureId and remove it from the texture viewer map.
		* 
		* @param textureId Unique id of the texture viewer that is to be closed.
		*/
		static void closeTextureViewerWindow(const uint32_t textureId);
		/**
		* Find texture atlas viewer of given textureId and remove it from the texture atlas viewer map.
		*
		* @param textureId Unique id of the texture atlas viewer that is to be closed.
		*/
		static void closeMonoSpaceTextureAtlasViewerWindow(const uint32_t textureId);
		/**
		* Find texture array viewer of given title and remove it from the texture array viewer map.
		*
		* @param title Title of the texture array viewer that is to be closed.
		*/
		static void closeTextureArrayViewerWindow(const char* title);
		/**
		* Cycle through all texture viewers and remove all that are currently hidden (rendering is disabled).
		*/
		static void removeHiddenTextureViewerWindows();

		/**
		* Search currently instantiated texture viewer windows to see if given textureId matches an instantiated one.
		*
		* @param textureId Unique id of the texture being used by the viewer window.
		* @returns If a texture viewer window with textureId is instantiated.
		*/
		static bool hasTextureViewerWindow(const uint32_t textureId);
		/**
		* Search currently instantiated mono space texture viewer windows to see if a given textureId matches an
		* instantiated one.
		* 
		* @param textureId Unique id of the texture atlas being used by the viewer window.
		* @returns If a texture atlas viewer window with textureId is instantiated.
		*/
		static bool hasMonoSpaceTextureAtlasViewerWindow(const uint32_t textureId);
		/**
		* Search currently instantiated texture array viewer widnows to see if a given title matches an instantiated one.
		* 
		* @param title The title of the texture array that is being used by the viewer window.
		* @return If a texture array viewer window with title is instantiated.
		*/
		static bool hasTextureArrayViewerWindow(const char* title);

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
		static void printDrawCallTableColumn(const char* pipelineName, const uint32_t drawCount, const char* renderPass);
	};
}
