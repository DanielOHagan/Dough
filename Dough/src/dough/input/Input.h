#pragma once

#include "dough/input/AInputLayer.h"
#include "dough/rendering/Config.h"

namespace DOH {

	class Input {

		friend class Application;

	private:
		static std::unique_ptr<Input> INSTANCE;

		//TODO:: Make this into a queue or something in-which there is a clearly defined priority for event handling
		//	(e.g. inputLayer[0] has the highest priority and inputLayer[inputLayer.size() - 1] has the least.
		std::vector<std::shared_ptr<AInputLayer>> mInputLayers;
		float mMousePosX;
		float mMousePosY;

		Input(const Input& copy) = delete;
		Input operator=(const Input& assignment) = delete;

		//Pressed Event includes EEventType::MOUSE_BUTTON_DOWN & EEventType::MOUSE_BUTTON_UP, the difference passed as the pressed param
		void onKeyPressedEvent(int keyCode, bool pressed);
		//Pressed Event includes EEventType::MOUSE_BUTTON_DOWN & EEventType::MOUSE_BUTTON_UP, the difference passed as the pressed param
		void onMouseButtonPressedEvent(int button, bool pressed);
		void onMouseMoveEvent(float x, float y);
		void onMouseScrollEvent(float offsetX, float offsetY);
		void resetCycleData();

		static void init();
		static void close();
		inline static Input& get() { return *INSTANCE; }

	public:
		static const std::array<int, 120> ALL_KEY_CODES;
		static const std::array<int, 53> DEFAULT_KEY_CODES;
		static const std::array<int, 8> DEFAULT_MOUSE_BUTTON_CODES;

		Input();

		inline bool hasInput() const { return mInputLayers.size() > 0; }

		static void addInputLayer(std::shared_ptr<AInputLayer> inputLayer);
		static void removeInputLayer(const char* name);
		static std::vector<std::shared_ptr<AInputLayer>>& getInputLayers();
		static std::optional<std::reference_wrapper<AInputLayer>> getInputLayer(const char* name);
		static std::optional<std::shared_ptr<AInputLayer>> getInputLayerPtr(const char* name);
		inline static float getMousePosX() { return INSTANCE->mMousePosX; }
		inline static float getMousePosY() { return INSTANCE->mMousePosY; }

		static const char* getInputString(int keyCode);
		static const char* getInputStringShortHand(int keyCode);
	};
}
