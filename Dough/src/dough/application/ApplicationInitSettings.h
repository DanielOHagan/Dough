#pragma once

#include "dough/Utils.h"
#include "dough/Window.h"

namespace DOH {

	struct ApplicationInitSettings {
		//String labels
		static constexpr const char* APPLICATION_NAME_LABEL = "ApplicationName";
		static constexpr const char* WINDOW_WIDTH_LABEL = "WindowWidth";
		static constexpr const char* WINDOW_HEIGHT_LABEL = "WindowHeight";
		static constexpr const char* WINDOW_DISPLAY_MODE_LABEL = "WindowDisplayMode";
		static constexpr const char* TARGET_FOREGROUND_FPS_LABEL = "TargetForegroundFps";
		static constexpr const char* TARGET_FOREGROUND_UPS_LABEL = "TargetForegroundUps";
		static constexpr const char* RUN_IN_BACKGROUND_LABEL = "RunInBackground";
		static constexpr const char* TARGET_BACKGROUND_FPS_LABEL = "TargetBackgroundFps";
		static constexpr const char* TARGET_BACKGROUND_UPS_LABEL = "TargetBackgroundUps";

		//The name of this file
		std::string FileName;

		//Window
		std::string ApplicationName = "Dough Application";
		uint32_t WindowWidth = 1280;
		uint32_t WindowHeight = 720;
		EWindowDisplayMode WindowDisplayMode = EWindowDisplayMode::WINDOWED;

		//Application Loop
		float TargetForegroundFps = 60.0f;
		float TargetForegroundUps = 60.0f;
		bool RunInBackground = true;
		float TargetBackgroundFps = 15.0f;
		float TargetBackgroundUps = 15.0f;

		//TODO:: some kind of custom debug callback or dump

		ApplicationInitSettings(const char* fileName)
		:	FileName(fileName)
		{}

		void setWindowed(uint32_t width, uint32_t height) {
			WindowDisplayMode = EWindowDisplayMode::WINDOWED;
			WindowWidth = width;
			WindowHeight = height;
		}

		void setTargetFps(float foreground, float background = 15.0f) {
			TargetForegroundFps = foreground;
			TargetBackgroundFps = background;
		}

		void setTargetUps(float foreground, float background = 15.0f) {
			TargetForegroundUps = foreground;
			TargetBackgroundUps = background;
		}
	};
}
