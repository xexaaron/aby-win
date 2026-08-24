#pragma once
#include "event.hpp"

#include <functional>
#include <memory>

namespace aby::win {

	enum class ERenderBackend {
		none,
		vulkan,
		d3d,
		metal,
		opengl,
	};

	enum class EWindow {
		glfw,
		sdl
	};

	enum class ECursorMode {
		normal,
		hidden,
		disabled
	};

	enum class ETheme {
		light,
		dark,
		automatic,
	};

	struct NativeWindow {
		void* backend_window;  // GLFWwindow*, SDL_Window*
		void* platform_window; // HWND, NSWindow, wl_surface*/XID
		EWindow backend;       // glfw, sdl
	};

	/**
	 * @brief A Listener function
	 * @param Event& a polymorphic event object to be used with an EventDispatcher
	 * @return true to stop propogating the event to other listeners, otherwise false.
	 */
	using WindowListener = std::function<bool(Event&)>;

	class Window {
	protected:
		Window(EWindow window_backend, std::string_view name, uint32_t w, uint32_t h, ERenderBackend render_backend, ETheme theme);
	public:
		/**
		 * @brief Create a window
		 * @param window_backend The type of window to create [sdl, glfw]
		 * @param name The window title
		 * @param w The initial window width
		 * @param h The initial window height
		 * @param render_backend The renderer backend to provide hints to the window backend
		 * @param theme [dark, light, automatic]
		 * @return std::unique_ptr<Window>
		 */
		static auto create(EWindow window_backend, std::string_view name, uint32_t w, uint32_t h, ERenderBackend render_backend, ETheme theme = ETheme::automatic) -> std::unique_ptr<Window>;
		/**
		 * @brief Create a window
		 * @param window_backend The type of window to create [sdl, glfw]
		 * @param name The window title
		 * @param w The initial window width
		 * @param h The initial window height
		 * @param render_backend The renderer backend to provide hints to the window backend
		 * @param theme [dark, light, automatic]
		 * @return Window*
		 */
		static auto create_raw(EWindow window_backend, std::string_view name, uint32_t w, uint32_t h, ERenderBackend render_backend, ETheme theme = ETheme::automatic) -> Window*;
		/**
		 * @brief Create a window
		 * @param window_backend The type of window to create [sdl, glfw]
		 * @param name The window title
		 * @param w The initial window width
		 * @param h The initial window height
		 * @param render_backend The renderer backend to provide hints to the window backend
		 * @param theme [dark, light, automatic]
		 * @return std::unique_ptr<Window>
		 */
		static auto create_unique(EWindow window_backend, std::string_view name, uint32_t w, uint32_t h, ERenderBackend render_backend, ETheme theme = ETheme::automatic) -> std::unique_ptr<Window>;
		/**
		 * @brief Create a window
		 * @param window_backend The type of window to create [sdl, glfw]
		 * @param name The window title
		 * @param w The initial window width
		 * @param h The initial window height
		 * @param render_backend The renderer backend to provide hints to the window backend
		 * @param theme [dark, light, automatic]
		 * @return std::shared_ptr<Window>
		 */
		static auto create_shared(EWindow window_backend, std::string_view name, uint32_t w, uint32_t h, ERenderBackend render_backend, ETheme theme = ETheme::automatic) -> std::shared_ptr<Window>;
		virtual ~Window() = default;

		/**
		 * @brief Set the window title
		 * @param name The new title
		 */
		virtual auto set_name(std::string_view name) -> void         = 0;
		/**
		 * @brief Set the window width
		 * @param w The new width
		 */
		virtual auto set_width(uint32_t w) -> void                   = 0;
		/**
		 * @brief Set the window height
		 * @param h The new height
		 */
		virtual auto set_height(uint32_t h) -> void                  = 0;
		/**
		 * @brief Set the window size
		 * @param w The new width
		 * @param h The new height
		 */
		virtual auto set_size(uint32_t w, uint32_t h) -> void        = 0;
		/**
		 * @brief Set the window position
		 * @param x The new x position
		 * @param y The new y position
		 */
		virtual auto set_position(int32_t x, int32_t y) -> void              = 0;
		/**
		 * @brief Set the fullscreen mode
		 * @param fullscreen [true|false]
		 */
		virtual auto set_fullscreen(bool fullscreen) -> void         = 0;
		/**
		 * @brief Set the cursor mode
		 * @param mode [normal, hidden, disabled]
		 */
		virtual auto set_cursor_mode(ECursorMode mode) -> void       = 0;
		/**
		 * @brief Set the cursor position
		 * @param x The new x position
		 * @param y The new y position
		 */
		virtual auto set_cursor_pos(float x, float y) -> void        = 0;
		/**
		 * @brief Set the window decoration theme
		 * @param theme [dark|light|automatic]
		 */
		virtual auto set_theme(ETheme theme) -> void                 = 0;
		/**
		 * @brief Add an event listener
		 * @param listener The new listener: [](Event&) -> bool
		 */
		virtual auto add_listener(WindowListener&& listener) -> void = 0;

		/**
		 * @brief Set the window as the top window and then focus it for input
		 */
		virtual auto focus() -> void    = 0;
		/**
		 * @brief Minimize the window
		 */
		virtual auto minimize() -> void = 0;
		/**
		 * @brief Maximize the window
		 */
		virtual auto maximize() -> void = 0;
		/**
		 * @brief Show the window if hidden
		 */
		virtual auto show() -> void     = 0;
		/**
		 * @brief Hide the window if shown
		 */
		virtual auto hide() -> void     = 0;
		/**
		 * @brief Tell the window it should close on the next frame
		 */
		virtual auto close() -> void    = 0;
		/**
		 * @brief Poll all pending events (blocking)
		 */
		virtual auto poll() -> void     = 0;

		/**
		 * @brief Get the window title
		 */
		auto name() const -> std::string_view;
		/**
		 * @brief Get the window theme
		 */
		auto theme() const -> ETheme;
		/**
		 * @brief Get the window width
		 */
		virtual auto width() const -> uint32_t                        = 0;
		/**
		 * @brief Get the window height
		 */
		virtual auto height() const -> uint32_t                       = 0;
		/**
		 * @brief Get the window size
		 */
		virtual auto size() const -> std::pair<uint32_t, uint32_t>    = 0;
		/**
		 * @brief Get the window position
		 */
		virtual auto position() const -> std::pair<int32_t, int32_t>          = 0;
		/**
		 * @brief Get the window display content scale
		 */
		virtual auto scale() const -> float                           = 0;
		/**
		 * @brief Get the native backend & platform handles
		 */
		virtual auto native() const -> NativeWindow                   = 0;
		/**
		 * @brief Get the pixel width of the window
		 */
		virtual auto fb_width() const -> uint32_t                     = 0;
		/**
		 * @brief Get the pixel height of the window
		 */
		virtual auto fb_height() const -> uint32_t                    = 0;
		/**
		 * @brief Get the pixel size of the window
		 */
		virtual auto fb_size() const -> std::pair<uint32_t, uint32_t> = 0;

		/**
		 * @brief Check if the window is focused
		 */
		virtual auto focused() const -> bool      = 0;
		/**
		 * @brief Check if the window is minimized
		 */
		virtual auto minimized() const -> bool    = 0;
		/**
		 * @brief Check if the window is maximized
		 */
		virtual auto maximized() const -> bool    = 0;
		/**
		 * @brief Check if the window is visible
		 */
		virtual auto visible() const -> bool      = 0;
		/**
		 * @brief Check if the window is in fullscreen mode
		 */
		virtual auto fullscreened() -> bool       = 0;
		/**
		 * @brief Check if the window should close
		 */
		virtual auto should_close() const -> bool = 0;
	protected:
		std::string m_Name;
		ERenderBackend m_RenderBackend;
		EWindow m_WindowBackend;
		ETheme m_Theme;
	};

} // namespace aby::win

namespace std {

	template <>
	struct formatter<aby::win::ERenderBackend> : formatter<std::string_view> {
		auto format(aby::win::ERenderBackend value, format_context& ctx) const {
			std::string_view name;

			switch (value) {
				case aby::win::ERenderBackend::none:
					name = "none";
					break;
				case aby::win::ERenderBackend::vulkan:
					name = "vulkan";
					break;
				case aby::win::ERenderBackend::d3d:
					name = "d3d";
					break;
				case aby::win::ERenderBackend::metal:
					name = "metal";
					break;
				case aby::win::ERenderBackend::opengl:
					name = "opengl";
					break;
			}

			return formatter<std::string_view>::format(name, ctx);
		}
	};

	template <>
	struct formatter<aby::win::EWindow> : formatter<std::string_view> {
		auto format(aby::win::EWindow value, format_context& ctx) const {
			std::string_view name;

			switch (value) {
				case aby::win::EWindow::glfw:
					name = "glfw";
					break;
				case aby::win::EWindow::sdl:
					name = "sdl";
					break;
			}

			return formatter<std::string_view>::format(name, ctx);
		}
	};

	template <>
	struct formatter<aby::win::ECursorMode> : formatter<std::string_view> {
		auto format(aby::win::ECursorMode value, format_context& ctx) const {
			std::string_view name;

			switch (value) {
				case aby::win::ECursorMode::normal:
					name = "normal";
					break;
				case aby::win::ECursorMode::hidden:
					name = "hidden";
					break;
				case aby::win::ECursorMode::disabled:
					name = "disabled";
					break;
			}

			return formatter<std::string_view>::format(name, ctx);
		}
	};

	template <>
	struct formatter<aby::win::ETheme> : formatter<std::string_view> {
		auto format(aby::win::ETheme value, format_context& ctx) const {
			std::string_view name;

			switch (value) {
				case aby::win::ETheme::light:
					name = "light";
					break;
				case aby::win::ETheme::dark:
					name = "dark";
					break;
				case aby::win::ETheme::automatic:
					name = "automatic";
					break;
			}

			return formatter<std::string_view>::format(name, ctx);
		}
	};

} // namespace std
