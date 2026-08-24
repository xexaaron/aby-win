#include <aby-win/window.hpp>

namespace aby::win {

	auto on_window_resize(WindowResizedEvent& event) -> bool {
		log_inf("FREE FN EVENT: [name: {}, category: {}, type: {}]", event.name(), event.category(), event.type());
		return false;
	}

	class EventManager {
	public:
		auto on_window_resize(WindowResizedEvent& event) -> bool {
			log_inf("MEMBER FN EVENT: [name: {}, category: {}, type: {}]", event.name(), event.category(), event.type());
			return false;
		}
	};

	auto on_window_resize_lambda = [](WindowResizedEvent& event) -> bool {
		log_inf("LAMBDA FN EVENT: [name: {}, category: {}, type: {}]", event.name(), event.category(), event.type());
		return false;
	};

} // namespace aby::win

int main(int argc, char** argv) {
	using namespace aby::win;
	auto window = Window::create(EWindow::sdl, "aby-win-test", 800, 600, ERenderBackend::none);
	EventManager ev_manager;

	window->add_listener([&ev_manager](Event& event) -> bool {
		EventDispatcher ev(event);
		ev.dispatch(on_window_resize);
		ev.dispatch(&EventManager::on_window_resize, &ev_manager);
		ev.dispatch(on_window_resize_lambda);
		return false;
	});

	while (!window->should_close()) {
		window->poll();
	}

	return 0;
}
