#include <aby-win/common.hpp>
#include <aby-win/window.hpp>

std::unique_ptr<aby::win::Window> s_Window;
std::string_view s_MonitorName = "";

namespace aby::win {

	auto on_window_resize(WindowResizedEvent& event) -> bool {
		aby_win_log("FREE FN EVENT: [name: {}, category: {}, type: {}], [w: {}, h: {}]",
		            event.name(), event.category(), event.type(),
		            event->width, event->height);
		return false;
	}

	class EventManager {
	public:
		auto on_window_resize(WindowResizedEvent& event) -> bool {
			aby_win_log("MEMBER FN EVENT: [name: {}, category: {}, type: {}], [w: {}, h: {}]",
			            event.name(), event.category(), event.type(),
			            event->width, event->height);

			return false;
		}
	};

	auto on_window_resize_lambda = [](WindowResizedEvent& event) -> bool {
		aby_win_log("LAMBDA FN EVENT: [name: {}, category: {}, type: {}], [w: {}, h: {}]",
		            event.name(), event.category(), event.type(),
		            event->width, event->height);
		return false;
	};

	auto on_window_moved(WindowMovedEvent& event) -> bool {
		if (!s_Window->monitor()) return false;
 
		if (s_MonitorName != s_Window->monitor()->name()) {
			aby_win_log("monitor changed from: {} -> {}", s_MonitorName, s_Window->monitor()->name());
			s_MonitorName = s_Window->monitor()->name();
		}
		return false;
	}

} // namespace aby::win

int main(int argc, char** argv) {
	using namespace aby::win;

	Config cfg;
	cfg.set_window_backend(EWindow::glfw)
	    .set_name("aby-win-test")
	    .set_size(800, 600)
	    .set_render_backend(ERenderBackend::none);

	s_Window = Window::create(cfg);
	EventManager ev_manager;

	auto* monitor = s_Window->monitor();
	s_MonitorName = s_Window->name();

	s_Window->add_listener([&ev_manager](Event& event) -> bool {
		EventDispatcher ev(event);
		ev.dispatch(on_window_moved);
		return false;
	});

	while (!s_Window->should_close()) {
		s_Window->poll();
	}

	return 0;
}
