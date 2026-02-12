#include "dough/application/Application.h"

#include "dough/Logging.h"
#include "dough/files/ResourceHandler.h"

#include <tracy/public/tracy/Tracy.hpp>

namespace DOH {

	Application* Application::INSTANCE = nullptr;

	Application::Application()
	:	mRunning(false),
		mFocussed(true),
		mIconified(false)
	{}

	void Application::run() {
		ZoneScoped;

		mRunning = true;

		mAppInfoTimer->recordInterval("AppLoop.run() start");
		mAppLoop->run();
		mAppInfoTimer->recordInterval("AppLoop.run() end");
	}

	void Application::init(std::shared_ptr<IApplicationLogic> appLogic, const char* appInitSettingsFileName) {
		ZoneScoped;

		{
			uint32_t config = 0;

			#if defined (_DEBUG)
				config = 1;
			#elif defined(_TRACING)
				config = 2;
			#elif defined (_RELEASE)
				config = 3;
			#endif

			if (config == 1) {
				LOGLN_CYAN("Config: DEBUG");
			} else if (config == 2) {
				LOGLN_CYAN("Config: TRACING");
			} else if (config == 3) {
				LOGLN_CYAN("Config: RELEASE");
			} else {
				//This shouldn't happen. Check build config properties.
				LOG_ERR("Failed to determine build config");
			}
		}

		{ //App init settings
			std::shared_ptr<ApplicationInitSettings> initSettings = nullptr;
			if (ResourceHandler::doesFileExist(appInitSettingsFileName)) {
				initSettings = ResourceHandler::loadAppInitSettings(appInitSettingsFileName);
			}

			//Use default
			if (initSettings == nullptr) {
				mAppInitSettings = std::make_shared<ApplicationInitSettings>(Application::INIT_SETTINGS_DEFAULT_FILE_NAME);
			} else {
				mAppInitSettings = initSettings;
			}
		}

		StaticVertexInputLayout::initEngineDefaultVertexInputLayouts();

		mAppDebugInfo = std::make_unique<AppDebugInfo>();
		mAppInfoTimer = std::make_unique<IntervalTimer>(true);
		mAppInfoTimer->recordInterval("Application.init() start");
		mAppLogic = appLogic;

		mWindow = std::make_unique<Window>(mAppInitSettings->WindowWidth, mAppInitSettings->WindowHeight, mAppInitSettings->WindowDisplayMode);
		mAppInfoTimer->recordInterval("Window.init() start");
		mWindow->init(mAppInitSettings->ApplicationName);
		mAppInfoTimer->recordInterval("Window.init() end");

		mAppLoop = std::make_unique<ApplicationLoop>(
			*this,
			mAppInitSettings->TargetForegroundFps,
			mAppInitSettings->TargetForegroundUps,
			mAppInitSettings->RunInBackground,
			mAppInitSettings->TargetBackgroundFps,
			mAppInitSettings->TargetBackgroundUps
		);

		Input::init();

		mRenderer = std::make_unique<RendererVulkan>();
		mAppInfoTimer->recordInterval("Renderer.init() start");
		mRenderer->init(*mWindow);
		mAppInfoTimer->recordInterval("Renderer.init() end");

		mAppInfoTimer->recordInterval("AppLogic.init() start");
		mAppLogic->init(static_cast<float>(mWindow->getWidth()) / mWindow->getHeight());
		mAppInfoTimer->recordInterval("AppLogic.init() end");

		//TODO:: Some kind of post app.init preparation?
		//mRenderer->prepareCurrentRenderState();

		if (!mRenderer->isReady()) {
			LOG_ERR("Renderer is not ready, forcing stop");
			THROW("Thrown: Renderer not ready");
		}

		mAppInfoTimer->recordInterval("Applicaiton.init() end");
	}

	void Application::update(float delta) {
		ZoneScoped;

		const double preUpdate = Time::getCurrentTimeMillis();

		mAppLogic->update(delta);

		Input::get().resetCycleData();

		mAppDebugInfo->LastUpdateTimeMillis = Time::getCurrentTimeMillis() - preUpdate;
	}

	void Application::render(float delta) {
		ZoneScoped;

		const double preRender = Time::getCurrentTimeMillis();

		mAppLogic->render();

		mRenderer->getContext().getImGuiWrapper().newFrame();
		mAppLogic->imGuiRender(delta);

		//NOTE:: ImGui end frame is now called after it is rendered in RenderingContext::drawFrame(), this is needed for multi-viewport windows
		//mRenderer->getContext().getImGuiWrapper().endFrame();

		mRenderer->drawFrame();

		mAppDebugInfo->LastRenderTimeMillis = Time::getCurrentTimeMillis() - preRender;

		FrameMark;
	}

	void Application::close() {
		ZoneScoped;

		mAppInfoTimer->recordInterval("Closing start");
		mAppLogic->close();
		mWindow->close();
		mRenderer->close();

		Input::close();
		mAppInfoTimer->recordInterval("Closing end");

		mAppInfoTimer->end();
		mAppInfoTimer->dump("Application Info");
	}

	int Application::start(std::shared_ptr<IApplicationLogic> appLogic, const char* appInitSettingsFileName) {
		ZoneScoped;

		int returnCode = 0;

		if (!Application::isInstantiated()) {
			INSTANCE = new Application();

			#if defined (_DEBUG)
				INSTANCE->init(appLogic, appInitSettingsFileName);
				INSTANCE->run();
			#else
				try {
					INSTANCE->init(appLogic, appInitSettingsFileName);
					INSTANCE->run();
					returnCode = EXIT_SUCCESS;
				} catch (const std::exception& e) {
					LOG_RED("Fatal error: ");
					LOG_ERR(e.what());
					LOG_ENDL;
					returnCode = EXIT_FAILURE;
				}
			#endif

			INSTANCE->close();
			delete INSTANCE;
		} else {
			LOG_ERR("Attempted to start application that is already running.");
			returnCode = EXIT_FAILURE;
		}

		return returnCode;
	}

	void Application::onWindowEvent(WindowEvent& windowEvent) {
		ZoneScoped;

		switch (windowEvent.getType()) {
			case EEventType::WINDOW_CLOSE:
				stop();
				return;
			case EEventType::WINDOW_FOCUS_CHANGE: {
				bool focussed = (static_cast<WindowFocusChangeEvent&>(windowEvent)).isFocused();
				if (focussed != mFocussed) {

					//TODO:: This only half works. Clicking on an item works fine
					// but when clicking on an enpty part of the window it doesn't.
					if (
						ImGui::IsAnyItemActive() ||
						ImGui::IsAnyItemFocused() ||
						ImGui::IsAnyItemHovered() ||
						ImGui::IsAnyMouseDown()
					) {
						return;
					}
					mFocussed = focussed;
					mAppLoop->onFocusChange(mFocussed);

					//If fullscreen iconify window so it doesn't display frozen on monitor
					if (!mFocussed && mWindow->getDisplayMode() == EWindowDisplayMode::FULLSCREEN) {
						mWindow->iconify();
					}

					if (mFocussed) {
						LOGLN("Focus Change: " << TEXT_GREEN("FOCUSED"));
					} else {
						LOGLN("Focus Change: " << TEXT_RED("NOT FOCUSED"));
					}
				}
				return;
			}
			case EEventType::WINDOW_RESIZE:
			{
				WindowResizeEvent& e = static_cast<WindowResizeEvent&>(windowEvent);
				mRenderer->onResize(
					e.getWidth(),
					e.getHeight()
				);
				mAppLogic->onResize(static_cast<float>(e.getWidth()) / e.getHeight());
				LOGLN("Window Resized: " << e.getWidth() << "x" << e.getHeight());
				return;
			}
			case EEventType::WINDOW_ICONIFY_CHANGE:
				mIconified = (static_cast<WindowIconifyChangeEvent&>(windowEvent)).isIconified();
				if (mIconified) {
					mRenderer->deviceWaitIdle("Forcing GPU wait while iconified");
				}
				return;

			case EEventType::WINDOW_MONITOR_CHANGE:
			{
				WindowMonitorChangeEvent& e = static_cast<WindowMonitorChangeEvent&>(windowEvent);

				//Resize swap chain if fullscreen & size is different
				if (e.getDisplayMode() != EWindowDisplayMode::WINDOWED) {
					if (mWindow->getWidth() != e.getWidth() && mWindow->getHeight() != e.getHeight()) {
						WindowResizeEvent resizeEvent{e.getWindow(), e.getWidth(), e.getHeight()};
						onWindowEvent(resizeEvent);
					}
				}
				return;
			}

			case EEventType::WINDOW_DISPLAY_MODE_CHANGE:
			{
				WindowDisplayModeChangeEvent& e = static_cast<WindowDisplayModeChangeEvent&>(windowEvent);

				if (e.getDisplayMode() == EWindowDisplayMode::FULLSCREEN) {
					mRenderer->getContext().getImGuiWrapper().setEnabledConfigFlag(EImGuiConfigFlag::VIEWPORTS, false);
				} else {
					mRenderer->getContext().getImGuiWrapper().setEnabledConfigFlag(EImGuiConfigFlag::VIEWPORTS, true);
				}
				return;
			}

			default:
				LOG_WARN("Unknown window event type");
				return;
		}
	}

	void Application::onKeyEvent(KeyEvent& keyEvent) {
		ZoneScoped;

		switch (keyEvent.getType()) {
			case EEventType::KEY_DOWN:
				Input::get().onKeyPressedEvent(keyEvent.getKeyCode(), true);
				return;

			case EEventType::KEY_UP:
				Input::get().onKeyPressedEvent(keyEvent.getKeyCode(), false);
				return;

			default:
				LOG_WARN("Unknown key event type");
				return;
		}
	}

	void Application::onMouseEvent(MouseEvent& mouseEvent) {
		ZoneScoped;

		switch (mouseEvent.getType()) {
			case EEventType::MOUSE_MOVE:
			{
				MouseMoveEvent& move = static_cast<MouseMoveEvent&>(mouseEvent);
				Input::get().onMouseMoveEvent(move.getPosX(), move.getPosY());
				return;
			}

			case EEventType::MOUSE_BUTTON_DOWN:
				Input::get().onMouseButtonPressedEvent((static_cast<MouseButtonEvent&>(mouseEvent)).getButton(), true);
				return;

			case EEventType::MOUSE_BUTTON_UP:
				Input::get().onMouseButtonPressedEvent((static_cast<MouseButtonEvent&>(mouseEvent)).getButton(), false);
				return;
			
			case EEventType::MOUSE_SCROLL: {
				MouseScrollEvent& scroll = static_cast<MouseScrollEvent&>(mouseEvent);
				Input::get().onMouseScrollEvent(scroll.getOffsetX(), scroll.getOffsetY());
				return;
			}

		default:
			LOG_WARN("Unknown mouse event type");
			return;
		}
	}

	void Application::resetAppInitSettings() {
		std::string fileName = mAppInitSettings->FileName;
		mAppInitSettings = std::make_shared<ApplicationInitSettings>(fileName.c_str());
	}

	void Application::saveAppInitSettings(const char* fileName) {
		ResourceHandler::wrtieAppInitSettings(fileName, mAppInitSettings);
	}
}
