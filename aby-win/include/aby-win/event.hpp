#pragma once
#include "common.hpp"
#include "inline/event.inl"

#include <filesystem>
#include <format>

namespace aby::win {

	enum class EEventCategory {
		none,
		keyboard,
		mouse,
		window,
		io,
		monitor,
		joystick
	};

	enum class EEvent {
		none,
		native, /* events that arent handled by this abstraction but are passed through so that other apis might handle them */
		key_pressed,
		key_released,
		key_typed,
		mouse_moved,
		mouse_pressed,
		mouse_released,
		mouse_entered,
		mouse_left,
		mouse_scrolled,
		window_moved,
		window_resized,
		window_closed,
		window_refreshed,
		window_focused,
		window_minimized,
		window_maximized,
		window_fb_resized,
		window_scaled,
		io_file_dropped,
		monitor_connected,
		monitor_disconnected,
		joystick_connected,
		joystick_disconnected,
	};

	enum class EKey : u16 {
		unknown = 0,

		// Letters
		a,
		b,
		c,
		d,
		e,
		f,
		g,
		h,
		i,
		j,
		k,
		l,
		m,
		n,
		o,
		p,
		q,
		r,
		s,
		t,
		u,
		v,
		w,
		x,
		y,
		z,

		// Numbers
		num_0,
		num_1,
		num_2,
		num_3,
		num_4,
		num_5,
		num_6,
		num_7,
		num_8,
		num_9,

		// Function
		f1,
		f2,
		f3,
		f4,
		f5,
		f6,
		f7,
		f8,
		f9,
		f10,
		f11,
		f12,

		// Modifiers
		left_shift,
		right_shift,
		left_ctrl,
		right_ctrl,
		left_alt,
		right_alt,
		left_super,
		right_super,

		// Navigation
		up,
		down,
		left,
		right,

		home,
		end,
		page_up,
		page_down,

		insert,
		del,

		// Editing
		backspace,
		enter,
		tab,
		escape,
		space,

		// Punctuation
		apostrophe,
		comma,
		minus,
		period,
		slash,
		semicolon,
		equal,
		left_bracket,
		backslash,
		right_bracket,
		grave_accent,

		// Locks
		caps_lock,
		num_lock,
		scroll_lock,

		// Numpad
		kp_0,
		kp_1,
		kp_2,
		kp_3,
		kp_4,
		kp_5,
		kp_6,
		kp_7,
		kp_8,
		kp_9,

		kp_decimal,
		kp_divide,
		kp_multiply,
		kp_subtract,
		kp_add,
		kp_enter,
		kp_equal,

		print_screen,
		pause,

		menu,

		count
	};

	enum class EMouseButton : u8 {
		none = 0,
		left,
		right,
		middle,
		button_4,
		button_5,
		button_6,
		button_7,
		button_8,
		count
	};

	enum class EMod : u8 {
		none      = 0,
		shift     = 1 << 0,
		ctrl      = 1 << 1,
		alt       = 1 << 2,
		super     = 1 << 3,
		caps_lock = 1 << 4,
		num_lock  = 1 << 5,
	};

	class Event {
	public:
		static auto static_type() -> EEvent {
			return EEvent::none;
		}

		virtual auto type() const -> EEvent {
			return EEvent::none;
		}

		virtual auto name() const -> std::string_view {
			return "Event";
		}

		virtual auto category() const -> EEventCategory {
			return EEventCategory::none;
		}
	};

	class EventDispatcher {
	public:
		explicit EventDispatcher(Event& event) :
		    m_Event(event) {
		}

		template <typename F>
		auto dispatch(F&& callback) -> bool {
			using Arg = detail::first_argument_t<F>;
			using T   = std::remove_cvref_t<Arg>;

			static_assert(std::derived_from<T, Event>, "EventDispatcher callback must take an Event-derived reference");

			if (m_Event.type() != T::static_type())
				return false;

			std::invoke(std::forward<F>(callback), static_cast<T&>(m_Event));

			return true;
		}

		template <typename F, typename O>
		auto dispatch(F&& callback, O* object) -> bool {
			using Arg = detail::first_argument_t<F>;
			using T   = std::remove_cvref_t<Arg>;

			static_assert(std::derived_from<T, Event>, "callback argument must derive from Event");

			if (m_Event.type() != T::static_type())
				return false;

			std::invoke(std::forward<F>(callback), object, static_cast<T&>(m_Event));

			return true;
		}
	private:
		Event& m_Event;
	};

	ABY_WIN_DECLARE_EVENT(
	    Native, native, none,
	    {
		    void* event;
	    });

	ABY_WIN_DECLARE_EVENT(
	    KeyPressed, key_pressed, keyboard,
	    {
		    EKey key;
		    EMod mods;
	    });

	ABY_WIN_DECLARE_EVENT(
	    KeyReleased, key_released, keyboard,
	    {
		    EKey key;
		    EMod mods;
	    });

	ABY_WIN_DECLARE_EVENT(
	    KeyTyped, key_typed, keyboard,
	    {
		    char32_t codepoint;
	    });

	ABY_WIN_DECLARE_EVENT(
	    MouseMoved, mouse_moved, mouse,
	    {
		    float x;
		    float y;
	    });

	ABY_WIN_DECLARE_EVENT(
	    MousePressed, mouse_pressed, mouse,
	    {
		    EMouseButton button;
		    EMod mods;
	    });

	ABY_WIN_DECLARE_EVENT(
	    MouseReleased, mouse_released, mouse,
	    {
		    EMouseButton button;
		    EMod mods;
	    });

	ABY_WIN_DECLARE_EMPTY_EVENT(MouseEntered, mouse_entered, mouse);

	ABY_WIN_DECLARE_EMPTY_EVENT(MouseLeft, mouse_left, mouse);

	ABY_WIN_DECLARE_EVENT(
	    MouseScrolled, mouse_scrolled, mouse,
	    {
		    float x;
		    float y;
	    });

	ABY_WIN_DECLARE_EVENT(
	    WindowMoved, window_moved, window,
	    {
		    i32 x;
		    i32 y;
	    });

	ABY_WIN_DECLARE_EVENT(
	    WindowResized, window_resized, window,
	    {
		    u32 width;
		    u32 height;
	    });

	ABY_WIN_DECLARE_EMPTY_EVENT(WindowClosed, window_closed, window);

	ABY_WIN_DECLARE_EMPTY_EVENT(WindowRefreshed, window_refreshed, window);

	ABY_WIN_DECLARE_EVENT(
	    WindowFocused, window_focused, window,
	    {
		    bool focused;
	    });

	ABY_WIN_DECLARE_EVENT(
	    WindowMinimized, window_minimized, window,
	    {
		    bool minimized;
	    });

	ABY_WIN_DECLARE_EVENT(
	    WindowMaximized, window_maximized, window,
	    {
		    bool maximized;
	    });

	ABY_WIN_DECLARE_EVENT(
	    WindowFramebufferResized, window_fb_resized, window,
	    {
		    u32 width;
		    u32 height;
	    });

	ABY_WIN_DECLARE_EVENT(
	    WindowScaled, window_scaled, window,
	    {
		    float x;
		    float y;
	    });

	ABY_WIN_DECLARE_EVENT(
	    FileDropped, io_file_dropped, io,
	    {
		    std::filesystem::path path;
	    });

	ABY_WIN_DECLARE_EVENT(
	    MonitorConnected, monitor_connected, monitor,
	    {
		    u32 monitor;
	    });

	ABY_WIN_DECLARE_EVENT(
	    MonitorDisconnected, monitor_disconnected, monitor,
	    {
		    u32 monitor;
	    });

	ABY_WIN_DECLARE_EVENT(
	    JoystickConnected, joystick_connected, joystick,
	    {
		    u32 joystick;
	    });

	ABY_WIN_DECLARE_EVENT(
	    JoystickDisconnected, joystick_disconnected, joystick,
	    {
		    u32 joystick;
	    });

} // namespace aby::win

namespace std {

	template <>
	struct std::formatter<aby::win::EEventCategory> : std::formatter<std::string_view> {
		auto format(aby::win::EEventCategory value, std::format_context& ctx) const {
			using enum aby::win::EEventCategory;

			std::string_view name = [value] {
				switch (value) {
					case none:
						return "none";
					case keyboard:
						return "keyboard";
					case mouse:
						return "mouse";
					case window:
						return "window";
					case io:
						return "io";
					case monitor:
						return "monitor";
					case joystick:
						return "joystick";
				}

				return "unknown";
			}();

			return std::formatter<std::string_view>::format(name, ctx);
		}
	};

	template <>
	struct std::formatter<aby::win::EEvent> : std::formatter<std::string_view> {
		auto format(aby::win::EEvent value, std::format_context& ctx) const {
			using enum aby::win::EEvent;

			std::string_view name = [value] {
				switch (value) {
					case none:
						return "none";
					case native:
						return "native";
					case key_pressed:
						return "key_pressed";
					case key_released:
						return "key_released";
					case key_typed:
						return "key_typed";
					case mouse_moved:
						return "mouse_moved";
					case mouse_pressed:
						return "mouse_pressed";
					case mouse_released:
						return "mouse_released";
					case mouse_entered:
						return "mouse_entered";
					case mouse_left:
						return "mouse_left";
					case mouse_scrolled:
						return "mouse_scrolled";
					case window_moved:
						return "window_moved";
					case window_resized:
						return "window_resized";
					case window_closed:
						return "window_closed";
					case window_refreshed:
						return "window_refreshed";
					case window_focused:
						return "window_focused";
					case window_minimized:
						return "window_minimized";
					case window_maximized:
						return "window_maximized";
					case window_fb_resized:
						return "window_fb_resized";
					case window_scaled:
						return "window_scaled";
					case io_file_dropped:
						return "io_file_dropped";
					case monitor_connected:
						return "monitor_connected";
					case monitor_disconnected:
						return "monitor_disconnected";
					case joystick_connected:
						return "joystick_connected";
					case joystick_disconnected:
						return "joystick_disconnected";
				}

				return "unknown";
			}();

			return std::formatter<std::string_view>::format(name, ctx);
		}
	};

	template <>
	struct std::formatter<aby::win::EKey> : std::formatter<std::string_view> {
		auto format(aby::win::EKey value, std::format_context& ctx) const {
			using enum aby::win::EKey;

			std::string_view name = [value] {
				switch (value) {
					case unknown:
						return "unknown";

					case a:
						return "a";
					case b:
						return "b";
					case c:
						return "c";
					case d:
						return "d";
					case e:
						return "e";
					case f:
						return "f";
					case g:
						return "g";
					case h:
						return "h";
					case i:
						return "i";
					case j:
						return "j";
					case k:
						return "k";
					case l:
						return "l";
					case m:
						return "m";
					case n:
						return "n";
					case o:
						return "o";
					case p:
						return "p";
					case q:
						return "q";
					case r:
						return "r";
					case s:
						return "s";
					case t:
						return "t";
					case u:
						return "u";
					case v:
						return "v";
					case w:
						return "w";
					case x:
						return "x";
					case y:
						return "y";
					case z:
						return "z";

					case num_0:
						return "0";
					case num_1:
						return "1";
					case num_2:
						return "2";
					case num_3:
						return "3";
					case num_4:
						return "4";
					case num_5:
						return "5";
					case num_6:
						return "6";
					case num_7:
						return "7";
					case num_8:
						return "8";
					case num_9:
						return "9";

					case f1:
						return "f1";
					case f2:
						return "f2";
					case f3:
						return "f3";
					case f4:
						return "f4";
					case f5:
						return "f5";
					case f6:
						return "f6";
					case f7:
						return "f7";
					case f8:
						return "f8";
					case f9:
						return "f9";
					case f10:
						return "f10";
					case f11:
						return "f11";
					case f12:
						return "f12";

					case left_shift:
						return "left_shift";
					case right_shift:
						return "right_shift";
					case left_ctrl:
						return "left_ctrl";
					case right_ctrl:
						return "right_ctrl";
					case left_alt:
						return "left_alt";
					case right_alt:
						return "right_alt";
					case left_super:
						return "left_super";
					case right_super:
						return "right_super";

					case up:
						return "up";
					case down:
						return "down";
					case left:
						return "left";
					case right:
						return "right";

					case home:
						return "home";
					case end:
						return "end";
					case page_up:
						return "page_up";
					case page_down:
						return "page_down";
					case insert:
						return "insert";
					case del:
						return "delete";

					case backspace:
						return "backspace";
					case enter:
						return "enter";
					case tab:
						return "tab";
					case escape:
						return "escape";
					case space:
						return "space";

					case apostrophe:
						return "'";
					case comma:
						return ",";
					case minus:
						return "-";
					case period:
						return ".";
					case slash:
						return "/";
					case semicolon:
						return ";";
					case equal:
						return "=";
					case left_bracket:
						return "[";
					case backslash:
						return "\\";
					case right_bracket:
						return "]";
					case grave_accent:
						return "`";

					case caps_lock:
						return "caps_lock";
					case num_lock:
						return "num_lock";
					case scroll_lock:
						return "scroll_lock";

					case kp_0:
						return "kp_0";
					case kp_1:
						return "kp_1";
					case kp_2:
						return "kp_2";
					case kp_3:
						return "kp_3";
					case kp_4:
						return "kp_4";
					case kp_5:
						return "kp_5";
					case kp_6:
						return "kp_6";
					case kp_7:
						return "kp_7";
					case kp_8:
						return "kp_8";
					case kp_9:
						return "kp_9";

					case kp_decimal:
						return "kp_decimal";
					case kp_divide:
						return "kp_divide";
					case kp_multiply:
						return "kp_multiply";
					case kp_subtract:
						return "kp_subtract";
					case kp_add:
						return "kp_add";
					case kp_enter:
						return "kp_enter";
					case kp_equal:
						return "kp_equal";

					case print_screen:
						return "print_screen";
					case pause:
						return "pause";
					case menu:
						return "menu";

					case count:
						return "count";
				}

				return "unknown";
			}();

			return std::formatter<std::string_view>::format(name, ctx);
		}
	};

	template <>
	struct std::formatter<aby::win::EMouseButton>
	    : std::formatter<std::string_view> {
		auto format(aby::win::EMouseButton value, std::format_context& ctx) const {
			using enum aby::win::EMouseButton;

			std::string_view name = [value] {
				switch (value) {
					case none:
						return "none";
					case left:
						return "left";
					case right:
						return "right";
					case middle:
						return "middle";
					case button_4:
						return "button_4";
					case button_5:
						return "button_5";
					case button_6:
						return "button_6";
					case button_7:
						return "button_7";
					case button_8:
						return "button_8";
					case count:
						return "count";
				}

				return "unknown";
			}();

			return std::formatter<std::string_view>::format(name, ctx);
		}
	};

	template <>
	struct std::formatter<aby::win::EMod> : std::formatter<std::string_view> {
		auto format(aby::win::EMod value, std::format_context& ctx) const {
			using namespace aby::win;

			const auto bits = static_cast<u8>(value);

			if (bits == 0)
				return std::formatter<std::string_view>::format("none", ctx);

			std::string out;

			auto append = [&](std::string_view name, EMod mod) {
				if ((bits & static_cast<u8>(mod)) == 0)
					return;

				if (!out.empty())
					out += '|';

				out += name;
			};

			append("shift", EMod::shift);
			append("ctrl", EMod::ctrl);
			append("alt", EMod::alt);
			append("super", EMod::super);
			append("caps_lock", EMod::caps_lock);
			append("num_lock", EMod::num_lock);

			return std::formatter<std::string_view>::format(out, ctx);
		}
	};

} // namespace std
