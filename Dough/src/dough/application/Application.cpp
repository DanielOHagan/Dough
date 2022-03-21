#include "dough/application/Application.h"
#include "dough/Logging.h"

namespace DOH {

	Application* Application::INSTANCE = nullptr;

	Application::Application()
	:	mRunning(false),
		mFocused(true)
	{}

	void Application::run() {
		mRunning = true;

		mAppInfoTimer->recordInterval("AppLoop.run() start");
		mAppLoop->run();
		mAppInfoTimer->recordInterval("AppLoop.run() end");
	}

	void Application::init(std::shared_ptr<IApplicationLogic> appLogic) {
		mAppInfoTimer = std::make_unique<IntervalTimer>("Application Info", true);
		mAppInfoTimer->recordInterval("Application.init() start");
		mAppLogic = appLogic;

		mWindow = std::make_unique<Window>(1920, 1080);
		mAppInfoTimer->recordInterval("Window.init() start");
		mWindow->init();
		mAppInfoTimer->recordInterval("Window.init() start");

		mAppLoop = std::make_unique<ApplicationLoop>(*this);

		Input::init();

		mRenderer = std::make_unique<RendererVulkan>();
		mAppInfoTimer->recordInterval("Renderer.init() start");
		mRenderer->init(mWindow->getWidth(), mWindow->getHeight());
		mAppInfoTimer->recordInterval("Renderer.init() end");

		mAppInfoTimer->recordInterval("AppLogic.init() start");
		mAppLogic->init((float) mWindow->getWidth() / mWindow->getHeight());
		mAppInfoTimer->recordInterval("AppLogic.init() end");

		mRenderer->setupPostAppLogicInit();
		if (!mRenderer->isReady()) {
			LOG_ERR("Renderer is not ready, forcing stop");
			THROW("Thrown: Renderer not ready");
		}

		mAppInfoTimer->recordInterval("Applicaiton.init() end");
	}

	void Application::update(float delta) {
		mAppLogic->update(delta);
		Input::get().resetCycleData();
	}

	void Application::render() {
		mRenderer->drawFrame();
	}

	void Application::close() {
		mAppInfoTimer->recordInterval("Closing start");
		mWindow->close();
		mAppLogic->close();
		mRenderer->close();

		Input::close();
		mAppInfoTimer->recordInterval("Closing end");

		mAppInfoTimer->end();
		mAppInfoTimer->dump();

		delete INSTANCE;
	}

	int Application::start(std::shared_ptr<IApplicationLogic> appLogic) {
		INSTANCE = new Application();
		
		if (Application::isInstantiated()) {
			try {
				INSTANCE->init(appLogic);
				INSTANCE->run();
				INSTANCE->close();
				return EXIT_SUCCESS;
			} catch (const std::exception& e) {
				//TODO:: Fix situation where if exception thrown during close() then close() will have been called twice.
				INSTANCE->close();
				LOG_ERR(e.what());
				return EXIT_FAILURE;
			}
		} else {
			return EXIT_FAILURE;
		}
	}

	void Application::onWindowEvent(WindowEvent& windowEvent) {
		switch (windowEvent.getType()) {
			case EEventType::WINDOW_CLOSE:
				stop();
				return;
			case EEventType::WINDOW_FOCUS_CHANGE:
				mFocused = ((WindowFocusChangeEvent&) windowEvent).isFocused();
				mAppLoop->onFocusChange(mFocused);
				LOGLN("Focus Change: " << (mFocused ? "Focused" : "Not Focused"));
				return;
			case EEventType::WINDOW_RESIZE:
				WindowResizeEvent& e = (WindowResizeEvent&) windowEvent;
				mRenderer->onResize(
					e.getWidth(),
					e.getHeight()
				);
				mAppLogic->onResize((float) e.getWidth() / e.getHeight());
				LOGLN("Window Resized: " << e.getWidth() << "x" << e.getHeight());
				return;
		}
	}

	void Application::onKeyEvent(KeyEvent& keyEvent) {
		switch (keyEvent.getType()) {
			case EEventType::KEY_DOWN:
				Input::get().onKeyEvent(keyEvent.getKeyCode(), true);
				return;

			case EEventType::KEY_UP:
				Input::get().onKeyEvent(keyEvent.getKeyCode(), false);
				return;

			default:
				LOG_WARN("Unknown key event type");
				return;
		}
	}

	void Application::onMouseEvent(MouseEvent& mouseEvent) {
		switch (mouseEvent.getType()) {
			case EEventType::MOUSE_BUTTON_DOWN:
				Input::get().onMouseButtonEvent(((MouseButtonEvent&) mouseEvent).getButton(), true);
				return;

			case EEventType::MOUSE_BUTTON_UP:
				Input::get().onMouseButtonEvent(((MouseButtonEvent&) mouseEvent).getButton(), false);
				return;
			
			case EEventType::MOUSE_MOVE: {
				MouseMoveEvent& move = (MouseMoveEvent&) mouseEvent;
				Input::get().onMouseMove(move.getPosX(), move.getPosY());
				return;
			}
			
			case EEventType::MOUSE_SCROLL: {
				MouseScrollEvent& scroll = (MouseScrollEvent&) mouseEvent;
				Input::get().onMouseScroll(scroll.getOffsetX(), scroll.getOffsetY());
				return;
			}

		default:
			LOG_WARN("Unknown mouse event type");
			return;
		}
	}
}
