#include "backend/glfw/glfw-window.hpp"

#ifdef _WIN32
#	define GLFW_EXPOSE_NATIVE_WIN32
#	include <dwmapi.h>
#	pragma comment(lib, "dwmapi.lib")
#elif defined(__APPLE__)
#	define GLFW_EXPOSE_NATIVE_COCOA
#elif defined(__linux__)
#	define GLFW_EXPOSE_NATIVE_WAYLAND
#	define GLFW_EXPOSE_NATIVE_X11
#endif

#ifndef DWMWA_USE_IMMERSIVE_DARK_MODE
#	define DWMWA_USE_IMMERSIVE_DARK_MODE 20
#endif

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

namespace aby::win::glfw::detail {

	auto err_callback(int error_code, const char* description) -> void {
		log_err("[glfw] ({}): {}", error_code, description);
	}
#ifdef _WIN32
	auto system_dark_theme() -> bool {
		DWORD value = 1;
		DWORD size  = sizeof(value);

		constexpr auto key =
		    L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize";

		constexpr auto name = L"AppsUseLightTheme";

		if (RegGetValueW(
		        HKEY_CURRENT_USER,
		        key,
		        name,
		        RRF_RT_REG_DWORD,
		        nullptr,
		        &value,
		        &size) != ERROR_SUCCESS) {
			// Default to light if the setting cannot be queried.
			return false;
		}

		return value == 0;
	}
#endif

} // namespace aby::win::glfw::detail

namespace aby::win::glfw {

	Window::Window(std::string_view name, u32 w, u32 h, ERenderBackend backend, ETheme theme) :
	    win::Window(EWindow::glfw, name, w, h, backend, theme) {
		glfwSetErrorCallback(&detail::err_callback);

		if (!glfwInit()) {
			return;
		}

		if (backend == ERenderBackend::opengl) {
			glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
			glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		} else {
			glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		}

		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
		glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
		glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
		glfwWindowHint(GLFW_FOCUSED, GLFW_TRUE);
		if (m_GLFW = glfwCreateWindow(w, h, name.data(), NULL, NULL); !m_GLFW) {
			glfwTerminate();
			return;
		}

		set_theme(theme);
	}

	Window::~Window() {
		glfwDestroyWindow(m_GLFW);
		glfwTerminate();
		m_GLFW = nullptr;
	}

	auto Window::set_name(std::string_view name) -> void {
		m_Name = std::string(name);
		glfwSetWindowTitle(m_GLFW, m_Name.c_str());
	}

	auto Window::set_width(u32 w) -> void {
		glfwSetWindowSize(m_GLFW, w, height());
	}

	auto Window::set_height(u32 h) -> void {
		glfwSetWindowSize(m_GLFW, width(), h);
	}

	auto Window::set_size(u32 w, u32 h) -> void {
		glfwSetWindowSize(m_GLFW, w, h);
	}

	auto Window::set_cursor_mode(ECursorMode mode) -> void {
		switch (mode) {
			case ECursorMode::normal:
				glfwSetInputMode(m_GLFW, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
				break;
			case ECursorMode::hidden:
				glfwSetInputMode(m_GLFW, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
				break;
			case ECursorMode::disabled:
				glfwSetInputMode(m_GLFW, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
				break;
		}
	}

	auto Window::set_cursor_pos(float x, float y) -> void {
		glfwSetCursorPos(m_GLFW, x, y);
	}

	auto Window::set_theme(ETheme theme) -> void {
		m_Theme = theme;
#ifdef _WIN32
		BOOL dark = FALSE;
		switch (theme) {
			case ETheme::dark:
				dark = TRUE;
				break;
			case ETheme::light:
				dark = FALSE;
				break;
			case ETheme::automatic:
				dark = detail::system_dark_theme() ? TRUE : FALSE;
				break;
		}

		DwmSetWindowAttribute(
		    glfwGetWin32Window(m_GLFW),
		    DWMWA_USE_IMMERSIVE_DARK_MODE,
		    &dark,
		    sizeof(dark));
#else
		log_wrn("theme setting is not implemented on platforms other than win32 currently");
#endif
	}

	auto Window::focus() -> void {
		glfwFocusWindow(m_GLFW);
	}

	auto Window::minimize() -> void {
		glfwIconifyWindow(m_GLFW);
	}

	auto Window::maximize() -> void {
		glfwMaximizeWindow(m_GLFW);
	}

	auto Window::show() -> void {
		glfwShowWindow(m_GLFW);
	}

	auto Window::close() -> void {
		glfwSetWindowShouldClose(m_GLFW, GLFW_TRUE);
	}

	auto Window::poll() -> void {
		glfwPollEvents();
	}

	auto Window::width() const -> u32 {
		int w, h;
		glfwGetWindowSize(m_GLFW, &w, &h);
		return w;
	}

	auto Window::height() const -> u32 {
		int w, h;
		glfwGetWindowSize(m_GLFW, &w, &h);
		return h;
	}

	auto Window::size() const -> std::pair<u32, u32> {
		int w, h;
		glfwGetWindowSize(m_GLFW, &w, &h);
		return std::make_pair<u32, u32>(w, h);
	}

	auto Window::scale() const -> float {
		float x, y;
		glfwGetWindowContentScale(m_GLFW, &x, &y);
		return x;
	}

	auto Window::native() const -> NativeWindow {
		NativeWindow out;
		out.backend_window = m_GLFW;
#ifdef _WIN32
		out.platform_window = glfwGetWin32Window(m_GLFW);
#elif defined(__APPLE__)
		out.platform_window = glfwGetCocoaWindow(m_GLFW);
#elif defined(__linux__)
		if (glfwGetPlatform() == GLFW_PLATFORM_WAYLAND) {
			out.platform_window = glfwGetWaylandWindow(m_GLFW);
		} else if (glfwGetPlatform() == GLFW_PLATFORM_X11) {
			out.platform_window =
			    reinterpret_cast<void*>(
			        static_cast<uintptr_t>(glfwGetX11Window(m_GLFW)));
		}
#endif
		out.backend = m_WindowBackend;
		return out;
	}

	auto Window::fb_width() const -> u32 {
		int w, h;
		glfwGetFramebufferSize(m_GLFW, &w, &h);
		return static_cast<u32>(w);
	}

	auto Window::fb_height() const -> u32 {
		int w, h;
		glfwGetFramebufferSize(m_GLFW, &w, &h);
		return static_cast<u32>(h);
	}

	auto Window::fb_size() const -> std::pair<u32, u32> {
		int w, h;
		glfwGetFramebufferSize(m_GLFW, &w, &h);
		return std::make_pair<u32, u32>(w, h);
	}

	auto Window::focused() const -> bool {
		int value = glfwGetWindowAttrib(m_GLFW, GLFW_FOCUSED);
		return value == GLFW_TRUE;
	}

	auto Window::minimized() const -> bool {
		int value = glfwGetWindowAttrib(m_GLFW, GLFW_ICONIFIED);
		return value == GLFW_TRUE;
	}

	auto Window::maximized() const -> bool {
		int value = glfwGetWindowAttrib(m_GLFW, GLFW_MAXIMIZED);
		return value == GLFW_TRUE;
	}

	auto Window::visible() const -> bool {
		int value = glfwGetWindowAttrib(m_GLFW, GLFW_VISIBLE);
		return value == GLFW_TRUE;
	}

	auto Window::should_close() const -> bool {
		return glfwWindowShouldClose(m_GLFW);
	}

} // namespace aby::win::glfw
