#pragma once

#if ABY_WIN_ENABLE_QT

#	include "window.hpp"

#	include <span>
#	include <unordered_map>
#	include <variant>

class QMainWindow;
class QWindow;
namespace aby::win::qt {

	class EventFilter;

	class ABY_WIN_API Window : public win::Window {
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
		auto set_cursor(ECursor cursor) -> void override;
		auto set_theme(ETheme theme) -> void override;
		auto set_icon(const Icon& icon) -> void override;
		auto set_hit_test_config(const HitTestConfig& cfg) -> void override;
		auto set_clipboard(std::string_view text) -> void override;
		auto set_resizable(bool value) -> void override;
		auto set_visible(bool value) -> void override;
		auto set_decorated(bool value) -> void override;
		auto set_focused(bool value) -> void override;

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
		auto id() const -> uint32_t override;
		auto clipboard() const -> std::string_view override;

		auto focused() const -> bool override;
		auto minimized() const -> bool override;
		auto maximized() const -> bool override;
		auto visible() const -> bool override;
		auto fullscreened() -> bool override;
		auto should_close() const -> bool override;
	private:
		auto dispatch(Event& event) -> bool;
		auto is_child() -> bool;
		friend class EventFilter;
	private:
		std::variant<QWindow*, QMainWindow*> m_QT;
		std::vector<WindowListener> m_Listeners;
		bool bMainWindow;
		bool bShouldClose;
		EventFilter* m_EventFilter;
#	ifdef __linux__
		mutable std::pair<void*, void*> m_NativeHandles;
#	endif
	};

} // namespace aby::win::qt

#endif
