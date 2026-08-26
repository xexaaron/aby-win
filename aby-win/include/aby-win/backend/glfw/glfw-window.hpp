#pragma once

#include "window.hpp"

#include <span>

struct GLFWwindow;

namespace aby::win::glfw {

	class Window : public win::Window {
	public:
		Window(const Config& config);
		~Window();

		auto set_name(std::string_view name) -> void override;
		auto set_width(uint32_t w) -> void override;
		auto set_height(uint32_t h) -> void override;
		auto set_size(uint32_t w, uint32_t h) -> void override;
		auto set_position(int32_t x, int32_t y) -> void override;
		auto set_fullscreen(bool fullscreen) -> void override;
		auto set_cursor_mode(ECursorMode mode) -> void override;
		auto set_cursor_pos(float x, float y) -> void override;
		auto set_theme(ETheme theme) -> void override;
		auto set_icon(const Icon& icon) -> void override;
		auto set_hit_test_config(const HitTestConfig& cfg) -> void override;
		auto add_listener(WindowListener&& listener) -> void override;

		auto focus() -> void override;
		auto minimize() -> void override;
		auto maximize() -> void override;
		auto show() -> void override;
		auto hide() -> void override;
		auto close() -> void override;
		auto poll() -> void override;

		auto width() const -> uint32_t override;
		auto height() const -> uint32_t override;
		auto size() const -> std::pair<uint32_t, uint32_t> override;
		auto position() const -> std::pair<int32_t, int32_t> override;
		auto scale() const -> float override;
		auto native() const -> NativeWindow override;
		auto fb_width() const -> uint32_t override;
		auto fb_height() const -> uint32_t override;
		auto fb_size() const -> std::pair<uint32_t, uint32_t> override;
		auto monitor() const -> const Monitor* override;
		auto listeners() -> std::span<WindowListener>;

		auto focused() const -> bool override;
		auto minimized() const -> bool override;
		auto maximized() const -> bool override;
		auto visible() const -> bool override;
		auto fullscreened() -> bool override;
		auto should_close() const -> bool override;

		auto internal_set_monitor(std::unique_ptr<Monitor>&& monitor) -> void;
		auto internal_get_hit_test_config() -> HitTestConfig&;
#ifdef _WIN32
		auto internal_get_old_wnd_proc() -> void*;
#endif
	private:
		bool bDecorated                    = true;
		bool bHitFnSet                     = false;
		uint32_t m_WindowedX               = 0;
		uint32_t m_WindowedY               = 0;
		uint32_t m_WindowedWidth           = 0;
		uint32_t m_WindowedHeight          = 0;
		HitTestConfig m_HitTestCfg         = {};
		GLFWwindow* m_GLFW                 = nullptr;
		std::unique_ptr<Monitor> m_Monitor = nullptr;
		std::vector<WindowListener> m_Listeners;
#ifdef _WIN32
		void* m_OldWndProc;
#endif
	};

} // namespace aby::win::glfw
