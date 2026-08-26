#include "window.hpp"

#include "backend/glfw/glfw-window.hpp"
#include "backend/sdl/sdl-window.hpp"
#include "common.hpp"

namespace aby::win {

	auto Window::create(const Config& config) -> std::unique_ptr<Window> {
		return create_unique(config);
	}

	auto Window::create_raw(const Config& config) -> Window* {
		switch (config.window_backend) {
			case EWindow::glfw:
				return new glfw::Window(config);
			case EWindow::sdl:
				return new sdl::Window(config);
			default:
				aby_win_assert(false, "unimplemented window backend");
		}
		return nullptr;
	}

	auto Window::create_unique(const Config& config) -> std::unique_ptr<Window> {
		switch (config.window_backend) {
			case EWindow::glfw:
				return std::make_unique<glfw::Window>(config);
			case EWindow::sdl:
				return std::make_unique<sdl::Window>(config);
			default:
				aby_win_assert(false, "unimplemented window backend");
		}
		return nullptr;
	}

	auto Window::create_shared(const Config& config) -> std::shared_ptr<Window> {
		switch (config.window_backend) {
			case EWindow::glfw:
				return std::make_shared<glfw::Window>(config);
			case EWindow::sdl:
				return std::make_shared<sdl::Window>(config);
			default:
				aby_win_assert(false, "unimplemented window backend");
		}
		return nullptr;
	}

	Window::Window(const Config& config) :
	    m_Name(config.name),
	    m_RenderBackend(config.render_backend),
	    m_WindowBackend(config.window_backend),
	    m_Theme(config.theme) {
		if (!ILogger::get()) {
			ILogger::set<DefaultLogger>();
		}
	}

	auto Window::name() const -> std::string_view {
		return m_Name;
	}

	auto Window::theme() const -> ETheme {
		return m_Theme;
	}

} // namespace aby::win

namespace aby::win {

	auto Config::set_name(std::string_view name) -> Config& {
		this->name = name;
		return *this;
	}

	auto Config::set_width(uint32_t w) -> Config& {
		this->width = w;
		return *this;
	}

	auto Config::set_height(uint32_t h) -> Config& {
		this->height = h;
		return *this;
	}

	auto Config::set_size(uint32_t w, uint32_t h) -> Config& {
		this->width  = w;
		this->height = h;
		return *this;
	}

	auto Config::set_theme(ETheme theme) -> Config& {
		this->theme = theme;
		return *this;
	}

	auto Config::set_resizable(bool resizable) -> Config& {
		this->resizable = resizable;
		return *this;
	}

	auto Config::set_visible(bool visible) -> Config& {
		this->visible = visible;
		return *this;
	}

	auto Config::set_decorated(bool decorated) -> Config& {
		this->decorated = decorated;
		return *this;
	}

	auto Config::set_focused(bool focused) -> Config& {
		this->focused = focused;
		return *this;
	}

	auto Config::set_flags(bool resziable, bool visible, bool decorated, bool focused) -> Config& {
		this->resizable = resizable;
		this->visible   = visible;
		this->decorated = decorated;
		this->focused   = focused;
		return *this;
	}

	auto Config::set_window_backend(EWindow backend) -> Config& {
		this->window_backend = backend;
		return *this;
	}

	auto Config::set_render_backend(ERenderBackend backend) -> Config& {
		this->render_backend = backend;
		return *this;
	}

	auto Config::set_backends(EWindow window_backend, ERenderBackend render_backend) -> Config& {
		this->window_backend = window_backend;
		this->render_backend = render_backend;
		return *this;
	}

} // namespace aby::win
