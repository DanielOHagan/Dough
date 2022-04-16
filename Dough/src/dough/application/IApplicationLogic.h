#pragma once

namespace DOH {

	class IApplicationLogic {

	private:
		IApplicationLogic(const IApplicationLogic& copy) = delete;
		IApplicationLogic operator=(const IApplicationLogic& assignment) = delete;

	public:
		IApplicationLogic() = default;

		virtual void init(float aspectRatio) = 0;
		virtual void update(float delta) = 0;
		virtual void render() = 0;
		virtual void imGuiRender() = 0;
		virtual void close() = 0;

		virtual void onResize(float aspectRatio) = 0;
	};

}
