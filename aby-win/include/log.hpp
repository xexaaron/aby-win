#pragma once
#include <chrono>
#include <filesystem>
#include <format>
#include <iostream>
#include <memory>
#include <print>
#include <unordered_map>

#define __LOC_FN_SEP_WIDTH__ "70"
#define __TIMESTAMP_FORMAT__ "\033[1;30m%H:%M:%S\033[0m"

#define client_inf(fmt, ...) ::aby::win::Logger::get(::aby::win::ELogger::Client)->log(::aby::win::ELevel::Log, fmt __VA_OPT__(, ) __VA_ARGS__);
#define client_wrn(fmt, ...) ::aby::win::Logger::get(::aby::win::ELogger::Client)->log(::aby::win::ELevel::Warn, fmt __VA_OPT__(, ) __VA_ARGS__);
#define client_err(fmt, ...) ::aby::win::Logger::get(::aby::win::ELogger::Client)->log(::aby::win::ELevel::Err, fmt __VA_OPT__(, ) __VA_ARGS__);

#ifndef _NDEBUG
#	define mconcat_impl(x, y) x##y
#	define mconcat(x, y) mconcat_impl(x, y)
#	define TIME_SCOPE(...) ::aby::win::Timer mconcat(__scoped_timer__, __LINE__)(__VA_ARGS__ __VA_OPT__(, ) __FUNCTION__)
#	define TRACE_SCOPE()                                   \
		::aby::win::TraceDepth mconcat(__trc_depth__, __LINE__); \
		::aby::win::Logger::get(::aby::win::ELogger::Internal)->trace(::aby::win::fs::path(__FILE__), ::aby::win::Line{ __LINE__ }, __FUNCTION__)
#	define log_trc(...) ::aby::win::Logger::get(::aby::win::ELogger::Internal)->trace(__FILE__, ::aby::win::Line{ __LINE__ }, __FUNCTION__ __VA_OPT__(, ) __VA_ARGS__)
#	define log_inf(fmt, ...) ::aby::win::Logger::get(::aby::win::ELogger::Internal)->log(::aby::win::ELevel::Log, fmt __VA_OPT__(, ) __VA_ARGS__)
#	define log_wrn(fmt, ...) ::aby::win::Logger::get(::aby::win::ELogger::Internal)->log(::aby::win::ELevel::Warn, fmt __VA_OPT__(, ) __VA_ARGS__)
#	define log_err(fmt, ...) ::aby::win::Logger::get(::aby::win::ELogger::Internal)->log(::aby::win::ELevel::Err, fmt __VA_OPT__(, ) __VA_ARGS__)
#	define log_dev(fmt, ...) ::aby::win::Logger::get(::aby::win::ELogger::Internal)->log(::aby::win::ELevel::Dev, fmt __VA_OPT__(, ) __VA_ARGS__)
#	define log_tdo(fmt, ...) ::aby::win::Logger::get(::aby::win::ELogger::Internal)->todo(::aby::win::fs::path(__FILE__), ::aby::win::Line{ __LINE__ }, fmt __VA_OPT__(, ) __VA_ARGS__)
#	define log_ast(expr, fmt, ...)                                                                                                                                     \
		do {                                                                                                                                                            \
			::aby::win::Logger::get(::aby::win::ELogger::Internal)->log(::aby::win::ELevel::Assert, "{}:({}) @ {}", ::aby::win::fs::path(__FILE__), ::aby::win::Line{ __LINE__ }, __FUNCTION__); \
			::aby::win::Logger::get(::aby::win::ELogger::Internal)->log(::aby::win::ELevel::Assert, "Trace Depth: {}", ::aby::win::Logger::format_depth(TraceDepth::value));                \
			::aby::win::Logger::get(::aby::win::ELogger::Internal)->log(::aby::win::ELevel::Assert, "Expression: {}", #expr);                                                          \
			::aby::win::Logger::get(::aby::win::ELogger::Internal)->log(::aby::win::ELevel::Assert, fmt __VA_OPT__(, ) __VA_ARGS__);                                                   \
		} while (0)
#	define expect(expr, fmt, ...)                             \
		do {                                                   \
			if (!(expr)) {                                     \
				log_ast(expr, fmt __VA_OPT__(, ) __VA_ARGS__); \
				std::exit(1);                                  \
			}                                                  \
		} while (0)
#else
#	define TIME_SCOPE(...)
#	define TRACE_SCOPE() (void)0
#	define log_trc(...)
#	define log_tdo(...)
#	define log_inf(...)
#	define log_err(...)
#	define log_wrn(...)
#	define log_ast(...)
#	define log_dev(...)
#	define expect(...)
#endif

namespace aby::win::win {

	namespace fs = std::filesystem;

	enum class ELogger {
		Internal,
		Client,
	};

	enum class ELevel {
		Todo   = -2, // Athough not affected by log level, we still support prefix, color, & stream settings
		Trace  = -1, // Athough not affected by log level, we still support prefix, color, & stream settings
		Log    = 0,
		Warn   = 1,
		Err    = 2,
		Assert = 3,
		Dev    = 4,
	};

	struct Location {
		uint64_t line;
		uint64_t col;
	};

	struct Line {
		uint64_t value;
	};

	struct TraceDepth {
		TraceDepth() {
			value++;
		}
		~TraceDepth() {
			value--;
		}

		thread_local static inline size_t value = 0;
	};

	struct Timer {
		Timer(std::string name, ...); // Variadic to allow the macro to default param as __FILE__
		~Timer();

		std::string name;
		std::chrono::high_resolution_clock::time_point start;
	private:
		friend class Logger;
		static inline bool enabled = false;
	};

	struct LogInfo {
		std::string color    = "";
		std::string prefix   = "";
		std::ostream* stream = &std::cout;
	};

	class Logger {
	private:
		template <typename K, typename V>
		using MapTy = std::unordered_map<K, V>;
	public:
		static auto get(ELogger logger) -> std::shared_ptr<Logger>;
		auto get_timestamp() -> std::string;

		static auto format_fn(std::string_view fn) -> std::string;
		static auto format_type(std::string_view type) -> std::string;
		static auto format_fn(const std::string& fn) -> std::string;
		static auto format_type(const std::string& type) -> std::string;
		static auto format_depth(size_t depth) -> std::string;

		template <typename... Args>
		auto log(ELevel lvl, std::format_string<Args...> fmt, Args&&... args) -> void;
		template <typename... Args>
		auto trace(const fs::path& file, Line line, const std::string& fn) -> void;
		template <typename... Args>
		auto trace(const fs::path& file, Line line, const std::string& fn, std::format_string<Args...> fmt, Args&&... args) -> void;
		template <typename... Args>
		auto todo(const fs::path& file, Line line, std::format_string<Args...> fmt, Args&&... args) -> void;
		auto time(const std::string& name, std::chrono::microseconds us, std::chrono::milliseconds ms) -> void;

		auto set_trace(bool enabled) -> Logger*;
		auto set_todo(bool enabled) -> Logger*;
		auto set_time(bool enabled) -> Logger*;
		auto set_level(ELevel lvl) -> Logger*;
		auto set_timestamp(bool enabled, std::string_view format = __TIMESTAMP_FORMAT__) -> Logger*;
		auto set_level_info(ELevel lvl, const LogInfo& info) -> Logger*;
		auto set_color(ELevel level, std::string_view color) -> Logger*;
		auto set_prefix(ELevel level, std::string_view prefix) -> Logger*;
		auto set_stream(ELevel level, std::ostream* stream) -> Logger*;
	private:
		bool bTimestamp                 = false;
		bool bTrace                     = false;
		bool bTodo                      = false;
		bool bTime                      = false;
		std::string m_TimestampFormat   = "";
		ELevel m_Level                  = ELevel::Dev;
		MapTy<ELevel, LogInfo> m_Levels = {};
	private:
		static MapTy<ELogger, std::shared_ptr<Logger>> s_Loggers;
	};

} // namespace aby::win::win

namespace std {

	template <>
	struct formatter<filesystem::path, char> {
		template <class ParseContext>
		constexpr ParseContext::iterator parse(ParseContext& ctx) {
			auto it = ctx.begin();
			if (it != ctx.end() && *it != '}')
				throw format_error("invalid format");
			return it;
		}

		template <class FmtContext>
		FmtContext::iterator format(const filesystem::path& path, FmtContext& ctx) const {
			string str = "\033[34m\033]8;;\033\\";
			str.append(path.string());
			str.append("\033]8;;\033\\\033[0m");
			return ranges::copy(str, ctx.out()).out;
		}
	};

	template <>
	struct formatter<aby::win::Location, char> {
		template <class ParseContext>
		constexpr ParseContext::iterator parse(ParseContext& ctx) {
			auto it = ctx.begin();
			if (it != ctx.end() && *it != '}')
				throw format_error("invalid format");
			return it;
		}

		template <class FmtContext>
		FmtContext::iterator format(const aby::win::Location& loc, FmtContext& ctx) const {
			return format_to(ctx.out(), "\033[38;2;181;206;168m{}\x1b[0m,\033[38;2;181;206;168m{}\x1b[0m", loc.line, loc.col);
		}
	};

	template <>
	struct formatter<aby::win::Line, char> {
		template <class ParseContext>
		constexpr ParseContext::iterator parse(ParseContext& ctx) {
			auto it = ctx.begin();
			if (it != ctx.end() && *it != '}')
				throw format_error("invalid format");
			return it;
		}

		template <class FmtContext>
		FmtContext::iterator format(const aby::win::Line& line, FmtContext& ctx) const {
			return format_to(ctx.out(), "\033[38;2;181;206;168m{}\x1b[0m", line.value);
		}
	};

} // namespace std

namespace aby::win::win {

	template <typename... Args>
	auto Logger::log(ELevel lvl, std::format_string<Args...> fmt, Args&&... args) -> void {
		expect(lvl >= ELevel::Log, "Use appropraite logger function instead. (trace|todo)");
		if (lvl > m_Level) {
			return;
		}
		auto& info = m_Levels[lvl];

		if (bTimestamp) {
			std::println(*info.stream, "{} {}{}\x1b[0m {}",
			             get_timestamp(), info.color, info.prefix,
			             std::format(fmt, std::forward<Args>(args)...));
		} else {
			std::println(*info.stream, "{}{}\x1b[0m {}",
			             info.color, info.prefix,
			             std::format(fmt, std::forward<Args>(args)...));
		}
	}

	template <typename... Args>
	auto Logger::trace(const fs::path& file, Line line, const std::string& fn) -> void {
		if (!bTrace) {
			return;
		}
		auto& info = m_Levels[ELevel::Trace];
		auto loc   = std::format("{}:({})", file, line);
		if (bTimestamp) {
			std::println(*info.stream, "{} {}{}\x1b[0m [{}] {}|\x1b[0m {:<" __LOC_FN_SEP_WIDTH__ "} @ {}",
			             get_timestamp(), info.color, info.prefix, format_depth(TraceDepth::value), info.color, loc, format_fn(fn));
		} else {
			std::println(*info.stream, "{}{}\x1b[0m [{}] {}|\x1b[0m {:<" __LOC_FN_SEP_WIDTH__ "} @ {}",
			             info.color, info.prefix, format_depth(TraceDepth::value), info.color, loc, format_fn(fn));
		}
	}

	template <typename... Args>
	auto Logger::trace(const fs::path& file, Line line, const std::string& fn, std::format_string<Args...> fmt, Args&&... args) -> void {
		if (!bTrace) {
			return;
		}
		auto& info = m_Levels[ELevel::Trace];
		auto loc   = std::format("{}:({})", file, line);
		if (bTimestamp) {
			std::println(*info.stream, "{} {}{}\x1b[0m [{}] {}|\x1b[0m {:<" __LOC_FN_SEP_WIDTH__ "} @ {} | {}",
			             get_timestamp(), info.color, info.prefix, format_depth(TraceDepth::value), info.color, loc, format_fn(fn),
			             std::format(fmt, std::forward<Args>(args)...));
		} else {
			std::println(*info.stream, "{}{}\x1b[0m [{}] {}|\x1b[0m {:<" __LOC_FN_SEP_WIDTH__ "} @ {} | {}",
			             info.color, info.prefix, format_depth(TraceDepth::value), info.color, loc, format_fn(fn),
			             std::format(fmt, std::forward<Args>(args)...));
		}
	}

	template <typename... Args>
	auto Logger::todo(const fs::path& file, Line line, std::format_string<Args...> fmt, Args&&... args) -> void {
		if (!bTodo) {
			return;
		}
		auto& info = m_Levels[ELevel::Todo];
		if (bTimestamp) {
			std::println(*info.stream, "{} {}{}\x1b[0m {}:({}): {}",
			             get_timestamp(), info.color, info.prefix,
			             fs::relative(file), line,
			             std::format(fmt, std::forward<Args>(args)...));
		} else {
			std::println(*info.stream, "{}{}\x1b[0m {}:({}): {}",
			             info.color, info.prefix,
			             fs::relative(file), line,
			             std::format(fmt, std::forward<Args>(args)...));
		}
	}

} // namespace aby::win::win

