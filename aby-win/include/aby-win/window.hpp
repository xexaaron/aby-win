#pragma once
#include "common.hpp"

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
		EWindow backend;
	};

	class Window {
	protected:
		Window(EWindow window_backend, std::string_view name, u32 w, u32 h, ERenderBackend render_backend, ETheme theme);
	public:
		static auto create(EWindow window_backend, std::string_view name, u32 w, u32 h, ERenderBackend render_backend, ETheme theme = ETheme::automatic) -> std::unique_ptr<Window>;
		static auto create_raw(EWindow window_backend, std::string_view name, u32 w, u32 h, ERenderBackend render_backend, ETheme theme = ETheme::automatic) -> Window*;
		static auto create_unique(EWindow window_backend, std::string_view name, u32 w, u32 h, ERenderBackend render_backend, ETheme theme = ETheme::automatic) -> std::unique_ptr<Window>;
		static auto create_shared(EWindow window_backend, std::string_view name, u32 w, u32 h, ERenderBackend render_backend, ETheme theme = ETheme::automatic) -> std::shared_ptr<Window>;
		virtual ~Window() = default;

		virtual auto set_name(std::string_view name) -> void   = 0;
		virtual auto set_width(u32 w) -> void                  = 0;
		virtual auto set_height(u32 h) -> void                 = 0;
		virtual auto set_size(u32 w, u32 h) -> void            = 0;
		virtual auto set_cursor_mode(ECursorMode mode) -> void = 0;
		virtual auto set_cursor_pos(float x, float y) -> void  = 0;
		virtual auto set_theme(ETheme theme) -> void           = 0;

		virtual auto focus() -> void    = 0;
		virtual auto minimize() -> void = 0;
		virtual auto maximize() -> void = 0;
		virtual auto show() -> void     = 0;
		virtual auto close() -> void    = 0;
		virtual auto poll() -> void     = 0;

		auto name() const -> std::string_view;
		auto theme() const -> ETheme;
		virtual auto width() const -> u32                   = 0;
		virtual auto height() const -> u32                  = 0;
		virtual auto size() const -> std::pair<u32, u32>    = 0;
		virtual auto scale() const -> float                 = 0;
		virtual auto native() const -> NativeWindow         = 0;
		virtual auto fb_width() const -> u32                = 0;
		virtual auto fb_height() const -> u32               = 0;
		virtual auto fb_size() const -> std::pair<u32, u32> = 0;

		virtual auto focused() const -> bool      = 0;
		virtual auto minimized() const -> bool    = 0;
		virtual auto maximized() const -> bool    = 0;
		virtual auto visible() const -> bool      = 0;
		virtual auto should_close() const -> bool = 0;
	protected:
		std::string m_Name;
		ERenderBackend m_RenderBackend;
		EWindow m_WindowBackend;
		ETheme m_Theme;
	};

} // namespace aby::win
