#pragma once

#include "dough/Maths.h"
#include "dough/Core.h"
#include "dough/rendering/textures/TextureVulkan.h"
#include "dough/ImGuiWrapper.h"
#include "dough/scene/geometry/primitives/Quad.h"
#include "dough/files/JsonFileData.h"

#include "editor/ui/EditorResourceUiViewer.h"

namespace DOH::EDITOR {

	class EditorGui {

	private:
		static std::unique_ptr<EditorGui> INSTANCE;

		EditorGui(const EditorGui& copy) = delete;
		EditorGui operator=(const EditorGui& assignment) = delete;

		//Map for texture windows, using the texture's ID as the key
		std::unordered_map<std::string, ResourceViewerUiTextureArray> mTextureArrayViewerWindows;

		//TODO:: UUID system for key. Currently only textures are used so using texture.ID is fine but adding models or sound will require a UUID
		std::unordered_map<uint32_t, std::shared_ptr<AResourceViewerUi>> mResourceViewerMap;

	private:
		//-----ImGui Implementations-----
		void imGuiDrawTextureViewerWindowsImpl();
		void imGuiRemoveHiddenTextureViewerWindowsImpl();

		void imGuiDisplayHelpTooltipImpl(const char* message);
		void imGuiBulletTextWrappedImpl(const char* message);

		void imGuiPrintMat4x4Impl(const glm::mat4x4& mat, const char* name);

		void imGuiPrintDrawCallTableColumnImpl(const char* pipelineName, const uint32_t drawCount, const char* renderPass);

		void imGuiSingleUnitTimeImpl(const uint64_t nanos, const bool attachSuffix);
		void imGuiSingleUnitTimeImpl(const long micros, const bool attachSuffix);
		void imGuiSingleUnitTimeImpl(const double millis, const bool attachSuffix);
		void imGuiSingleUnitTimeImpl(const float seconds, const bool attachSuffix);

		bool isGuiHandlingKeyboardInputImpl();
		bool isGuiHandlingMouseInputImpl();
		bool isGuiHandlingTextInputImpl();

		void imGuiControlsAGeometryImpl(AGeometry& geo, const char* name);
		void imGuiControlsQuadImpl(Quad& quad, const char* name);

		void imGuiInfoAGeometryImpl(AGeometry& geo, const char* name);
		void imGuiInfoQuadImpl(Quad& quad, const char* name);

		void imGuiJsonElementImpl(JsonElement& element, const char* name);
		void imGuiJsonObjectImpl(JsonElement& object, const char* name);
		void imGuiJsonArrayImpl(JsonElement& array, const char* name);

		//-----GUI Implementation Agnostic Functions-----
		void openTextureViewerWindowImpl(const TextureVulkan& texture);
		void openMonoSpaceTextureAtlasViewerWindowImpl(const MonoSpaceTextureAtlas& textureAtlas);
		void openIndexedTextureAtlasViewerWindowImpl(const IndexedTextureAtlas& textureAtlas);
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

		//-----Resource Viewers-----

		//TODO:: Rename TextureViewers to ResourceViewers.

		/**
		* Display information on the given resource, it stores the type of resource it references.
		*
		* @param uiViewer The resource viewer to display.
		* @param openAsWindow Open the resource viewer in a separate window.
		*/
		static void resourceViewer(AResourceViewerUi& uiViewer, bool openAsWindow = false);

		//-----Texture Viewing-----
		/**
		* Cycle through current TextureViewerWindows and draw those enabled for rendering
		*/
		static inline void drawTextureViewerWindows() { INSTANCE->imGuiDrawTextureViewerWindowsImpl(); }

		/**
		* If a texture viewer of given texture isn't already open then create a new texture viewer.
		* Then enable it for rendering.
		*
		* @param texture The desired texture for use in creating a new texture viewer window or enabling an already existing one.
		*/
		static inline void openTextureViewerWindow(const TextureVulkan& texture) { INSTANCE->openTextureViewerWindowImpl(texture); }
		/**
		* If a texture viewer of given mono space texture atlas isn't already open then create a new texture viewer.
		* Then enable it for rendering.
		*
		* @param textureAtlas The desired texture atlas for use in creating a new texture viewer window or enabling an already existing one.
		*/
		static inline void openMonoSpaceTextureAtlasViewerWindow(const MonoSpaceTextureAtlas& textureAtlas) { INSTANCE->openMonoSpaceTextureAtlasViewerWindowImpl(textureAtlas); }
		/**
		* If a texture viewer of given indexed texture atlas isn't already open then create a new texture viewer.
		* Then enable it for rendering.
		*
		* @param textureAtlas The desired texture atlas for use in creating a new texture viewer window or enabling an already existing one.
		*/
		static inline void openIndexedTextureAtlasViewerWindow(const IndexedTextureAtlas& textureAtlas) { INSTANCE->openIndexedTextureAtlasViewerWindowImpl(textureAtlas); }
		/**
		* If a texture viewer of given texture array isn't already open then create a new texture viewer.
		* Then enable it for rendering.
		*
		* @param title The title of the texture array, to be used as a UUID for further reference/use.
		* @param texArray The desired texture array for use in creating a new texture viewer window.
		*/
		static inline void openTextureArrayViewerWindow(const char* title, const TextureArray& texArray) { INSTANCE->openTextureArrayViewerWindowImpl(title, texArray); }

		/**
		* Find texture viewer of given textureId and remove it from the texture viewer map.
		*
		* @param textureId Unique id of the texture viewer that is to be closed.
		*/
		static inline void closeTextureViewerWindow(const uint32_t textureId) { INSTANCE->closeTextureViewerWindowImpl(textureId); }
		/**
		* Find texture atlas viewer of given textureId and remove it from the texture atlas viewer map.
		*
		* @param textureId Unique id of the texture atlas viewer that is to be closed.
		*/
		static inline void closeMonoSpaceTextureAtlasViewerWindow(const uint32_t textureId) { INSTANCE->closeMonoSpaceTextureAtlasViewerWindowImpl(textureId); }
		/**
		* Find texture array viewer of given title and remove it from the texture array viewer map.
		*
		* @param title Title of the texture array viewer that is to be closed.
		*/
		static inline void closeTextureArrayViewerWindow(const char* title) { INSTANCE->closeTextureArrayViewerWindowImpl(title); }
		/**
		* Cycle through all texture viewers and remove all that are currently hidden (rendering is disabled).
		*/
		static inline void removeHiddenTextureViewerWindows() { INSTANCE->imGuiRemoveHiddenTextureViewerWindowsImpl(); }

		/**
		* Search currently instantiated texture viewer windows to see if given textureId matches an instantiated one.
		*
		* @param textureId Unique id of the texture being used by the viewer window.
		* @returns If a texture viewer window with textureId is instantiated.
		*/
		static inline bool hasTextureViewerWindow(const uint32_t textureId) { return INSTANCE->hasTextureViewerWindowImpl(textureId); }
		/**
		* Search currently instantiated mono space texture viewer windows to see if a given textureId matches an
		* instantiated one.
		*
		* @param textureId Unique id of the texture atlas being used by the viewer window.
		* @returns If a texture atlas viewer window with textureId is instantiated.
		*/
		static inline bool hasMonoSpaceTextureAtlasViewerWindow(const uint32_t textureId) { return INSTANCE->hasMonoSpaceTextureAtlasViewerWindowImpl(textureId); }
		/**
		* Search currently instantiated texture array viewer widnows to see if a given title matches an instantiated one.
		*
		* @param title The title of the texture array that is being used by the viewer window.
		* @return If a texture array viewer window with title is instantiated.
		*/
		static inline bool hasTextureArrayViewerWindow(const char* title) { return INSTANCE->hasTextureArrayViewerWindowImpl(title); }

		//-----GUI Text-----
		/**
		* Display a hoverable tooltip that displays the given message.
		* Tooltip icon: (?)
		*
		* @param message The given message that is to be displayed when hovering over the tooltip icon.
		*/
		static inline void displayHelpTooltip(const char* message) { INSTANCE->imGuiDisplayHelpTooltipImpl(message); }
		/**
		* Display a bullet pointed wrapped text of given message.
		*
		* @param message The given message tat is to be displayed starting with a bullet point and wraps depending on container dimensions.
		*/
		static inline void bulletTextWrapped(const char* message) { INSTANCE->imGuiBulletTextWrappedImpl(message); }

		//-----Time-----
		/**
		* Display time as a single unit as text, possibly with a shorthand suffix attached.
		*
		* @param nanos The number of nanoseconds to display.
		* @param attachSuffix Whether the shorthand suffix should be attached to the text after the end of the number.
		*/
		static inline void singleUnitTime(const uint64_t nanos, const bool attachSuffix = false) { INSTANCE->imGuiSingleUnitTimeImpl(nanos, attachSuffix); }
		/**
		* Display time as a single unit as text, possibly with a shorthand suffix attached.
		*
		* @param micros The number of microseconds to display.
		* @param attachSuffix Whether the shorthand suffix should be attached to the text after the end of the number.
		*/
		static inline void singleUnitTime(const long micros, const bool attachSuffix = false) { INSTANCE->imGuiSingleUnitTimeImpl(micros, attachSuffix); }
		/**
		* Display time as a single unit as text, possibly with a shorthand suffix attached.
		*
		* @param millis The number of milliseconds to display.
		* @param attachSuffix Whether the shorthand suffix should be attached to the text after the end of the number.
		*/
		static inline void singleUnitTime(const double millis, const bool attachSuffix = false) { INSTANCE->imGuiSingleUnitTimeImpl(millis, attachSuffix); }
		/**
		* Display time as a single unit as text, possibly with a shorthand suffix attached.
		*
		* @param seconds The number of seconds to display.
		* @param attachSuffix Whether the shorthand suffix should be attached to the text after the end of the number.
		*/
		static inline void singleUnitTime(const float seconds, const bool attachSuffix = false) { INSTANCE->imGuiSingleUnitTimeImpl(seconds, attachSuffix); }

		//-----Maths-----
		/**
		* Display a table of the current values of a given Matrix4x4.
		*
		* @param mat The Matrix to be displayed.
		* @param name The name/label of the Matrix.
		*/
		static inline void printMat4x4(const glm::mat4x4& mat, const char* name) { INSTANCE->imGuiPrintMat4x4Impl(mat, name); }


		//-----Input Handling-----
		/**
		* Whether the Editor GUI is currently handling any keyboard events for this frame.
		*
		* @returns If the GUI is handling any keyboard event.
		*/
		static inline bool isGuiHandlingKeyboardInput() { return INSTANCE->isGuiHandlingKeyboardInputImpl(); }
		/**
		* Whether the Editor GUI is currently handling any mouse events for this frame.
		*
		* @returns If the GUI is handling any mouse event.
		*/
		static inline bool isGuiHandlingMouseInput() { return INSTANCE->isGuiHandlingMouseInputImpl(); }
		/**
		* Whether the Editor GUI is currently handling any text events for this frame.
		*
		* @returns If the GUI is handling any text event.
		*/
		static inline bool isGuiHandlingTextInput() { return INSTANCE->isGuiHandlingTextInputImpl(); }

		//-----Primitive Controls-----
		/**
		* Display controls for the Primitive Base Class: AGeometry
		* 
		* @param geo The geometry instance to control.
		* @param name Unique name for geometry to display.
		*/
		static inline void controlsAGeometry(AGeometry& geo, const char* name) { INSTANCE->imGuiControlsAGeometryImpl(geo, name); }
		/**
		* Display controls for the primitive: Quad
		* 
		* @param Quad The quad to display a set of controls (a.k.a editable properties) for.
		* @param name Unique name for Quad to display.
		*/
		static inline void controlsQuad(Quad& quad, const char* name) { INSTANCE->imGuiControlsQuadImpl(quad, name); }

		//-----Primitive Information-----
		/**
		* Display information for the Primitive Base Class: AGeometry
		* 
		* @param geo The geometry to display.
		* @param name Unique name for geometry to display.
		*/
		static inline void infoAGeometry(AGeometry& geo, const char* name) { INSTANCE->imGuiInfoAGeometryImpl(geo, name); }
		/**
		* Display information for the primitive: Quad
		* 
		* @param quad The Quad to display.
		* @param name Unique name for Quad to display.
		*/
		static inline void infoQuad(Quad& quad, const char* name) { INSTANCE->imGuiInfoQuadImpl(quad, name); }
		static inline void infoQuad(const Quad& quad, const char* name) { infoQuad((Quad&) quad, name); }

		//-----File Data Viewers-----
		/**
		* Display a JSON element. The type of the element is used to determine how it is displayed.
		* 
		* @param element The JSON element that is to be displayed.
		* @param name Unique name for JsonElement to display.
		*/
		static inline void jsonElement(JsonElement& element, const char* name) { INSTANCE->imGuiJsonElementImpl(element, name); }
		/** 
		* Display a JSON object, including its children recursively.
		* 
		* @param object JSON object to be displayed, whether it has children elements or not.
		* @param name Unique name for JsonObject to display.
		*/
		static inline void jsonObject(JsonElement& object, const char* name) { INSTANCE->imGuiJsonObjectImpl(object, name); }
		/**
		* Display an array of JSON elements.
		* 
		* @param array JSON array to be displayed, whether it has elements or not.
		* @param name Unique name for JsonArray to display.
		*/
		static inline void jsonArray(JsonElement& array, const char* name) { INSTANCE->imGuiJsonArrayImpl(array, name); }

		//IMPORTANT:: Calling this function outside of the table start/end is undefined behaviour.
		//TODO:: Some kind of way of creating custom tables easily in the GUI
		// It might be worth just keeping this function, in its current form, in EditorAppLogic
		static inline void printDrawCallTableColumn(const char* pipelineName, const uint32_t drawCount, const char* renderPass) { INSTANCE->imGuiPrintDrawCallTableColumnImpl(pipelineName, drawCount, renderPass); }
	};
}
