#include "window.hpp"

#include "backend/glfw/glfw-window.hpp"

namespace aby::win {

	auto Window::create(EWindow window_backend, std::string_view name, u32 w, u32 h, ERenderBackend render_backend, ETheme theme) -> std::unique_ptr<Window> {
		return create_unique(window_backend, name, w, h, render_backend, theme);
	}

	auto Window::create_raw(EWindow window_backend, std::string_view name, u32 w, u32 h, ERenderBackend render_backend, ETheme theme) -> Window* {
		switch (window_backend) {
			case EWindow::glfw:
				return new glfw::Window(name, w, h, render_backend, theme);
			case EWindow::sdl:
			default:
				expect(false, "unimplemented window backend");
		}
	}

	auto Window::create_unique(EWindow window_backend, std::string_view name, u32 w, u32 h, ERenderBackend render_backend, ETheme theme) -> std::unique_ptr<Window> {
		switch (window_backend) {
			case EWindow::glfw:
				return std::make_unique<glfw::Window>(name, w, h, render_backend, theme);
			case EWindow::sdl:
			default:
				expect(false, "unimplemented window backend");
		}
	}

	auto Window::create_shared(EWindow window_backend, std::string_view name, u32 w, u32 h, ERenderBackend render_backend, ETheme theme) -> std::shared_ptr<Window> {
		switch (window_backend) {
			case EWindow::glfw:
				return std::make_shared<glfw::Window>(name, w, h, render_backend, theme);
			case EWindow::sdl:
			default:
				expect(false, "unimplemented window backend");
		}
	}

	Window::Window(EWindow window_backend, std::string_view name, u32 w, u32 h, ERenderBackend render_backend, ETheme theme) :
	    m_Name(name),
	    m_RenderBackend(render_backend),
	    m_WindowBackend(window_backend),
	    m_Theme(theme) {
	}

	auto Window::name() const -> std::string_view {
		return m_Name;
	}

	auto Window::theme() const -> ETheme {
		return m_Theme;
	}

} // namespace aby::win
