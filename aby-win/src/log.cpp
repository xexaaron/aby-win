#include "Log.hpp"

#include <chrono>
#include <string>
#include <string_view>

namespace aby::win::win {

	Logger::MapTy<ELogger, std::shared_ptr<Logger>> Logger::s_Loggers = {};

	auto Logger::get(ELogger logger) -> std::shared_ptr<Logger> {
		auto it = s_Loggers.find(logger);
		if (it == s_Loggers.end()) {
			auto logger_ref   = std::make_shared<Logger>();
			s_Loggers[logger] = logger_ref;
			return logger_ref;
		}
		return it->second;
	}

	auto Logger::get_timestamp() -> std::string {
		auto now = std::chrono::system_clock::now();
		auto t   = std::chrono::system_clock::to_time_t(now);
		std::stringstream ss;
		ss << std::put_time(std::localtime(&t), m_TimestampFormat.data());
		return ss.str();
	}

	auto Logger::format_depth(size_t depth) -> std::string {
		constexpr auto dim   = "\033[38;2;45;45;45m";
		constexpr auto green = "\033[38;2;181;206;168m";
		constexpr auto reset = "\x1b[0m";

		if (depth < 10) {
			return std::string(dim) + "00" + reset +
			       green + std::to_string(depth) + reset;
		}

		if (depth < 100) {
			return std::string(dim) + "0" + reset +
			       green + std::to_string(depth) + reset;
		}

		return std::format("{}{}{}", green, depth, reset);
	}

	auto Logger::format_fn(std::string_view fn) -> std::string {
		return format_fn(std::string(fn));
	}

	auto Logger::format_type(std::string_view type) -> std::string {
		return format_type(std::string(type));
	}

	auto Logger::format_fn(const std::string& fn) -> std::string {
		constexpr std::string_view ns_color    = "\x1b[38;2;183;200;200m";
		constexpr std::string_view class_color = "\x1b[38;2;62;201;149m";
		constexpr std::string_view fn_color    = "\x1b[38;2;208;220;144m";
		constexpr std::string_view sep_color   = "\x1b[37m";
		constexpr std::string_view reset       = "\x1b[0m";

		std::string out;
		out.reserve(fn.size() + 64);

		size_t last = fn.rfind("::");

		// func
		if (last == std::string::npos) {
			out += fn_color;
			out += fn;
			out += reset;
			return out;
		}

		size_t second_last = fn.rfind("::", last - 1);

		// foo::func
		if (second_last == std::string::npos) {
			auto ns       = fn.substr(0, last);
			auto function = fn.substr(last + 2);

			out += ns_color;
			out += ns;

			out += sep_color;
			out += "::";

			out += fn_color;
			out += function;

			out += reset;
			return out;
		}

		// foo::Class::func
		auto ns       = fn.substr(0, second_last);
		auto cls      = fn.substr(second_last + 2, last - (second_last + 2));
		auto function = fn.substr(last + 2);

		out += ns_color;
		out += ns;

		out += sep_color;
		out += "::";

		out += class_color;
		out += cls;

		out += sep_color;
		out += "::";

		out += fn_color;
		out += function;

		out += reset;

		return out;
	}

	auto Logger::format_type(const std::string& type) -> std::string {
		constexpr std::string_view class_color = "\x1b[38;2;62;201;149m";
		constexpr std::string_view reset       = "\x1b[0m";
		std::string out;
		out += class_color;
		out += type;
		out += reset;
		return out;
	}

	auto Logger::set_trace(bool enabled) -> Logger* {
		bTrace = enabled;
		return this;
	}

	auto Logger::set_todo(bool enabled) -> Logger* {
		bTodo = enabled;
		return this;
	}

	auto Logger::set_time(bool enabled) -> Logger* {
		bTime          = enabled;
		Timer::enabled = enabled;
		return this;
	}

	auto Logger::set_level(ELevel lvl) -> Logger* {
		m_Level = lvl;
		return this;
	}

	auto Logger::set_timestamp(bool enabled, std::string_view format) -> Logger* {
		bTimestamp        = enabled;
		m_TimestampFormat = format;
		return this;
	}

	auto Logger::set_level_info(ELevel lvl, const LogInfo& info) -> Logger* {
		m_Levels[lvl] = info;
		return this;
	}

	auto Logger::set_color(ELevel level, std::string_view color) -> Logger* {
		m_Levels[level].color = color;
		return this;
	}

	auto Logger::set_prefix(ELevel level, std::string_view prefix) -> Logger* {
		m_Levels[level].prefix = prefix;
		return this;
	}

	auto Logger::set_stream(ELevel level, std::ostream* stream) -> Logger* {
		m_Levels[level].stream = stream;
		return this;
	}

	auto Logger::time(const std::string& name, std::chrono::microseconds us, std::chrono::milliseconds ms) -> void {
		if (!bTime) {
			return;
		}

		auto& info     = m_Levels[ELevel::Log];
		double seconds = static_cast<double>(us.count()) / 1'000'000.0;

		if (bTimestamp) {
			std::println(*info.stream,
			             "{} {}{}\x1b[0m {}",
			             get_timestamp(),
			             info.color,
			             info.prefix,
			             std::format("{} took {:.4f}s", name, seconds));
		} else {
			std::println(*info.stream,
			             "{}{}\x1b[0m {}",
			             info.color,
			             info.prefix,
			             std::format("{} took {:.4f}s", name, seconds));
		}
	}

} // namespace aby::win::win

namespace aby::win::win {

	Timer::Timer(std::string name, ...) :
	    name(std::move(name)),
	    start(enabled ? std::chrono::high_resolution_clock::now() : std::chrono::high_resolution_clock::time_point()) {
	}

	Timer::~Timer() {
		if (enabled) {
			auto end = std::chrono::high_resolution_clock::now();
			auto us  = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
			auto ms  = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
			Logger::get(ELogger::Client)->time(this->name, us, ms);
		}
	}

} // namespace aby::win::win
