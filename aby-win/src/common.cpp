#include "common.hpp"

namespace aby::win {

	namespace {

		constexpr std::string_view reset = "\033[0m";

		constexpr std::string_view green     = "\033[32m";
		constexpr std::string_view yellow    = "\033[33m";
		constexpr std::string_view red       = "\033[31m";
		constexpr std::string_view cyan      = "\033[36m";
		constexpr std::string_view white     = "\033[37m";
		constexpr std::string_view dark_grey = "\033[90m";

		constexpr std::string_view red_background_white_text = "\033[41;97m";

	} // namespace

	std::unique_ptr<ILogger> ILogger::s_Logger = nullptr;

	auto ILogger::get() -> ILogger* {
		return s_Logger.get();
	}

	auto ILogger::set(std::unique_ptr<ILogger> logger) -> void {
		s_Logger = std::move(logger);
	}

	auto DefaultLogger::log(ELogLevel level, const std::string& msg) -> void {
		std::string_view fmt;
		std::string_view color;

		switch (level) {
			case ELogLevel::debug:
#ifdef NDEBUG // skip on release build
				return;
#endif
				fmt   = "[%sdbg%s] %s\n";
				color = cyan;
				break;
			case ELogLevel::trace:
#ifdef NDEBUG // skip on release build
				return;
#endif
				fmt   = "[%strc%s] %s\n";
				color = dark_grey;
				break;
			case ELogLevel::info:
				fmt   = "[%slog%s] %s\n";
				color = green;
				break;
			case ELogLevel::warn:
				fmt   = "[%swrn%s] %s\n";
				color = yellow;
				break;
			case ELogLevel::error:
				fmt   = "[%serr%s] %s\n";
				color = red;
				break;
			case ELogLevel::fatal:
				fmt   = "[%sftl%s] %s\n";
				color = red_background_white_text;
				break;
		}

		std::fprintf(stdout, fmt.data(), color.data(), reset.data(), msg.c_str());
	}

} // namespace aby::win
