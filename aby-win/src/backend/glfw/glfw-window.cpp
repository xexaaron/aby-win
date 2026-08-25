#include "backend/glfw/glfw-window.hpp"

#include "common.hpp"

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

	auto window_pos_callback(GLFWwindow* window, int x, int y) -> void;
	auto window_size_callback(GLFWwindow* window, int width, int height) -> void;
	auto window_close_callback(GLFWwindow* window) -> void;
	auto window_refresh_callback(GLFWwindow* window) -> void;
	auto window_focus_callback(GLFWwindow* window, int focused) -> void;
	auto window_iconify_callback(GLFWwindow* window, int iconified) -> void;
	auto window_maximize_callback(GLFWwindow* window, int maximized) -> void;
	auto framebuffer_size_callback(GLFWwindow* window, int width, int height) -> void;
	auto window_content_scale_callback(GLFWwindow* window, float xscale, float yscale) -> void;
	auto key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) -> void;
	auto mouse_button_callback(GLFWwindow* window, int button, int action, int mods) -> void;
	auto char_callback(GLFWwindow* window, unsigned int codepoint) -> void;
	auto cursor_enter_callback(GLFWwindow* window, int entered) -> void;
	auto scroll_callback(GLFWwindow* window, double xoffset, double yoffset) -> void;
	auto cursor_pos_callback(GLFWwindow* window, double x, double y) -> void;
	auto drop_callback(GLFWwindow* window, int count, const char** paths) -> void;
	auto err_callback(int error_code, const char* description) -> void;

	auto system_dark_theme() -> bool;
	auto get_listeners(GLFWwindow* window) -> std::span<WindowListener>;
	auto to_key(int key) -> EKey;
	auto to_mods(int mods) -> EMod;
	auto to_mouse_button(int button) -> EMouseButton;

	auto create_monitor_based_on_window_pos(GLFWwindow* window) -> std::unique_ptr<Monitor>;

	/**
	 * @brief Get the monitor the mouse is currently on
	 * @param monitor out monitor pointer parameter
	 * @param window the window to check against
	 * @author https://github.com/ghost 
	 * @link https://github.com/glfw/glfw/issues/1699
	 * @return false if monitor was not found, otherwise true 
	*/
	auto glfw_get_mouse_monitor(GLFWmonitor** monitor, GLFWwindow* window) -> bool;
	/**
	 * @brief Get the monitor the window is *most* geometrically on 
	 * @param monitor out monitor pointer parameter
	 * @param window the window to check against
	 * @author https://github.com/ghost 
	 * @link https://github.com/glfw/glfw/issues/1699
	 * @return false if monitor was not found, otherwise true 
	*/
	auto glfw_get_window_monitor(GLFWmonitor** monitor, GLFWwindow* window) -> bool;

	template <typename T>
	auto dispatch(GLFWwindow* window, T& event) -> void {
		for (auto& listener : get_listeners(window)) {
			if (listener(event))
				break;
		}
	}

} // namespace aby::win::glfw::detail

namespace aby::win::glfw {

	Window::Window(const Config& config) :
	    win::Window(config) {
		glfwSetErrorCallback(&detail::err_callback);

		if (!glfwInit()) {
			return;
		}

		if (config.render_backend == ERenderBackend::opengl) {
			glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
			glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		} else {
			glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		}

		glfwWindowHint(GLFW_RESIZABLE, config.resizable ? GLFW_TRUE : GLFW_FALSE);
		glfwWindowHint(GLFW_VISIBLE, config.visible ? GLFW_TRUE : GLFW_FALSE);
		glfwWindowHint(GLFW_DECORATED, config.decorated ? GLFW_TRUE : GLFW_FALSE);
		glfwWindowHint(GLFW_FOCUSED, config.focused ? GLFW_TRUE : GLFW_FALSE);

		if (m_GLFW = glfwCreateWindow(config.width, config.height, m_Name.data(), NULL, NULL); !m_GLFW) {
			glfwTerminate();
			return;
		}

		glfwSetWindowUserPointer(m_GLFW, this);
		glfwSetWindowPosCallback(m_GLFW, &detail::window_pos_callback);
		glfwSetWindowSizeCallback(m_GLFW, &detail::window_size_callback);
		glfwSetWindowCloseCallback(m_GLFW, &detail::window_close_callback);
		glfwSetWindowRefreshCallback(m_GLFW, &detail::window_refresh_callback);
		glfwSetWindowFocusCallback(m_GLFW, &detail::window_focus_callback);
		glfwSetWindowIconifyCallback(m_GLFW, &detail::window_iconify_callback);
		glfwSetWindowMaximizeCallback(m_GLFW, &detail::window_maximize_callback);
		glfwSetFramebufferSizeCallback(m_GLFW, &detail::framebuffer_size_callback);
		glfwSetWindowContentScaleCallback(m_GLFW, &detail::window_content_scale_callback);
		glfwSetKeyCallback(m_GLFW, &detail::key_callback);
		glfwSetCharCallback(m_GLFW, &detail::char_callback);
		glfwSetMouseButtonCallback(m_GLFW, &detail::mouse_button_callback);
		glfwSetCursorPosCallback(m_GLFW, &detail::cursor_pos_callback);
		glfwSetCursorEnterCallback(m_GLFW, &detail::cursor_enter_callback);
		glfwSetScrollCallback(m_GLFW, &detail::scroll_callback);
		glfwSetDropCallback(m_GLFW, &detail::drop_callback);
		set_theme(config.theme);

		m_Monitor = detail::create_monitor_based_on_window_pos(m_GLFW);
	}

	Window::~Window() {
		if (m_GLFW) {
			glfwDestroyWindow(m_GLFW);
			m_GLFW = nullptr;
		}
		glfwTerminate();
	}

	auto Window::set_name(std::string_view name) -> void {
		m_Name = std::string(name);
		glfwSetWindowTitle(m_GLFW, m_Name.c_str());
	}

	auto Window::set_width(uint32_t w) -> void {
		glfwSetWindowSize(m_GLFW, w, height());
	}

	auto Window::set_height(uint32_t h) -> void {
		glfwSetWindowSize(m_GLFW, width(), h);
	}

	auto Window::set_size(uint32_t w, uint32_t h) -> void {
		glfwSetWindowSize(m_GLFW, w, h);
	}

	auto Window::set_position(int32_t x, int32_t y) -> void {
		glfwSetWindowPos(m_GLFW, x, y);
	}

	auto Window::set_fullscreen(bool fullscreen) -> void {
		if (this->fullscreened() == fullscreen) {
			return;
		}

		if (fullscreen) {
			std::tie(m_WindowedX, m_WindowedY)          = position();
			std::tie(m_WindowedWidth, m_WindowedHeight) = size();

			GLFWmonitor* monitor = glfwGetPrimaryMonitor();
			if (!monitor) {
				aby_win_err("[glfw] failed to get primary monitor");
				return;
			}

			const GLFWvidmode* mode = glfwGetVideoMode(monitor);
			if (!mode) {
				aby_win_err("[glfw] failed to get primary monitor video mode");
				return;
			}

			glfwSetWindowMonitor(m_GLFW, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
		} else {
			glfwSetWindowMonitor(
			    m_GLFW,
			    nullptr,
			    m_WindowedX,
			    m_WindowedY,
			    m_WindowedWidth,
			    m_WindowedHeight,
			    0);
		}
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

	auto Window::add_listener(WindowListener&& listener) -> void {
		m_Listeners.push_back(std::move(listener));
	}

	auto Window::internal_set_monitor(std::unique_ptr<Monitor>&& monitor) -> void {
		m_Monitor = std::move(monitor);
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

	auto Window::hide() -> void {
		glfwHideWindow(m_GLFW);
	}

	auto Window::close() -> void {
		glfwSetWindowShouldClose(m_GLFW, GLFW_TRUE);
	}

	auto Window::poll() -> void {
		glfwPollEvents();
	}

	auto Window::width() const -> uint32_t {
		int32_t w, h;
		glfwGetWindowSize(m_GLFW, &w, &h);
		return w;
	}

	auto Window::height() const -> uint32_t {
		int32_t w, h;
		glfwGetWindowSize(m_GLFW, &w, &h);
		return h;
	}

	auto Window::size() const -> std::pair<uint32_t, uint32_t> {
		int32_t w, h;
		glfwGetWindowSize(m_GLFW, &w, &h);
		return std::make_pair<uint32_t, uint32_t>(w, h);
	}

	auto Window::position() const -> std::pair<int32_t, int32_t> {
		int32_t x, y;
		glfwGetWindowPos(m_GLFW, &x, &y);
		return std::make_pair(x, y);
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

	auto Window::fb_width() const -> uint32_t {
		int32_t w, h;
		glfwGetFramebufferSize(m_GLFW, &w, &h);
		return static_cast<uint32_t>(w);
	}

	auto Window::fb_height() const -> uint32_t {
		int32_t w, h;
		glfwGetFramebufferSize(m_GLFW, &w, &h);
		return static_cast<uint32_t>(h);
	}

	auto Window::fb_size() const -> std::pair<uint32_t, uint32_t> {
		int32_t w, h;
		glfwGetFramebufferSize(m_GLFW, &w, &h);
		return std::make_pair<uint32_t, uint32_t>(w, h);
	}

	auto Window::monitor() const -> const Monitor* {
		return m_Monitor.get();
	}

	auto Window::listeners() -> std::span<WindowListener> {
		return m_Listeners;
	}

	auto Window::focused() const -> bool {
		int32_t value = glfwGetWindowAttrib(m_GLFW, GLFW_FOCUSED);
		return value == GLFW_TRUE;
	}

	auto Window::minimized() const -> bool {
		int32_t value = glfwGetWindowAttrib(m_GLFW, GLFW_ICONIFIED);
		return value == GLFW_TRUE;
	}

	auto Window::maximized() const -> bool {
		int32_t value = glfwGetWindowAttrib(m_GLFW, GLFW_MAXIMIZED);
		return value == GLFW_TRUE;
	}

	auto Window::visible() const -> bool {
		int32_t value = glfwGetWindowAttrib(m_GLFW, GLFW_VISIBLE);
		return value == GLFW_TRUE;
	}

	auto Window::fullscreened() -> bool {
		return glfwGetWindowMonitor(m_GLFW) != nullptr;
	}

	auto Window::should_close() const -> bool {
		return glfwWindowShouldClose(m_GLFW);
	}

} // namespace aby::win::glfw

namespace aby::win::glfw::detail {

	auto err_callback(int error_code, const char* description) -> void {
		aby_win_err("[glfw] ({}): {}", error_code, description);
	}

	auto system_dark_theme() -> bool {
#ifdef _WIN32
		DWORD value         = 1;
		DWORD size          = sizeof(value);
		constexpr auto key  = L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize";
		constexpr auto name = L"AppsUseLightTheme";

		if (RegGetValueW(
		        HKEY_CURRENT_USER,
		        key,
		        name,
		        RRF_RT_REG_DWORD,
		        nullptr,
		        &value,
		        &size) != ERROR_SUCCESS) {
			// default to light if the setting cannot be queried.
			return false;
		}

		return value == 0;
#endif
		return false;
	}

	auto window_pos_callback(GLFWwindow* window, int x, int y) -> void {
		auto* win = static_cast<Window*>(glfwGetWindowUserPointer(window));
		win->internal_set_monitor(create_monitor_based_on_window_pos(window));

		WindowMovedEvent event(x, y);
		dispatch(window, event);
	}

	auto window_size_callback(GLFWwindow* window, int width, int height) -> void {
		WindowResizedEvent event(static_cast<uint32_t>(width), static_cast<uint32_t>(height));
		dispatch(window, event);
	}

	auto window_close_callback(GLFWwindow* window) -> void {
		WindowClosedEvent event;
		dispatch(window, event);
	}

	auto window_refresh_callback(GLFWwindow* window) -> void {
		WindowRefreshedEvent event;
		dispatch(window, event);
	}

	auto window_focus_callback(GLFWwindow* window, int focused) -> void {
		WindowFocusedEvent event(focused == GLFW_TRUE);
		dispatch(window, event);
	}

	auto window_iconify_callback(GLFWwindow* window, int iconified) -> void {
		WindowMinimizedEvent event(iconified == GLFW_TRUE);
		dispatch(window, event);
	}

	auto window_maximize_callback(GLFWwindow* window, int maximized) -> void {
		WindowMaximizedEvent event(maximized == GLFW_TRUE);
		dispatch(window, event);
	}

	auto framebuffer_size_callback(GLFWwindow* window, int width, int height) -> void {
		WindowFramebufferResizedEvent event(static_cast<uint32_t>(width), static_cast<uint32_t>(height));
		dispatch(window, event);
	}

	auto window_content_scale_callback(GLFWwindow* window, float xscale, float yscale) -> void {
		WindowScaledEvent event(xscale, yscale);
		dispatch(window, event);
	}

	auto key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) -> void {
		auto ekey = to_key(key);
		auto emod = to_mods(mods);

		switch (action) {
			case GLFW_PRESS: {
				KeyPressedEvent event(ekey, emod);
				dispatch(window, event);
				break;
			}

			case GLFW_RELEASE: {
				KeyReleasedEvent event(ekey, emod);
				dispatch(window, event);
				break;
			}

			case GLFW_REPEAT: {
				KeyPressedEvent event(ekey, emod);
				dispatch(window, event);
				break;
			}
		}
	}

	auto mouse_button_callback(GLFWwindow* window, int button, int action, int mods) -> void {
		auto ebutton = to_mouse_button(button);
		auto emod    = to_mods(mods);

		switch (action) {
			case GLFW_PRESS: {
				MousePressedEvent event(ebutton, emod);
				dispatch(window, event);
				break;
			}

			case GLFW_RELEASE: {
				MouseReleasedEvent event(ebutton, emod);
				dispatch(window, event);
				break;
			}
		}
	}

	auto char_callback(GLFWwindow* window, unsigned int codepoint) -> void {
		KeyTypedEvent event(static_cast<char32_t>(codepoint));
		dispatch(window, event);
	}

	auto cursor_enter_callback(GLFWwindow* window, int entered) -> void {
		if (entered == GLFW_TRUE) {
			MouseEnteredEvent event;
			dispatch(window, event);
		} else {
			MouseLeftEvent event;
			dispatch(window, event);
		}
	}

	auto scroll_callback(GLFWwindow* window, double xoffset, double yoffset) -> void {
		MouseScrolledEvent event(static_cast<float>(xoffset), static_cast<float>(yoffset));
		dispatch(window, event);
	}

	auto cursor_pos_callback(GLFWwindow* window, double x, double y) -> void {
		MouseMovedEvent event(static_cast<float>(x), static_cast<float>(y));

		dispatch(window, event);
	}

	auto drop_callback(GLFWwindow* window, int count, const char** paths) -> void {
		for (int i = 0; i < count; ++i) {
			const char* path = paths[i];
			auto fpath       = std::filesystem::path(path);
			FileDroppedEvent event(fpath);
			dispatch(window, event);
		}
	}

	auto get_listeners(GLFWwindow* window) -> std::span<WindowListener> {
		auto* win = static_cast<Window*>(glfwGetWindowUserPointer(window));
		return win->listeners();
	}

	auto to_key(int key) -> EKey {
		switch (key) {
			case GLFW_KEY_A:
				return EKey::a;
			case GLFW_KEY_B:
				return EKey::b;
			case GLFW_KEY_C:
				return EKey::c;
			case GLFW_KEY_D:
				return EKey::d;
			case GLFW_KEY_E:
				return EKey::e;
			case GLFW_KEY_F:
				return EKey::f;
			case GLFW_KEY_G:
				return EKey::g;
			case GLFW_KEY_H:
				return EKey::h;
			case GLFW_KEY_I:
				return EKey::i;
			case GLFW_KEY_J:
				return EKey::j;
			case GLFW_KEY_K:
				return EKey::k;
			case GLFW_KEY_L:
				return EKey::l;
			case GLFW_KEY_M:
				return EKey::m;
			case GLFW_KEY_N:
				return EKey::n;
			case GLFW_KEY_O:
				return EKey::o;
			case GLFW_KEY_P:
				return EKey::p;
			case GLFW_KEY_Q:
				return EKey::q;
			case GLFW_KEY_R:
				return EKey::r;
			case GLFW_KEY_S:
				return EKey::s;
			case GLFW_KEY_T:
				return EKey::t;
			case GLFW_KEY_U:
				return EKey::u;
			case GLFW_KEY_V:
				return EKey::v;
			case GLFW_KEY_W:
				return EKey::w;
			case GLFW_KEY_X:
				return EKey::x;
			case GLFW_KEY_Y:
				return EKey::y;
			case GLFW_KEY_Z:
				return EKey::z;
			case GLFW_KEY_0:
				return EKey::num_0;
			case GLFW_KEY_1:
				return EKey::num_1;
			case GLFW_KEY_2:
				return EKey::num_2;
			case GLFW_KEY_3:
				return EKey::num_3;
			case GLFW_KEY_4:
				return EKey::num_4;
			case GLFW_KEY_5:
				return EKey::num_5;
			case GLFW_KEY_6:
				return EKey::num_6;
			case GLFW_KEY_7:
				return EKey::num_7;
			case GLFW_KEY_8:
				return EKey::num_8;
			case GLFW_KEY_9:
				return EKey::num_9;
			case GLFW_KEY_F1:
				return EKey::f1;
			case GLFW_KEY_F2:
				return EKey::f2;
			case GLFW_KEY_F3:
				return EKey::f3;
			case GLFW_KEY_F4:
				return EKey::f4;
			case GLFW_KEY_F5:
				return EKey::f5;
			case GLFW_KEY_F6:
				return EKey::f6;
			case GLFW_KEY_F7:
				return EKey::f7;
			case GLFW_KEY_F8:
				return EKey::f8;
			case GLFW_KEY_F9:
				return EKey::f9;
			case GLFW_KEY_F10:
				return EKey::f10;
			case GLFW_KEY_F11:
				return EKey::f11;
			case GLFW_KEY_F12:
				return EKey::f12;
			case GLFW_KEY_LEFT_SHIFT:
				return EKey::left_shift;
			case GLFW_KEY_RIGHT_SHIFT:
				return EKey::right_shift;
			case GLFW_KEY_LEFT_CONTROL:
				return EKey::left_ctrl;
			case GLFW_KEY_RIGHT_CONTROL:
				return EKey::right_ctrl;
			case GLFW_KEY_LEFT_ALT:
				return EKey::left_alt;
			case GLFW_KEY_RIGHT_ALT:
				return EKey::right_alt;
			case GLFW_KEY_LEFT_SUPER:
				return EKey::left_super;
			case GLFW_KEY_RIGHT_SUPER:
				return EKey::right_super;
			case GLFW_KEY_UP:
				return EKey::up;
			case GLFW_KEY_DOWN:
				return EKey::down;
			case GLFW_KEY_LEFT:
				return EKey::left;
			case GLFW_KEY_RIGHT:
				return EKey::right;
			case GLFW_KEY_HOME:
				return EKey::home;
			case GLFW_KEY_END:
				return EKey::end;
			case GLFW_KEY_PAGE_UP:
				return EKey::page_up;
			case GLFW_KEY_PAGE_DOWN:
				return EKey::page_down;
			case GLFW_KEY_INSERT:
				return EKey::insert;
			case GLFW_KEY_DELETE:
				return EKey::del;
			case GLFW_KEY_BACKSPACE:
				return EKey::backspace;
			case GLFW_KEY_ENTER:
				return EKey::enter;
			case GLFW_KEY_TAB:
				return EKey::tab;
			case GLFW_KEY_ESCAPE:
				return EKey::escape;
			case GLFW_KEY_SPACE:
				return EKey::space;
			case GLFW_KEY_APOSTROPHE:
				return EKey::apostrophe;
			case GLFW_KEY_COMMA:
				return EKey::comma;
			case GLFW_KEY_MINUS:
				return EKey::minus;
			case GLFW_KEY_PERIOD:
				return EKey::period;
			case GLFW_KEY_SLASH:
				return EKey::slash;
			case GLFW_KEY_SEMICOLON:
				return EKey::semicolon;
			case GLFW_KEY_EQUAL:
				return EKey::equal;
			case GLFW_KEY_LEFT_BRACKET:
				return EKey::left_bracket;
			case GLFW_KEY_BACKSLASH:
				return EKey::backslash;
			case GLFW_KEY_RIGHT_BRACKET:
				return EKey::right_bracket;
			case GLFW_KEY_GRAVE_ACCENT:
				return EKey::grave_accent;
			case GLFW_KEY_CAPS_LOCK:
				return EKey::caps_lock;
			case GLFW_KEY_NUM_LOCK:
				return EKey::num_lock;
			case GLFW_KEY_SCROLL_LOCK:
				return EKey::scroll_lock;
			case GLFW_KEY_KP_0:
				return EKey::kp_0;
			case GLFW_KEY_KP_1:
				return EKey::kp_1;
			case GLFW_KEY_KP_2:
				return EKey::kp_2;
			case GLFW_KEY_KP_3:
				return EKey::kp_3;
			case GLFW_KEY_KP_4:
				return EKey::kp_4;
			case GLFW_KEY_KP_5:
				return EKey::kp_5;
			case GLFW_KEY_KP_6:
				return EKey::kp_6;
			case GLFW_KEY_KP_7:
				return EKey::kp_7;
			case GLFW_KEY_KP_8:
				return EKey::kp_8;
			case GLFW_KEY_KP_9:
				return EKey::kp_9;
			case GLFW_KEY_KP_DECIMAL:
				return EKey::kp_decimal;
			case GLFW_KEY_KP_DIVIDE:
				return EKey::kp_divide;
			case GLFW_KEY_KP_MULTIPLY:
				return EKey::kp_multiply;
			case GLFW_KEY_KP_SUBTRACT:
				return EKey::kp_subtract;
			case GLFW_KEY_KP_ADD:
				return EKey::kp_add;
			case GLFW_KEY_KP_ENTER:
				return EKey::kp_enter;
			case GLFW_KEY_KP_EQUAL:
				return EKey::kp_equal;
			case GLFW_KEY_PRINT_SCREEN:
				return EKey::print_screen;
			case GLFW_KEY_PAUSE:
				return EKey::pause;
			case GLFW_KEY_MENU:
				return EKey::menu;
			default:
				return EKey::unknown;
		}
	}

	auto to_mods(int mods) -> EMod {
		EMod out = EMod::none;

		if (mods & GLFW_MOD_SHIFT)
			out |= EMod::shift;

		if (mods & GLFW_MOD_CONTROL)
			out |= EMod::ctrl;

		if (mods & GLFW_MOD_ALT)
			out |= EMod::alt;

		if (mods & GLFW_MOD_SUPER)
			out |= EMod::super;

		if (mods & GLFW_MOD_CAPS_LOCK)
			out |= EMod::caps_lock;

		if (mods & GLFW_MOD_NUM_LOCK)
			out |= EMod::num_lock;

		return out;
	}

	auto to_mouse_button(int button) -> EMouseButton {
		switch (button) {
			case GLFW_MOUSE_BUTTON_LEFT:
				return EMouseButton::left;
			case GLFW_MOUSE_BUTTON_RIGHT:
				return EMouseButton::right;
			case GLFW_MOUSE_BUTTON_MIDDLE:
				return EMouseButton::middle;
			case GLFW_MOUSE_BUTTON_4:
				return EMouseButton::button_4;
			case GLFW_MOUSE_BUTTON_5:
				return EMouseButton::button_5;
			case GLFW_MOUSE_BUTTON_6:
				return EMouseButton::button_6;
			case GLFW_MOUSE_BUTTON_7:
				return EMouseButton::button_7;
			case GLFW_MOUSE_BUTTON_8:
				return EMouseButton::button_8;
			default:
				return EMouseButton::none;
		}
	}

	auto create_monitor_based_on_window_pos(GLFWwindow* window) -> std::unique_ptr<Monitor> {
		GLFWmonitor* window_monitor = nullptr;

		if (detail::glfw_get_window_monitor(&window_monitor, window)) {
			int32_t x, y;
			glfwGetMonitorPos(window_monitor, &x, &y);

			WorkArea work_area;
			glfwGetMonitorWorkarea(
			    window_monitor,
			    &work_area.x,
			    &work_area.y,
			    &work_area.w,
			    &work_area.h);

			int32_t physical_width_mm;
			int32_t physical_height_mm;
			glfwGetMonitorPhysicalSize(window_monitor, &physical_width_mm, &physical_height_mm);

			float scale_x, scale_y;
			glfwGetMonitorContentScale(window_monitor, &scale_x, &scale_y);

			const char* monitor_name = glfwGetMonitorName(window_monitor);

			int32_t video_mode_count;
			const GLFWvidmode* glfw_video_modes   = glfwGetVideoModes(window_monitor, &video_mode_count);
			const GLFWvidmode* current_video_mode = glfwGetVideoMode(window_monitor);

			std::vector<VideoMode> video_modes;
			video_modes.reserve(video_mode_count);

			size_t current_video_mode_index = 0;

			for (int32_t i = 0; i < video_mode_count; ++i) {
				const auto* mode = &glfw_video_modes[i];

				if (current_video_mode->width == mode->width &&
				    current_video_mode->height == mode->height &&
				    current_video_mode->redBits == mode->redBits &&
				    current_video_mode->greenBits == mode->greenBits &&
				    current_video_mode->blueBits == mode->blueBits &&
				    current_video_mode->refreshRate == mode->refreshRate) {
					current_video_mode_index = static_cast<size_t>(i);
				}

				video_modes.emplace_back(
				    std::make_pair(mode->width, mode->height),
				    std::make_tuple(
				        mode->redBits,
				        mode->greenBits,
				        mode->blueBits),
				    mode->refreshRate);
			}

			return std::make_unique<Monitor>(
			    std::make_pair(x, y),
			    work_area,
			    std::make_pair(physical_width_mm, physical_height_mm),
			    std::make_pair(scale_x, scale_y),
			    monitor_name ? monitor_name : "",
			    std::move(video_modes),
			    current_video_mode_index);
		}

		return nullptr;
	}

	auto glfw_get_mouse_monitor(GLFWmonitor** monitor, GLFWwindow* window) -> bool {
		bool success = false;

		double cursor_position[2] = { 0 };
		glfwGetCursorPos(window, &cursor_position[0], &cursor_position[1]);

		int window_position[2] = { 0 };
		glfwGetWindowPos(window, &window_position[0], &window_position[1]);

		int monitors_size      = 0;
		GLFWmonitor** monitors = glfwGetMonitors(&monitors_size);

		// convert cursor position from window coordinates to screen coordinates
		cursor_position[0] += window_position[0];
		cursor_position[1] += window_position[1];

		for (int i = 0; ((!success) && (i < monitors_size)); ++i) {
			int monitor_position[2] = { 0 };
			glfwGetMonitorPos(monitors[i], &monitor_position[0], &monitor_position[1]);

			const GLFWvidmode* monitor_video_mode = glfwGetVideoMode(monitors[i]);

			if (
			    (cursor_position[0] < monitor_position[0]) ||
			    (cursor_position[0] > (monitor_position[0] + monitor_video_mode->width)) ||
			    (cursor_position[1] < monitor_position[1]) ||
			    (cursor_position[1] > (monitor_position[1] + monitor_video_mode->height))) {
				*monitor = monitors[i];
				success  = true;
			}
		}

		return success;
	}

	auto glfw_get_window_monitor(GLFWmonitor** monitor, GLFWwindow* window) -> bool {
		bool success = false;

		int window_rectangle[4] = { 0 };
		glfwGetWindowPos(window, &window_rectangle[0], &window_rectangle[1]);
		glfwGetWindowSize(window, &window_rectangle[2], &window_rectangle[3]);

		int monitors_size      = 0;
		GLFWmonitor** monitors = glfwGetMonitors(&monitors_size);

		GLFWmonitor* closest_monitor = NULL;
		int max_overlap_area         = 0;

		for (int i = 0; i < monitors_size; ++i) {
			int monitor_position[2] = { 0 };
			glfwGetMonitorPos(monitors[i], &monitor_position[0], &monitor_position[1]);
			const GLFWvidmode* monitor_video_mode = glfwGetVideoMode(monitors[i]);

			int monitor_rectangle[4] = {
				monitor_position[0],
				monitor_position[1],
				monitor_video_mode->width,
				monitor_video_mode->height
			};

			if (
			    !(
			        ((window_rectangle[0] + window_rectangle[2]) < monitor_rectangle[0]) ||
			        (window_rectangle[0] > (monitor_rectangle[0] + monitor_rectangle[2])) ||
			        ((window_rectangle[1] + window_rectangle[3]) < monitor_rectangle[1]) ||
			        (window_rectangle[1] > (monitor_rectangle[1] + monitor_rectangle[3])))) {
				int intersection_rectangle[4] = { 0 };

				// x, width
				if (window_rectangle[0] < monitor_rectangle[0]) {
					intersection_rectangle[0] = monitor_rectangle[0];

					if ((window_rectangle[0] + window_rectangle[2]) < (monitor_rectangle[0] + monitor_rectangle[2])) {
						intersection_rectangle[2] = (window_rectangle[0] + window_rectangle[2]) - intersection_rectangle[0];
					} else {
						intersection_rectangle[2] = monitor_rectangle[2];
					}
				} else {
					intersection_rectangle[0] = window_rectangle[0];

					if ((monitor_rectangle[0] + monitor_rectangle[2]) < (window_rectangle[0] + window_rectangle[2])) {
						intersection_rectangle[2] = (monitor_rectangle[0] + monitor_rectangle[2]) - intersection_rectangle[0];
					} else {
						intersection_rectangle[2] = window_rectangle[2];
					}
				}

				// y, height
				if (window_rectangle[1] < monitor_rectangle[1]) {
					intersection_rectangle[1] = monitor_rectangle[1];

					if ((window_rectangle[1] + window_rectangle[3]) < (monitor_rectangle[1] + monitor_rectangle[3])) {
						intersection_rectangle[3] = (window_rectangle[1] + window_rectangle[3]) - intersection_rectangle[1];
					} else {
						intersection_rectangle[3] = monitor_rectangle[3];
					}
				} else {
					intersection_rectangle[1] = window_rectangle[1];

					if ((monitor_rectangle[1] + monitor_rectangle[3]) < (window_rectangle[1] + window_rectangle[3])) {
						intersection_rectangle[3] = (monitor_rectangle[1] + monitor_rectangle[3]) - intersection_rectangle[1];
					} else {
						intersection_rectangle[3] = window_rectangle[3];
					}
				}

				int overlap_area = intersection_rectangle[2] * intersection_rectangle[3];
				if (overlap_area > max_overlap_area) {
					closest_monitor  = monitors[i];
					max_overlap_area = overlap_area;
				}
			}
		}

		if (closest_monitor) {
			*monitor = closest_monitor;
			success  = true;
		}

		return success;
	}

} // namespace aby::win::glfw::detail
