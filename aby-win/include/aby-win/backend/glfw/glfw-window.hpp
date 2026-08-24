#pragma once

#include "window.hpp"

struct GLFWwindow;

namespace aby::win::glfw {

	class Window : public win::Window {
	public:
		Window(std::string_view name, u32 w, u32 h, ERenderBackend backend, ETheme theme);
		~Window();

		auto set_name(std::string_view name) -> void override;
		auto set_width(u32 w) -> void override;
		auto set_height(u32 h) -> void override;
		auto set_size(u32 w, u32 h) -> void override;
		auto set_cursor_mode(ECursorMode mode) -> void override;
		auto set_cursor_pos(float x, float y) -> void override;
		auto set_theme(ETheme theme) -> void override;

		auto focus() -> void override;
		auto minimize() -> void override;
		auto maximize() -> void override;
		auto show() -> void override;
		auto close() -> void override;
		auto poll() -> void override;

		auto width() const -> u32 override;
		auto height() const -> u32 override;
		auto size() const -> std::pair<u32, u32> override;
		auto scale() const -> float override;
		auto native() const -> NativeWindow override;
		auto fb_width() const -> u32 override;
		auto fb_height() const -> u32 override;
		auto fb_size() const -> std::pair<u32, u32> override;

		auto focused() const -> bool override;
		auto minimized() const -> bool override;
		auto maximized() const -> bool override;
		auto visible() const -> bool override;
		auto should_close() const -> bool override;
	private:
		GLFWwindow* m_GLFW;
	};

} // namespace aby::win::glfw
