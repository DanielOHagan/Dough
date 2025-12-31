#pragma once

#include "dough/Logging.h"
#include "dough/Core.h"

namespace DOH {

	class AInputLayer;

	constexpr static const int EDeviceInputTypeCount = 6;

	enum class EDeviceInputType {
		NONE = 0,			//Used to represnt no more action codes in InputAction.ActionCodesByDevice
		KEY_PRESS,
		MOUSE_PRESS,
		MOUSE_SCROLL_DOWN,
		MOUSE_SCROLL_UP,
		MOUSE_MOVE
		//,
		//CONTROLLER //a.k.a Gamepad
	};

	static std::array<const char*, EDeviceInputTypeCount> EDeviceInputType_Strings = {
		"NONE",
		"KEY_PRESS",
		"MOUSE_PRESS",
		"MOUSE_SCROLL_DOWN",
		"MOUSE_SCROLL_UP",
		"MOUSE_MOVE"
	};

	struct InputAction {
	public:
		constexpr static const char* JSON_KEYBOARD_AND_MOUSE_STRING = "kbm";
		constexpr static const char* JSON_CONTROLLER_STRING = "con";
		constexpr static const char* JSON_ACTION_TYPE_STRING = "t";
		constexpr static const char* JSON_ACTION_VALUE_STRING = "v";
		//Arbitrary maximum value. Can't see why this would need changing. Famous last words...
		constexpr static const uint32_t MAX_INPUTS_PER_ACTION = 3;

		//DeviceInputType (method of action e.g. key_press, mouse_press, etc...) with corresponding input code.
		std::array<std::pair<EDeviceInputType, int>, InputAction::MAX_INPUTS_PER_ACTION> ActionCodesByDevice;

		InputAction() {
			for (uint32_t i = 0; i < InputAction::MAX_INPUTS_PER_ACTION; i++) {
				ActionCodesByDevice[i] = { EDeviceInputType::NONE, 0 };
			}
		}

		InputAction(const InputAction& copy) {
			for (uint32_t i = 0; i < InputAction::MAX_INPUTS_PER_ACTION; i++) {
				ActionCodesByDevice[i] = copy.ActionCodesByDevice[i];
			}
		}

		InputAction operator=(const InputAction& assignment) {
			for (uint32_t i = 0; i < InputAction::MAX_INPUTS_PER_ACTION; i++) {
				ActionCodesByDevice[i] = assignment.ActionCodesByDevice[i];
			}
		}

		InputAction(std::initializer_list<std::pair<EDeviceInputType, int>> actionCodesByDevice) {
			uint32_t i = 0;
			for (auto& actionCode : actionCodesByDevice) {
				if (i == 3) {
					LOG_WARN("InputAction given above max action codes: " << actionCodesByDevice.size());
					break;
				}
				ActionCodesByDevice[i] = actionCode;
				i++;
			}

			//Fill remaining to NONE
			for (uint32_t remaining = i; remaining < InputAction::MAX_INPUTS_PER_ACTION; remaining++) {
				ActionCodesByDevice[remaining] = { EDeviceInputType::NONE, 0 };
			}
		}

		bool isActiveAND(const AInputLayer& inputLayer) const;
		bool isActiveANDConsume(AInputLayer& inputLayer);
		//bool isActiveOR() const;
		//bool isActiveORConsume(AInputLayer& inputLayer) const;
		//bool isActiveNOT() const;
		//bool isActiveNOTConsume(AInputLayer& inputLayer) const;

		static EDeviceInputType getEDeviceInputTypeFromString(const char* string);
	};

	class InputActionMap {
	private:
		std::unordered_map<std::string, InputAction> mActions;

	public:
		InputActionMap();
		InputActionMap(const char* filePath);
		
		//Read from a .json file. Adds actions to map if name not already taken.
		void addActionsFromFile(const char* filePath);

		bool addAction(const char* name, InputAction& action);
		void updateAction(const char* name, InputAction& action);
		void removeAction(const char* name);
		bool hasAction(const char* name);

		//AND - All steps in action must be true.
		bool isActionActiveAND(const char* name, const AInputLayer& inputLayer) const;
		//AND - All steps in action must be true. All steps are consumed if true.
		bool isActionActiveANDConsume(const char* name, AInputLayer& inputLayer);
		//TODO::
		//OR - Only one step in action must be true.
		//bool isActionActiveOR(const char* name, const AInputLayer& inputLayer);
		//OR - Only one step in action must be true. TODO:: Should all steps (if true) be consumed or just the first found?
		//bool isActionActiveORConsume(const char* name, const AInputLayer& inputLayer);

		inline std::unordered_map<std::string, InputAction>& getActions() { return mActions; }
	};
}
