#pragma once
#include <cstdint>
#include <span>
#include <string_view>
#include <tuple>
#include <utility>
#include <vector>

namespace aby::win {

	struct WorkArea {
		int32_t x;
		int32_t y;
		int32_t w;
		int32_t h;
	};

	using Position = std::pair<int32_t, int32_t>;
	using Size     = std::pair<int32_t, int32_t>;
	using Scale    = std::pair<float, float>;
	using Channels = std::tuple<int32_t, int32_t, int32_t>;

	struct VideoMode {
		Size size;
		Channels channel_bits;
		int32_t refresh_rate;
	};

	class Monitor {
	public:
		Monitor(Position pos, const WorkArea& wa, Size physical_size, Scale scale, std::string_view name, std::vector<VideoMode> video_modes, size_t current_video_mode);

		auto position() const -> const Position&;
		auto work_area() const -> const WorkArea&;
		auto physical_size() const -> const Size&;
		auto name() const -> std::string_view;
		auto video_mode() const -> const VideoMode&;
		auto video_modes() const -> std::span<const VideoMode>;
	private:
		Position m_Position;
		WorkArea m_WorkArea;
		Size m_PhysicalSize;
		Scale m_ContentDisplayScale;
		std::string_view m_Name;
		std::vector<VideoMode> m_VideoModes;
		size_t m_CurrentVideoMode;
	};

} // namespace aby::win
