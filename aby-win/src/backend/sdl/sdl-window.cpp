#include "backend/sdl/sdl-window.hpp"

#include "common.hpp"

#include <SDL3/SDL.h>

namespace aby::win::sdl::detail {

	auto to_key(SDL_Keycode key) -> EKey;
	auto to_mods(SDL_Keymod mods) -> EMod;
	auto to_mouse_button(uint8_t button) -> EMouseButton;

	auto sdl_get_phys_dp_size(SDL_DisplayID id) -> std::pair<int32_t, int32_t>;
	auto sdl_get_current_monitor(SDL_Window* window) -> std::unique_ptr<Monitor>;

} // namespace aby::win::sdl::detail

namespace aby::win::sdl {

	Window::Window(const Config& config) :
	    win::Window(config) {
		if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK)) {
			aby_win_err("[sdl] failed to initialize SDL: {}", SDL_GetError());
			return;
		}

		SDL_WindowFlags flags = 0;
		if (config.resizable) {
			flags |= SDL_WINDOW_RESIZABLE;
		}
		if (!config.visible) {
			flags |= SDL_WINDOW_HIDDEN;
		}
		if (!config.decorated) {
			flags |= SDL_WINDOW_BORDERLESS;
		}

		bDecorated = config.decorated;

		if (config.focused) {
			flags |= SDL_WINDOW_INPUT_FOCUS;
			flags |= SDL_WINDOW_MOUSE_FOCUS;
		}

		switch (config.render_backend) {
			case ERenderBackend::opengl:
				flags |= SDL_WINDOW_OPENGL;
				break;
			case ERenderBackend::vulkan:
				flags |= SDL_WINDOW_VULKAN;
				break;
			case ERenderBackend::d3d:
				// SDL doesn't need a special flag for D3D.
				break;
			case ERenderBackend::metal:
				flags |= SDL_WINDOW_METAL;
				break;
			case ERenderBackend::none:
				break;
		}

		m_SDL = SDL_CreateWindow(m_Name.c_str(), static_cast<int>(config.width), static_cast<int>(config.height), flags);

		if (!m_SDL) {
			aby_win_err("[sdl] failed to create window: {}", SDL_GetError());
			SDL_Quit();
			return;
		}

		SDL_SetWindowPosition(m_SDL, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);

		m_Monitor = detail::sdl_get_current_monitor(m_SDL);
	}

	Window::~Window() {
		if (m_Icon) {
			SDL_DestroySurface(m_Icon);
		}
		if (m_SDL) {
			SDL_DestroyWindow(m_SDL);
			m_SDL = nullptr;
		}

		SDL_Quit();
	}

	auto Window::set_name(std::string_view name) -> void {
		m_Name = std::string(name);
		SDL_SetWindowTitle(m_SDL, m_Name.data());
	}

	auto Window::set_width(uint32_t w) -> void {
		SDL_SetWindowSize(m_SDL, w, height());
	}

	auto Window::set_height(uint32_t h) -> void {
		SDL_SetWindowSize(m_SDL, width(), h);
	}

	auto Window::set_size(uint32_t w, uint32_t h) -> void {
		SDL_SetWindowSize(m_SDL, w, h);
	}

	auto Window::set_position(int32_t x, int32_t y) -> void {
		SDL_SetWindowPosition(m_SDL, x, y);
	}

	auto Window::set_fullscreen(bool fullscreen) -> void {
		SDL_SetWindowFullscreen(m_SDL, fullscreen);
	}

	auto Window::set_cursor_mode(ECursorMode mode) -> void {
		switch (mode) {
			case ECursorMode::normal:
				SDL_ShowCursor();
				break;
			case ECursorMode::hidden:
			case ECursorMode::disabled:
				SDL_HideCursor();
				break;
		}
	}

	auto Window::set_cursor_pos(float x, float y) -> void {
		SDL_WarpMouseInWindow(m_SDL, x, y);
	}

	auto Window::set_theme(ETheme theme) -> void {
		m_Theme = theme;
		aby_win_wrn("theme control not implemented for SDL, SDL Automatically tracks system the system theme");
	}

	auto Window::set_icon(const Icon& icon) -> void {
		if (m_Icon) {
			SDL_DestroySurface(m_Icon);
		}
		m_Icon = SDL_CreateSurfaceFrom(icon.width, icon.height, SDL_PIXELFORMAT_RGBA8888, icon.pixels.data(), icon.width * 4);
		SDL_SetWindowIcon(m_SDL, m_Icon);
	}

	auto Window::set_hit_test_config(const HitTestConfig& cfg) -> void {
		if (bDecorated) {
			aby_win_wrn("[sdl] The window was not set as undecorated. The hit test configuration will be ignored");
			return;
		}

		m_HitTestConfig = cfg;

		if (bHitFnSet) {
			return;
		}

		SDL_SetWindowHitTest(m_SDL, [](SDL_Window* win, const SDL_Point* p, void* user) -> SDL_HitTestResult {
			auto* cfg = static_cast<HitTestConfig*>(user);

			int width  = 0;
			int height = 0;
			SDL_GetWindowSize(win, &width, &height);

			const int border = static_cast<int>(cfg->resize_border);

			if (p->x <= border && p->y <= border) {
				return SDL_HITTEST_RESIZE_TOPLEFT;
			}

			if (p->x >= width - border && p->y <= border) {
				return SDL_HITTEST_RESIZE_TOPRIGHT;
			}

			if (p->x <= border && p->y >= height - border) {
				return SDL_HITTEST_RESIZE_BOTTOMLEFT;
			}

			if (p->x >= width - border && p->y >= height - border) {
				return SDL_HITTEST_RESIZE_BOTTOMRIGHT;
			}

			if (p->y <= border) {
				return SDL_HITTEST_RESIZE_TOP;
			}

			if (p->y >= height - border) {
				return SDL_HITTEST_RESIZE_BOTTOM;
			}

			if (p->x <= border) {
				return SDL_HITTEST_RESIZE_LEFT;
			}

			if (p->x >= width - border) {
				return SDL_HITTEST_RESIZE_RIGHT;
			}

			if (p->y <= static_cast<int>(cfg->title_bar_height)) {
				return SDL_HITTEST_DRAGGABLE;
			}

			return SDL_HITTEST_NORMAL;
		}, &m_HitTestConfig);

		bHitFnSet = true;
	}

	auto Window::add_listener(WindowListener&& listener) -> void {
		m_Listeners.push_back(std::move(listener));
	}

	auto Window::focus() -> void {
		SDL_RaiseWindow(m_SDL);
	}

	auto Window::minimize() -> void {
		SDL_MinimizeWindow(m_SDL);
	}

	auto Window::maximize() -> void {
		SDL_MaximizeWindow(m_SDL);
	}

	auto Window::show() -> void {
		SDL_ShowWindow(m_SDL);
	}

	auto Window::hide() -> void {
		SDL_HideWindow(m_SDL);
	}

	auto Window::close() -> void {
		bShouldClose = true;
	}

	auto Window::poll() -> void {
		SDL_Event sdl_event;

		while (SDL_PollEvent(&sdl_event)) {
			switch (sdl_event.type) {
				case SDL_EVENT_QUIT: {
					bShouldClose = true;
					WindowClosedEvent event;
					dispatch(event);
					break;
				}
				case SDL_EVENT_TERMINATING:
				case SDL_EVENT_LOW_MEMORY:
				case SDL_EVENT_WILL_ENTER_BACKGROUND:
				case SDL_EVENT_DID_ENTER_BACKGROUND:
				case SDL_EVENT_WILL_ENTER_FOREGROUND:
				case SDL_EVENT_DID_ENTER_FOREGROUND:
				case SDL_EVENT_LOCALE_CHANGED:
				case SDL_EVENT_SYSTEM_THEME_CHANGED:
				case SDL_EVENT_DISPLAY_ORIENTATION: {
					NativeEvent event(static_cast<void*>(&sdl_event));
					dispatch(event);
					break;
				}
				case SDL_EVENT_DISPLAY_ADDED: {
					MonitorConnectedEvent event(sdl_event.display.displayID);
					dispatch(event);
					break;
				}
				case SDL_EVENT_DISPLAY_REMOVED: {
					MonitorDisconnectedEvent event(sdl_event.display.displayID);
					dispatch(event);
					break;
				}
				case SDL_EVENT_DISPLAY_MOVED:
				case SDL_EVENT_DISPLAY_DESKTOP_MODE_CHANGED:
				case SDL_EVENT_DISPLAY_CURRENT_MODE_CHANGED:
				case SDL_EVENT_DISPLAY_CONTENT_SCALE_CHANGED:
				case SDL_EVENT_DISPLAY_USABLE_BOUNDS_CHANGED:
				case SDL_EVENT_WINDOW_SHOWN:
				case SDL_EVENT_WINDOW_HIDDEN:
				case SDL_EVENT_WINDOW_EXPOSED: {
					NativeEvent event(static_cast<void*>(&sdl_event));
					dispatch(event);
					break;
				}
				case SDL_EVENT_WINDOW_MOVED: {
					m_Monitor = detail::sdl_get_current_monitor(m_SDL);
					WindowMovedEvent event(sdl_event.window.data1, sdl_event.window.data2);
					dispatch(event);
					break;
				}
				case SDL_EVENT_WINDOW_RESIZED: {
					WindowResizedEvent event(
					    static_cast<uint32_t>(sdl_event.window.data1),
					    static_cast<uint32_t>(sdl_event.window.data2));
					dispatch(event);
					break;
				}
				case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED: {
					WindowFramebufferResizedEvent event(
					    static_cast<uint32_t>(sdl_event.window.data1),
					    static_cast<uint32_t>(sdl_event.window.data2));
					dispatch(event);
					break;
				}
				case SDL_EVENT_WINDOW_METAL_VIEW_RESIZED: {
					NativeEvent event(&sdl_event);
					dispatch(event);
					break;
				}
				case SDL_EVENT_WINDOW_MINIMIZED: {
					WindowMinimizedEvent event;
					dispatch(event);
					break;
				}
				case SDL_EVENT_WINDOW_MAXIMIZED: {
					WindowMaximizedEvent event;
					dispatch(event);
					break;
				}
				case SDL_EVENT_WINDOW_RESTORED:
				case SDL_EVENT_WINDOW_MOUSE_ENTER: {
					MouseEnteredEvent event;
					dispatch(event);
					break;
				}
				case SDL_EVENT_WINDOW_MOUSE_LEAVE: {
					MouseLeftEvent event;
					dispatch(event);
					break;
				}
				case SDL_EVENT_WINDOW_FOCUS_GAINED: {
					WindowFocusedEvent event;
					dispatch(event);
					break;
				}
				case SDL_EVENT_WINDOW_FOCUS_LOST: {
					NativeEvent event(&sdl_event);
					dispatch(event);
					break;
				}
				case SDL_EVENT_WINDOW_CLOSE_REQUESTED: {
					bShouldClose = true;
					break;
				}
				case SDL_EVENT_WINDOW_HIT_TEST:
				case SDL_EVENT_WINDOW_ICCPROF_CHANGED:
				case SDL_EVENT_WINDOW_DISPLAY_CHANGED:
				case SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED:
				case SDL_EVENT_WINDOW_SAFE_AREA_CHANGED:
				case SDL_EVENT_WINDOW_OCCLUDED:
				case SDL_EVENT_WINDOW_ENTER_FULLSCREEN:
				case SDL_EVENT_WINDOW_LEAVE_FULLSCREEN:
				case SDL_EVENT_WINDOW_DESTROYED:
				case SDL_EVENT_WINDOW_HDR_STATE_CHANGED:
				case SDL_EVENT_WINDOW_SETTINGS_CHANGED: {
					NativeEvent event(&sdl_event);
					dispatch(event);
					break;
				}
				case SDL_EVENT_KEY_DOWN: {
					auto ebutton = detail::to_key(sdl_event.key.key);
					auto mods    = detail::to_mods(sdl_event.key.mod);
					KeyPressedEvent event(ebutton);
					dispatch(event);
					break;
				}
				case SDL_EVENT_KEY_UP: {
					auto ebutton = detail::to_key(sdl_event.key.key);
					auto mods    = detail::to_mods(sdl_event.key.mod);
					KeyPressedEvent event(ebutton);
					dispatch(event);
					break;
				}
				case SDL_EVENT_TEXT_EDITING: {
					NativeEvent event(&sdl_event);
					dispatch(event);
					break;
				}
				case SDL_EVENT_TEXT_INPUT: {
					auto len = strlen(sdl_event.text.text);
					for (size_t i = 0; i < len; i++) {
						KeyTypedEvent event(static_cast<char32_t>(sdl_event.text.text[i]));
						dispatch(event);
					}
					break;
				}
				case SDL_EVENT_KEYMAP_CHANGED:
				case SDL_EVENT_KEYBOARD_ADDED:
				case SDL_EVENT_KEYBOARD_REMOVED:
				case SDL_EVENT_TEXT_EDITING_CANDIDATES:
				case SDL_EVENT_SCREEN_KEYBOARD_SHOWN:
				case SDL_EVENT_SCREEN_KEYBOARD_HIDDEN: {
					NativeEvent event(&sdl_event);
					dispatch(event);
					break;
				}
				case SDL_EVENT_MOUSE_MOTION: {
					MouseMovedEvent event(sdl_event.motion.x, sdl_event.motion.y);
					dispatch(event);
					break;
				}
				case SDL_EVENT_MOUSE_BUTTON_DOWN: {
					auto ebutton = detail::to_mouse_button(sdl_event.button.button);
					auto emods   = detail::to_mods(SDL_GetModState());
					MousePressedEvent event(ebutton, emods);
					dispatch(event);
					break;
				}
				case SDL_EVENT_MOUSE_BUTTON_UP: {
					auto ebutton = detail::to_mouse_button(sdl_event.button.button);
					auto emods   = detail::to_mods(SDL_GetModState());
					MouseReleasedEvent event(ebutton, emods);
					dispatch(event);
					break;
				}
				case SDL_EVENT_MOUSE_WHEEL: {
					MouseScrolledEvent event(sdl_event.button.x, sdl_event.button.y);
					dispatch(event);
					break;
				}
				case SDL_EVENT_MOUSE_ADDED:
				case SDL_EVENT_MOUSE_REMOVED: {
					NativeEvent event(&sdl_event);
					dispatch(event);
					break;
				}
				case SDL_EVENT_JOYSTICK_AXIS_MOTION:
				case SDL_EVENT_JOYSTICK_BALL_MOTION:
				case SDL_EVENT_JOYSTICK_HAT_MOTION:
				case SDL_EVENT_JOYSTICK_BUTTON_DOWN:
				case SDL_EVENT_JOYSTICK_BUTTON_UP:
				case SDL_EVENT_JOYSTICK_ADDED:
				case SDL_EVENT_JOYSTICK_REMOVED:
				case SDL_EVENT_JOYSTICK_BATTERY_UPDATED:
				case SDL_EVENT_JOYSTICK_UPDATE_COMPLETE: {
					NativeEvent event(&sdl_event);
					dispatch(event);
					break;
				}
				case SDL_EVENT_GAMEPAD_AXIS_MOTION:
				case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
				case SDL_EVENT_GAMEPAD_BUTTON_UP:
				case SDL_EVENT_GAMEPAD_ADDED:
				case SDL_EVENT_GAMEPAD_REMOVED:
				case SDL_EVENT_GAMEPAD_REMAPPED:
				case SDL_EVENT_GAMEPAD_TOUCHPAD_DOWN:
				case SDL_EVENT_GAMEPAD_TOUCHPAD_MOTION:
				case SDL_EVENT_GAMEPAD_TOUCHPAD_UP:
				case SDL_EVENT_GAMEPAD_SENSOR_UPDATE:
				case SDL_EVENT_GAMEPAD_UPDATE_COMPLETE:
				case SDL_EVENT_GAMEPAD_STEAM_HANDLE_UPDATED:
				case SDL_EVENT_GAMEPAD_CAPSENSE_TOUCH:
				case SDL_EVENT_GAMEPAD_CAPSENSE_RELEASE: {
					NativeEvent event(&sdl_event);
					dispatch(event);
					break;
				}
				case SDL_EVENT_FINGER_DOWN:
				case SDL_EVENT_FINGER_UP:
				case SDL_EVENT_FINGER_MOTION:
				case SDL_EVENT_FINGER_CANCELED: {
					NativeEvent event(&sdl_event);
					dispatch(event);
					break;
				}
				case SDL_EVENT_PINCH_BEGIN:
				case SDL_EVENT_PINCH_UPDATE:
				case SDL_EVENT_PINCH_END: {
					NativeEvent event(&sdl_event);
					dispatch(event);
					break;
				}
				case SDL_EVENT_CLIPBOARD_UPDATE: {
					NativeEvent event(&sdl_event);
					dispatch(event);
					break;
				}
				case SDL_EVENT_DROP_FILE: {
					FileDroppedEvent event(sdl_event.drop.data);
					dispatch(event);
					break;
				}
				case SDL_EVENT_DROP_TEXT:
				case SDL_EVENT_DROP_BEGIN:
				case SDL_EVENT_DROP_COMPLETE:
				case SDL_EVENT_DROP_POSITION: {
					NativeEvent event(&sdl_event);
					dispatch(event);
					break;
				}
				case SDL_EVENT_AUDIO_DEVICE_ADDED:
				case SDL_EVENT_AUDIO_DEVICE_REMOVED:
				case SDL_EVENT_AUDIO_DEVICE_FORMAT_CHANGED: {
					NativeEvent event(&sdl_event);
					dispatch(event);
					break;
				}
				case SDL_EVENT_SENSOR_UPDATE: {
					NativeEvent event(&sdl_event);
					dispatch(event);
					break;
				}
				case SDL_EVENT_PEN_PROXIMITY_IN:
				case SDL_EVENT_PEN_PROXIMITY_OUT:
				case SDL_EVENT_PEN_DOWN:
				case SDL_EVENT_PEN_UP:
				case SDL_EVENT_PEN_BUTTON_DOWN:
				case SDL_EVENT_PEN_BUTTON_UP:
				case SDL_EVENT_PEN_MOTION:
				case SDL_EVENT_PEN_AXIS: {
					NativeEvent event(&sdl_event);
					dispatch(event);
					break;
				}
				case SDL_EVENT_CAMERA_DEVICE_ADDED:
				case SDL_EVENT_CAMERA_DEVICE_REMOVED:
				case SDL_EVENT_CAMERA_DEVICE_APPROVED:
				case SDL_EVENT_CAMERA_DEVICE_DENIED: {
					NativeEvent event(&sdl_event);
					dispatch(event);
					break;
				}
				case SDL_EVENT_NOTIFICATION_ACTION_INVOKED:
				case SDL_EVENT_RENDER_TARGETS_RESET:
				case SDL_EVENT_RENDER_DEVICE_RESET:
				case SDL_EVENT_RENDER_DEVICE_LOST: {
					NativeEvent event(&sdl_event);
					dispatch(event);
					break;
				}
				case SDL_EVENT_POLL_SENTINEL:
					break;
			}
		}
	}

	auto Window::width() const -> uint32_t {
		int32_t w, h;
		SDL_GetWindowSize(m_SDL, &w, &h);
		return w;
	}

	auto Window::height() const -> uint32_t {
		int32_t w, h;
		SDL_GetWindowSize(m_SDL, &w, &h);
		return h;
	}

	auto Window::size() const -> std::pair<uint32_t, uint32_t> {
		int32_t w, h;
		SDL_GetWindowSize(m_SDL, &w, &h);
		return std::make_pair<uint32_t, uint32_t>(w, h);
	}

	auto Window::position() const -> std::pair<int32_t, int32_t> {
		int32_t x, y;
		SDL_GetWindowPosition(m_SDL, &x, &y);
		return std::make_pair(x, y);
	}

	auto Window::scale() const -> float {
		return SDL_GetWindowDisplayScale(m_SDL);
	}

	auto Window::native() const -> NativeWindow {
		NativeWindow out{
			.backend_window  = m_SDL,
			.platform_window = nullptr,
			.backend         = EWindow::sdl
		};

#if defined(_WIN32)

		SDL_PropertiesID props = SDL_GetWindowProperties(m_SDL);
		out.platform_window    = SDL_GetPointerProperty(props, SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr);
#elif defined(__APPLE__)
		SDL_PropertiesID props = SDL_GetWindowProperties(m_SDL);
		out.platform_window    = SDL_GetPointerProperty(props, SDL_PROP_WINDOW_COCOA_WINDOW_POINTER, nullptr);
#elif defined(__linux__)
		SDL_PropertiesID props = SDL_GetWindowProperties(m_SDL);
		if (SDL_GetWindowWMInfo(m_SDL, nullptr)) {
			// Platform-specific handling can go here.
		}
#endif
		return out;
	}

	auto Window::fb_width() const -> uint32_t {
		int w, h;
		SDL_GetWindowSizeInPixels(m_SDL, &w, &h);
		return static_cast<uint32_t>(w);
	}

	auto Window::fb_height() const -> uint32_t {
		int w, h;
		SDL_GetWindowSizeInPixels(m_SDL, &w, &h);

		return static_cast<uint32_t>(h);
	}

	auto Window::fb_size() const -> std::pair<uint32_t, uint32_t> {
		int w, h;
		SDL_GetWindowSizeInPixels(m_SDL, &w, &h);

		return {
			static_cast<uint32_t>(w),
			static_cast<uint32_t>(h)
		};
	}

	auto Window::monitor() const -> const Monitor* {
		return m_Monitor.get();
	}

	auto Window::listeners() -> std::span<WindowListener> {
		return m_Listeners;
	}

	auto Window::focused() const -> bool {
		return SDL_GetWindowFlags(m_SDL) & SDL_WINDOW_INPUT_FOCUS;
	}

	auto Window::minimized() const -> bool {
		return SDL_GetWindowFlags(m_SDL) & SDL_WINDOW_MINIMIZED;
	}

	auto Window::maximized() const -> bool {
		return SDL_GetWindowFlags(m_SDL) & SDL_WINDOW_MAXIMIZED;
	}

	auto Window::visible() const -> bool {
		return SDL_GetWindowFlags(m_SDL) & SDL_WINDOW_HIDDEN
		         ? false
		         : true;
	}

	auto Window::fullscreened() -> bool {
		return SDL_GetWindowFlags(m_SDL) & SDL_WINDOW_FULLSCREEN;
	}

	auto Window::should_close() const -> bool {
		return bShouldClose;
	}

} // namespace aby::win::sdl

namespace aby::win::sdl::detail {

	auto to_key(SDL_Keycode key) -> EKey {
		switch (key) {
			case SDLK_A:
				return EKey::a;
			case SDLK_B:
				return EKey::b;
			case SDLK_C:
				return EKey::c;
			case SDLK_D:
				return EKey::d;
			case SDLK_E:
				return EKey::e;
			case SDLK_F:
				return EKey::f;
			case SDLK_G:
				return EKey::g;
			case SDLK_H:
				return EKey::h;
			case SDLK_I:
				return EKey::i;
			case SDLK_J:
				return EKey::j;
			case SDLK_K:
				return EKey::k;
			case SDLK_L:
				return EKey::l;
			case SDLK_M:
				return EKey::m;
			case SDLK_N:
				return EKey::n;
			case SDLK_O:
				return EKey::o;
			case SDLK_P:
				return EKey::p;
			case SDLK_Q:
				return EKey::q;
			case SDLK_R:
				return EKey::r;
			case SDLK_S:
				return EKey::s;
			case SDLK_T:
				return EKey::t;
			case SDLK_U:
				return EKey::u;
			case SDLK_V:
				return EKey::v;
			case SDLK_W:
				return EKey::w;
			case SDLK_X:
				return EKey::x;
			case SDLK_Y:
				return EKey::y;
			case SDLK_Z:
				return EKey::z;

			case SDLK_0:
				return EKey::num_0;
			case SDLK_1:
				return EKey::num_1;
			case SDLK_2:
				return EKey::num_2;
			case SDLK_3:
				return EKey::num_3;
			case SDLK_4:
				return EKey::num_4;
			case SDLK_5:
				return EKey::num_5;
			case SDLK_6:
				return EKey::num_6;
			case SDLK_7:
				return EKey::num_7;
			case SDLK_8:
				return EKey::num_8;
			case SDLK_9:
				return EKey::num_9;

			case SDLK_F1:
				return EKey::f1;
			case SDLK_F2:
				return EKey::f2;
			case SDLK_F3:
				return EKey::f3;
			case SDLK_F4:
				return EKey::f4;
			case SDLK_F5:
				return EKey::f5;
			case SDLK_F6:
				return EKey::f6;
			case SDLK_F7:
				return EKey::f7;
			case SDLK_F8:
				return EKey::f8;
			case SDLK_F9:
				return EKey::f9;
			case SDLK_F10:
				return EKey::f10;
			case SDLK_F11:
				return EKey::f11;
			case SDLK_F12:
				return EKey::f12;

			case SDLK_LSHIFT:
				return EKey::left_shift;
			case SDLK_RSHIFT:
				return EKey::right_shift;
			case SDLK_LCTRL:
				return EKey::left_ctrl;
			case SDLK_RCTRL:
				return EKey::right_ctrl;
			case SDLK_LALT:
				return EKey::left_alt;
			case SDLK_RALT:
				return EKey::right_alt;
			case SDLK_LGUI:
				return EKey::left_super;
			case SDLK_RGUI:
				return EKey::right_super;

			case SDLK_UP:
				return EKey::up;
			case SDLK_DOWN:
				return EKey::down;
			case SDLK_LEFT:
				return EKey::left;
			case SDLK_RIGHT:
				return EKey::right;

			case SDLK_HOME:
				return EKey::home;
			case SDLK_END:
				return EKey::end;
			case SDLK_PAGEUP:
				return EKey::page_up;
			case SDLK_PAGEDOWN:
				return EKey::page_down;
			case SDLK_INSERT:
				return EKey::insert;
			case SDLK_DELETE:
				return EKey::del;

			case SDLK_BACKSPACE:
				return EKey::backspace;
			case SDLK_RETURN:
				return EKey::enter;
			case SDLK_TAB:
				return EKey::tab;
			case SDLK_ESCAPE:
				return EKey::escape;
			case SDLK_SPACE:
				return EKey::space;

			case SDLK_APOSTROPHE:
				return EKey::apostrophe;
			case SDLK_COMMA:
				return EKey::comma;
			case SDLK_MINUS:
				return EKey::minus;
			case SDLK_PERIOD:
				return EKey::period;
			case SDLK_SLASH:
				return EKey::slash;
			case SDLK_SEMICOLON:
				return EKey::semicolon;
			case SDLK_EQUALS:
				return EKey::equal;
			case SDLK_LEFTBRACKET:
				return EKey::left_bracket;
			case SDLK_BACKSLASH:
				return EKey::backslash;
			case SDLK_RIGHTBRACKET:
				return EKey::right_bracket;
			case SDLK_GRAVE:
				return EKey::grave_accent;

			case SDLK_CAPSLOCK:
				return EKey::caps_lock;
			case SDLK_NUMLOCKCLEAR:
				return EKey::num_lock;
			case SDLK_SCROLLLOCK:
				return EKey::scroll_lock;

			case SDLK_KP_0:
				return EKey::kp_0;
			case SDLK_KP_1:
				return EKey::kp_1;
			case SDLK_KP_2:
				return EKey::kp_2;
			case SDLK_KP_3:
				return EKey::kp_3;
			case SDLK_KP_4:
				return EKey::kp_4;
			case SDLK_KP_5:
				return EKey::kp_5;
			case SDLK_KP_6:
				return EKey::kp_6;
			case SDLK_KP_7:
				return EKey::kp_7;
			case SDLK_KP_8:
				return EKey::kp_8;
			case SDLK_KP_9:
				return EKey::kp_9;

			case SDLK_KP_DECIMAL:
				return EKey::kp_decimal;
			case SDLK_KP_DIVIDE:
				return EKey::kp_divide;
			case SDLK_KP_MULTIPLY:
				return EKey::kp_multiply;
			case SDLK_KP_MINUS:
				return EKey::kp_subtract;
			case SDLK_KP_PLUS:
				return EKey::kp_add;
			case SDLK_KP_ENTER:
				return EKey::kp_enter;
			case SDLK_KP_EQUALS:
				return EKey::kp_equal;

			case SDLK_PRINTSCREEN:
				return EKey::print_screen;
			case SDLK_PAUSE:
				return EKey::pause;
			case SDLK_MENU:
				return EKey::menu;

			default:
				return EKey::unknown;
		}
	}

	auto to_mods(SDL_Keymod mods) -> EMod {
		EMod result = EMod::none;

		if (mods & SDL_KMOD_SHIFT)
			result |= EMod::shift;

		if (mods & SDL_KMOD_CTRL)
			result |= EMod::ctrl;

		if (mods & SDL_KMOD_ALT)
			result |= EMod::alt;

		if (mods & SDL_KMOD_GUI)
			result |= EMod::super;

		if (mods & SDL_KMOD_CAPS)
			result |= EMod::caps_lock;

		if (mods & SDL_KMOD_NUM)
			result |= EMod::num_lock;

		return result;
	}

	auto to_mouse_button(uint8_t button) -> EMouseButton {
		switch (button) {
			case SDL_BUTTON_LEFT:
				return EMouseButton::left;

			case SDL_BUTTON_RIGHT:
				return EMouseButton::right;

			case SDL_BUTTON_MIDDLE:
				return EMouseButton::middle;

			case SDL_BUTTON_X1:
				return EMouseButton::button_4;

			case SDL_BUTTON_X2:
				return EMouseButton::button_5;

			default:
				if (button >= 6 && button <= 8)
					return static_cast<EMouseButton>(
					    static_cast<uint8_t>(EMouseButton::button_4) + (button - SDL_BUTTON_X1));

				return EMouseButton::none;
		}
	}

	auto sdl_get_phys_dp_size(SDL_DisplayID id) -> std::pair<int32_t, int32_t> {
		const SDL_DisplayMode* mode;
		if (mode = SDL_GetCurrentDisplayMode(id); !mode) {
			aby_win_err("[sdl] failed to get display mode: {}", SDL_GetError());
			return std::make_pair(0, 0);
		}

		float dpi = SDL_GetDisplayContentScale(id);
		float win = mode->w / dpi;
		float hin = mode->h / dpi;
		return std::make_pair(static_cast<int32_t>(win), static_cast<int32_t>(hin));
	}

	auto sdl_get_current_monitor(SDL_Window* window) -> std::unique_ptr<Monitor> {
		if (SDL_DisplayID monitor = SDL_GetDisplayForWindow(window); monitor != 0) {
			const char* name        = SDL_GetDisplayName(monitor);
			float scale             = SDL_GetDisplayContentScale(monitor);
			int32_t display_mode_ct = 0;

			SDL_DisplayMode** modes = SDL_GetFullscreenDisplayModes(monitor, &display_mode_ct);
			std::vector<VideoMode> video_modes(display_mode_ct);
			size_t current_video_mode           = 0;
			const SDL_DisplayMode* current_mode = SDL_GetCurrentDisplayMode(monitor);

			for (int32_t i = 0; i < display_mode_ct; i++) {
				auto* mode                            = modes[i];
				const SDL_PixelFormatDetails* details = SDL_GetPixelFormatDetails(mode->format);

				if (mode->w == current_mode->w &&
				    mode->h == current_mode->h &&
				    mode->pixel_density == current_mode->pixel_density &&
				    mode->refresh_rate == current_mode->refresh_rate &&
				    mode->refresh_rate_denominator == current_mode->refresh_rate_denominator &&
				    mode->refresh_rate_numerator == current_mode->refresh_rate_numerator) {
					current_video_mode = static_cast<size_t>(i);
				}
				VideoMode m;
				m.size         = std::make_pair(mode->w, mode->h);
				m.refresh_rate = mode->refresh_rate;
				m.channel_bits = {
					details->Rbits,
					details->Gbits,
					details->Bbits
				};
				video_modes.push_back(m);
			}

			SDL_Rect bounds;
			if (!SDL_GetDisplayBounds(monitor, &bounds)) {
				aby_win_err("[sdl] failed to get the monitor bounds: {}", SDL_GetError());
				return nullptr;
			}

			return std::make_unique<Monitor>(
			    std::make_pair(bounds.x, bounds.y),
			    WorkArea{ .x = bounds.x, .y = bounds.y, .w = bounds.w, .h = bounds.h },
			    detail::sdl_get_phys_dp_size(monitor),
			    std::make_pair(scale, scale),
			    name ? name : "",
			    video_modes,
			    current_video_mode);
		} else {
			aby_win_err("[sdl] failed to get the monitor the window resides on: {}", SDL_GetError());
			return nullptr;
		}
	}

} // namespace aby::win::sdl::detail
