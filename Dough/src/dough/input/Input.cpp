#include "dough/input/Input.h"

#include "dough/input/InputCodes.h"
#include "dough/Logging.h"

namespace DOH {

	std::unique_ptr<Input> Input::INSTANCE = nullptr;

	const std::array<int, 53> Input::DEFAULT_KEY_CODES = {
		//Alphabet
		DOH_KEY_A, DOH_KEY_B, DOH_KEY_C, DOH_KEY_D, DOH_KEY_E, DOH_KEY_F,
		DOH_KEY_G, DOH_KEY_H, DOH_KEY_I, DOH_KEY_J, DOH_KEY_K, DOH_KEY_L,
		DOH_KEY_M, DOH_KEY_N, DOH_KEY_O, DOH_KEY_P, DOH_KEY_Q, DOH_KEY_R,
		DOH_KEY_S, DOH_KEY_T, DOH_KEY_U, DOH_KEY_V, DOH_KEY_W, DOH_KEY_X,
		DOH_KEY_Y, DOH_KEY_Z,

		//Numeric
		DOH_KEY_0, DOH_KEY_1, DOH_KEY_2, DOH_KEY_3, DOH_KEY_4, DOH_KEY_5,
		DOH_KEY_6, DOH_KEY_7, DOH_KEY_8, DOH_KEY_9,

		//Functions
		DOH_KEY_ESCAPE, DOH_KEY_SPACE, DOH_KEY_ENTER, DOH_KEY_LEFT_SHIFT,
		DOH_KEY_RIGHT, DOH_KEY_LEFT, DOH_KEY_UP, DOH_KEY_DOWN,
		DOH_KEY_F1, DOH_KEY_F2, DOH_KEY_F3, DOH_KEY_F4, DOH_KEY_F5

	};

	const std::array<int, 3> Input::DEFAULT_MOUSE_BUTTON_CODES = {
		DOH_MOUSE_BUTTON_LEFT, DOH_MOUSE_BUTTON_MIDDLE, DOH_MOUSE_BUTTON_RIGHT
	};

	void Input::init() {
		INSTANCE = std::make_unique<Input>();
	}

	void Input::close() {
		if (INSTANCE != nullptr) {
			INSTANCE->mInputLayers.clear();
			INSTANCE.reset();
			INSTANCE = nullptr;
		}
	}

	void Input::onKeyPressedEvent(int keyCode, bool pressed) {
		for (auto& inputLayer : mInputLayers) {
			if (inputLayer.second->handleKeyPressed(keyCode, pressed)) {
				return;
			}
		}
	}

	void Input::onMouseButtonPressedEvent(int button, bool pressed) {
		for (auto& inputLayer : mInputLayers) {
			if (inputLayer.second->handleMouseButtonPressed(button, pressed)) {
				return;
			}
		}
	}

	void Input::onMouseMoveEvent(float x, float y) {
		for (auto& inputLayer : mInputLayers) {
			if (inputLayer.second->handleMouseMoved(x, y)) {
				return;
			}
		}
	}

	void Input::onMouseScrollEvent(float offsetX, float offsetY) {
		for (auto& inputLayer : mInputLayers) {
			if (inputLayer.second->handleMouseScroll(offsetX, offsetY)) {
				return;
			}
		}
	}

	void Input::resetCycleData() {
		for (auto& inputLayer : mInputLayers) {
			inputLayer.second->resetCycleData();
		}
	}

	void Input::addInputLayer(const char* name, std::shared_ptr<AInputLayer> inputLayer) {
		const auto& result = INSTANCE->mInputLayers.emplace(name, inputLayer);
		if (!result.second) {
			LOG_ERR("Failed to add input layer: " << name);
		}
	}

	void Input::removeInputLayer(const char* name) {
		if (INSTANCE->mInputLayers.erase(name) == 0) {
			LOG_ERR("Failed to remove input layer: " << name);
		}
	}

	std::optional<std::reference_wrapper<AInputLayer>> Input::getInputLayer(const char* name) {
		const auto& itr = INSTANCE->mInputLayers.find(name);
		if (itr != INSTANCE->mInputLayers.end()) {
			return { *itr->second };
		} else {
			return {};
		}
	}
}
