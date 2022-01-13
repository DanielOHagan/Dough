#include "dough/application/Application.h"

namespace DOH {

	Application* Application::INSTANCE = nullptr;

	Application::Application()
	:	mRunning(false)
	{}

	void Application::run(std::shared_ptr<IApplicationLogic> appLogic) {
		mAppLogic = appLogic;
		init();

		mainLoop();
	}

	void Application::init() {
		mWindow = std::make_unique<Window>(1920, 1080);
		mWindow->init();

		Input::init();

		mRenderer = std::make_unique<RendererVulkan>();
		mRenderer->init(mWindow->getWidth(), mWindow->getHeight());

		mAppLogic->init((float) mWindow->getWidth() / mWindow->getHeight());
	}

	void Application::mainLoop() {
		mRunning = true;

		while (mRunning) {
			mWindow->pollEvents();
			update();
			render();

			Input::get().resetCycleData();
		}
	}

	void Application::update() {
		mAppLogic->update(0.0f);
	}

	void Application::render() {
		mRenderer->drawFrame();
	}

	void Application::close() {
		mWindow->close();
		mAppLogic->close();
		mRenderer->close();

		Input::close();

		delete INSTANCE;
	}

	int Application::start(std::shared_ptr<IApplicationLogic> appLogic) {
		INSTANCE = new Application();
		
		if (Application::isInstantiated()) {
			try {
				INSTANCE->run(appLogic);
				Application::get().close();
				return EXIT_SUCCESS;
			} catch (const std::exception& e) {
				Application::get().close();
				std::cerr << e.what() << std::endl;
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
				//TODO:: mAppLoop->setFocused(focusChangeEvent.isFocused());
				return;
			case EEventType::WINDOW_RESIZE:
				WindowResizeEvent& e = (WindowResizeEvent&) windowEvent;
				mRenderer->resizeSwapChain(
					e.getWidth(),
					e.getHeight()
				);
				mAppLogic->onResize((float) e.getWidth() / e.getHeight());
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
				THROW("Unknown key event type");
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
			THROW("Unknown mouse event type");
			return;
		}
	}
}
