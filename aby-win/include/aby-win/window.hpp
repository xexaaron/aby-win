#pragma once
#include "event.hpp"
#include "monitor.hpp"

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

	/// @brief Loader independent icon structure.
	struct Icon {
		uint32_t width;              // the icon width. (16, 32, 48)
		uint32_t height;             // the icon height (16, 32, 48)
		std::span<std::byte> pixels; // the pixel data
	};

	/// @brief Used for custom undecorated windows
	struct HitTestConfig {
		uint32_t resize_border    = 8;  // px length of the resize border
		uint32_t title_bar_height = 32; // px length of the title bar height
	};

	/**
	 * @brief A Listener function
	 * @param Event& a polymorphic event object to be used with an EventDispatcher
	 * @return true to stop propogating the event to other listeners, otherwise false.
	 */
	using WindowListener = std::function<bool(Event&)>;

	struct Config {
		auto set_name(std::string_view name) -> Config&;
		auto set_width(uint32_t w) -> Config&;
		auto set_height(uint32_t h) -> Config&;
		auto set_theme(ETheme theme) -> Config&;
		auto set_resizable(bool resizable) -> Config&;
		auto set_visible(bool visible) -> Config&;
		auto set_decorated(bool decorated) -> Config&;
		auto set_focused(bool focused) -> Config&;
		auto set_window_backend(EWindow backend) -> Config&;
		auto set_render_backend(ERenderBackend backend) -> Config&;
		auto set_render_doc(bool render_doc) -> Config&;

		auto set_size(uint32_t w, uint32_t h) -> Config&;
		auto set_flags(bool resziable, bool visible, bool decorated, bool focused) -> Config&;
		auto set_backends(EWindow window_backend, ERenderBackend render_backend) -> Config&;

		std::string_view name         = "";                   // the title
		uint32_t width                = 800;                  // the initial width
		uint32_t height               = 600;                  // the initial height
		ETheme theme                  = ETheme::automatic;    // the initial theme
		bool resizable                = true;                 // is window resizing allowed.
		bool visible                  = true;                 // is the window initially visible.
		bool decorated                = true;                 // does the window have a title bar
		bool focused                  = true;                 // does the window start focused
		bool render_doc               = false;                // use x11 over wayland to support vulkan render doc
		EWindow window_backend        = EWindow::glfw;        // the windowing library
		ERenderBackend render_backend = ERenderBackend::none; // the rendering backend
	};

	class Window {
	protected:
		Window(const Config& config);
	public:
		/**
		 * @brief Create a window
		 * @param config The window configuration
		 * @return std::unique_ptr<Window>
		 */
		static auto create(const Config& config) -> std::unique_ptr<Window>;
		/**
		 * @brief Create a raw window ptr
		 * @param config The window configuration
		 * @return Window*
		 */
		static auto create_raw(const Config& config) -> Window*;
		/**
		 * @brief Create a unique ptr window
		 * @param config The window configuration
		 * @return std::unique_ptr<Window>
		 */
		static auto create_unique(const Config& config) -> std::unique_ptr<Window>;
		/**
		 * @brief Create a shared ptr window
		 * @param config The window configuraiton
		 * @return std::shared_ptr<Window>
		 */
		static auto create_shared(const Config& config) -> std::shared_ptr<Window>;
		virtual ~Window() = default;

		/**
		 * @brief Set the window title
		 * @param name The new title
		 */
		virtual auto set_name(std::string_view name) -> void               = 0;
		/**
		 * @brief Set the window width
		 * @param w The new width
		 */
		virtual auto set_width(uint32_t w) -> void                         = 0;
		/**
		 * @brief Set the window height
		 * @param h The new height
		 */
		virtual auto set_height(uint32_t h) -> void                        = 0;
		/**
		 * @brief Set the window size
		 * @param w The new width
		 * @param h The new height
		 */
		virtual auto set_size(uint32_t w, uint32_t h) -> void              = 0;
		/**
		 * @brief Set the window position
		 * @param x The new x position
		 * @param y The new y position
		 */
		virtual auto set_position(int32_t x, int32_t y) -> void            = 0;
		/**
		 * @brief Set the fullscreen mode
		 * @param fullscreen [true|false]
		 */
		virtual auto set_fullscreen(bool fullscreen) -> void               = 0;
		/**
		 * @brief Set the cursor mode
		 * @param mode [normal, hidden, disabled]
		 */
		virtual auto set_cursor_mode(ECursorMode mode) -> void             = 0;
		/**
		 * @brief Set the cursor position
		 * @param x The new x position
		 * @param y The new y position
		 */
		virtual auto set_cursor_pos(float x, float y) -> void              = 0;
		/**
		 * @brief Set the window decoration theme
		 * @param theme [dark|light|automatic]
		 */
		virtual auto set_theme(ETheme theme) -> void                       = 0;
		/**
		 * @brief Set the window icon
		 * @param icon a loaded image 
		 * @note The pixel data is expected to be in 32-bit RGBA format, 8 bits per channel.
		 */
		virtual auto set_icon(const Icon& icon) -> void                    = 0;
		/**
		 * @brief Set the hit test configuration for undecorated windows
		 * @param cfg the configuration
		 */
		virtual auto set_hit_test_config(const HitTestConfig& cfg) -> void = 0;
		/**
		 * @brief Add an event listener
		 * @param listener The new listener: [](Event&) -> bool
		 */
		virtual auto add_listener(WindowListener&& listener) -> void       = 0;

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
		virtual auto position() const -> std::pair<int32_t, int32_t>  = 0;
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
		 * @brief Get the current monitor that the window is *mostly* on
		*/
		virtual auto monitor() const -> const Monitor*                = 0;

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
