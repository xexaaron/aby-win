#include "monitor.hpp"

namespace aby::win {

	Monitor::Monitor(Position pos, const WorkArea& wa, Size physical_size, Scale scale, std::string_view name, std::vector<VideoMode> video_modes, size_t current_video_mode) :
	    m_Position(pos),
	    m_WorkArea(wa),
	    m_PhysicalSize(physical_size),
	    m_ContentDisplayScale(scale),
	    m_Name(name),
	    m_VideoModes(video_modes),
	    m_CurrentVideoMode(current_video_mode) {
	}

	auto Monitor::position() const -> const Position& {
		return m_Position;
	}

	auto Monitor::work_area() const -> const WorkArea& {
		return m_WorkArea;
	}

	auto Monitor::physical_size() const -> const Size& {
		return m_PhysicalSize;
	}

	auto Monitor::name() const -> std::string_view {
		return m_Name;
	}

	auto Monitor::video_mode() const -> const VideoMode& {
		return m_VideoModes[m_CurrentVideoMode];
	}

	auto Monitor::video_modes() const -> std::span<const VideoMode> {
		return m_VideoModes;
	}

} // namespace aby::win
