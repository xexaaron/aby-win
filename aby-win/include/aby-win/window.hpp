#pragma once
#include "event.hpp"
#include "monitor.hpp"

#include <functional>
#include <memory>

#if ABY_WIN_ENABLE_GLFW == 0 && ABY_WIN_ENABLE_SDL == 0 && ABY_WIN_ENABLE_QT == 0
#	error "no window backend enabled"
#endif

namespace aby::win::external {

	struct Interface;

}

namespace aby::win {

	enum class ERenderBackend {
		none,
		vulkan,
		d3d,
		metal,
		opengl,
	};

	enum class EWindow {
#if ABY_WIN_ENABLE_GLFW
		glfw,
#endif
#if ABY_WIN_ENABLE_SDL
		sdl,
#endif
#if ABY_WIN_ENABLE_QT
		qt
#endif
	};

	enum class EPlatform {
		wayland,
		x11,
		cocoa,
		winapi
	};

	enum class ECursorMode {
		normal,
		hidden,
		disabled
	};

	enum class ECursor {
		arrow,       // ↖    Default pointer
		ibeam,       // I    Text selection/editing
		crosshair,   // +    Precise selection
		hand,        // ☝   Clickable element
		hresize,     // ↔    Horizontal resize
		vresize,     // ↕    Vertical resize
		nwse_resize, // ⤡    Diagonal resize (NW ↔ SE)
		nesw_resize, // ⤢    Diagonal resize (NE ↔ SW)
		move,        // ✥    Move/drag
		not_allowed, // ⊘   Operation not permitted
	};

	enum class ETheme {
		light,
		dark,
		automatic,
	};

	struct NativeWindow {
		void* backend_window  = nullptr; // [GLFWwindow*|SDL_Window*]
		/**
		* @brief The native platform window.
		* @param win32[winapi] HWND
		* @param macos[cocoa] NSWindow*
		* @param linux[wayland] std::pair<wl_display*, wl_surface*>*
		* @param linux[x11] std::pair<Display*, Window>*
		*/
		void* platform_window = nullptr;
		EPlatform platform    = EPlatform::winapi; // [winapi|x11|wayland|cocoa]
		EWindow backend       = EWindow::glfw;     // [glfw|sdl|external]
	};

	/// @brief Loader independent icon structure.
	struct Icon {
		uint32_t width;              // the icon width. (16, 32, 48)
		uint32_t height;             // the icon height (16, 32, 48)
		std::span<std::byte> pixels; // the pixel data in 32-bit RGBA format, 8 bits per channel.
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

	struct ABY_WIN_API Config {
		/// @brief Set the initial window title
		auto set_name(std::string_view name) -> Config&;
		/// @brief Set the initial window width
		auto set_width(uint32_t w) -> Config&;
		/// @brief Set the initial window height
		auto set_height(uint32_t h) -> Config&;
		/// @brief Set the initial window theme
		auto set_theme(ETheme theme) -> Config&;
		/// @brief Set the window resizability capability
		auto set_resizable(bool resizable) -> Config&;
		/// @brief Set the window initial visbility state
		auto set_visible(bool visible) -> Config&;
		/// @brief Set the window initial decoration state
		auto set_decorated(bool decorated) -> Config&;
		/// @brief Set the window initial focused state
		auto set_focused(bool focused) -> Config&;
		/// @brief Set the window backend to use [sdl, glfw]
		auto set_window_backend(EWindow backend) -> Config&;
		/// @brief Set the renderer backend to use for the window
		auto set_render_backend(ERenderBackend backend) -> Config&;
		/// @brief Set the native window to use x11 over wayland on linux to be render doc compatible with vulkan
		auto set_render_doc(bool render_doc) -> Config&;
		/// @brief Set the flag that dictates if this window initializes the window backend
		auto set_child(bool child) -> Config&;
		/// @brief Set the initial window width and height
		auto set_size(uint32_t w, uint32_t h) -> Config&;
		/// @brief Set the common window flags
		auto set_flags(bool resziable, bool visible, bool decorated, bool focused) -> Config&;
		/// @brief Set the window backend and the renderer backend
		auto set_backends(EWindow window_backend, ERenderBackend render_backend) -> Config&;

		std::string_view name         = "";                   // the title
		uint32_t width                = 800;                  // the initial width
		uint32_t height               = 600;                  // the initial height
		ETheme theme                  = ETheme::automatic;    // the initial theme
		bool resizable                = true;                 // is window resizing allowed.
		bool visible                  = true;                 // is the window initially visible.
		bool decorated                = true;                 // does the window have a title bar
		bool focused                  = true;                 // does the window start focused
		bool render_doc               = false;                // use x11 over wayland to support vulkan render doc (for qt you must set this by env 'QT_QPA_PLATFORM=xcb')
		bool child                    = false;                // is the window the main window (the one that will initialize and deinitialize the window backend)
		EWindow window_backend        = EWindow::glfw;        // the windowing library
		ERenderBackend render_backend = ERenderBackend::none; // the rendering backend
	};

	class ABY_WIN_API Window {
	protected:
		Window(const Config& config);
	public:
		/**
		 * @brief Create a window
		 * @param[in] config The window configuration
		 * @return std::unique_ptr<Window>
		 */
		static auto create(const Config& config) -> std::unique_ptr<Window>;
		/**
		 * @brief Create a raw window ptr
		 * @param[in] config The window configuration
		 * @return Window*
		 */
		static auto create_raw(const Config& config) -> Window*;
		/**
		 * @brief Create a unique ptr window
		 * @param[in] config The window configuration
		 * @return std::unique_ptr<Window>
		 */
		static auto create_unique(const Config& config) -> std::unique_ptr<Window>;
		/**
		 * @brief Create a shared ptr window
		 * @param[in] config The window configuraiton
		 * @return std::shared_ptr<Window>
		 */
		static auto create_shared(const Config& config) -> std::shared_ptr<Window>;

		virtual ~Window()                                                  = default;
		/**
		 * @brief Set the window title
		 * @param[in] name The new title
		 */
		virtual auto set_name(std::string_view name) -> void               = 0;
		/**
		 * @brief Set the window width
		 * @param[in] w The new width
		 */
		virtual auto set_width(uint32_t w) -> void                         = 0;
		/**
		 * @brief Set the window height
		 * @param[in] h The new height
		 */
		virtual auto set_height(uint32_t h) -> void                        = 0;
		/**
		 * @brief Set the window size
		 * @param[in] w The new width
		 * @param[in] h The new height
		 */
		virtual auto set_size(uint32_t w, uint32_t h) -> void              = 0;
		/**
		 * @brief Set the window position
		 * @param[in] x The new x position
		 * @param[in] y The new y position
		 */
		virtual auto set_position(int32_t x, int32_t y) -> void            = 0;
		/**
		 * @brief Set the fullscreen mode
		 * @param[in] fullscreen [true|false]
		 */
		virtual auto set_fullscreen(bool fullscreen) -> void               = 0;
		/**
		 * @brief Set the cursor mode
		 * @param[in] mode [normal, hidden, disabled]
		 */
		virtual auto set_cursor_mode(ECursorMode mode) -> void             = 0;
		/**
		 * @brief Set the cursor position
		 * @param[in] x The new x position
		 * @param[in] y The new y position
		 */
		virtual auto set_cursor_pos(float x, float y) -> void              = 0;
		/**
		 * @brief Set the cursor shape
		 * @param[in] cursor the cursor shape
		 */
		virtual auto set_cursor(ECursor cursor) -> void                    = 0;
		/**
		 * @brief Set the window decoration theme
		 * @param[in] theme [dark|light|automatic]
		 */
		virtual auto set_theme(ETheme theme) -> void                       = 0;
		/**
		 * @brief Set the window icon
		 * @param[in] icon a loaded image 
		 * @note The pixel data is expected to be in 32-bit RGBA format, 8 bits per channel.
		 */
		virtual auto set_icon(const Icon& icon) -> void                    = 0;
		/**
		 * @brief Set the hit test configuration for undecorated windows
		 * @param[in] cfg the configuration
		 */
		virtual auto set_hit_test_config(const HitTestConfig& cfg) -> void = 0;
		/**
		* @brief Set the system clipboard
		* @param[in] text the text to set the clipboard to
		*/
		virtual auto set_clipboard(std::string_view text) -> void          = 0;
		/**
		* @brief Set the window resizability flag
		* @param[in] value [true|false]
		*/
		virtual auto set_resizable(bool value) -> void                     = 0;
		/**
		* @brief Set the window visbility flag
		* @param[in] value [true|false]
		*/
		virtual auto set_visible(bool value) -> void                       = 0;
		/**
		* @brief Set the window decorated flag
		* @param[in] value [true|false]
		*/
		virtual auto set_decorated(bool value) -> void                     = 0;
		/**
		* @brief Set the window focused flag
		* @param[in] value [true|false]
		*/
		virtual auto set_focused(bool value) -> void                       = 0;
		/**
		 * @brief Add an event listener
		 * @param[in] listener The new listener: [](Event&) -> bool
		 */
		virtual auto add_listener(WindowListener&& listener) -> void       = 0;
		/**
		 * @brief Set the window as the top window and then focus it for input
		 */
		virtual auto focus() -> void                                       = 0;
		/**
		 * @brief Minimize the window
		 */
		virtual auto minimize() -> void                                    = 0;
		/**
		 * @brief Maximize the window
		 */
		virtual auto maximize() -> void                                    = 0;
		/**
		 * @brief Show the window if hidden
		 */
		virtual auto show() -> void                                        = 0;
		/**
		 * @brief Hide the window if shown
		 */
		virtual auto hide() -> void                                        = 0;
		/**
		 * @brief Tell the window it should close on the next frame
		 */
		virtual auto close() -> void                                       = 0;
		/**
		 * @brief Poll all pending events for all windows (blocking)
		 * @warning This should ONLY be called once per frame by the MAIN window.
		 * 			Each window will process events via their listeners.
		 */
		virtual auto poll() -> void                                        = 0;
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
		* @brief Get the window ID
		*/
		virtual auto id() const -> uint32_t                           = 0;
		/**
		* @brief Get the system clipboard text
		*/
		virtual auto clipboard() const -> std::string_view            = 0;
		/**
		 * @brief Check if the window is focused
		 */
		virtual auto focused() const -> bool                          = 0;
		/**
		 * @brief Check if the window is minimized
		 */
		virtual auto minimized() const -> bool                        = 0;
		/**
		 * @brief Check if the window is maximized
		 */
		virtual auto maximized() const -> bool                        = 0;
		/**
		 * @brief Check if the window is visible
		 */
		virtual auto visible() const -> bool                          = 0;
		/**
		 * @brief Check if the window is in fullscreen mode
		 */
		virtual auto fullscreened() -> bool                           = 0;
		/**
		 * @brief Check if the window should close
		 */
		virtual auto should_close() const -> bool                     = 0;
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
				case aby::win::EWindow::qt:
					name = "qt";
					break;
			}

			return formatter<std::string_view>::format(name, ctx);
		}
	};

	template <>
	struct formatter<aby::win::EPlatform> : formatter<std::string_view> {
		auto format(aby::win::EPlatform value, format_context& ctx) const {
			std::string_view name;

			switch (value) {
				case aby::win::EPlatform::winapi:
					name = "winapi";
					break;
				case aby::win::EPlatform::x11:
					name = "x11";
					break;
				case aby::win::EPlatform::wayland:
					name = "wayland";
				case aby::win::EPlatform::cocoa:
					name = "cocoa";
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
