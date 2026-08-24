#pragma once
#include <cstdarg>
#include <memory>
#include <string>

#ifndef NDEBUG
#	define aby_win_dbg(msg, ...) ::aby::win::ILogger::get()->log(::aby::win::ELogLevel::debug, std::format(msg __VA_OPT__(, ) __VA_ARGS__))
#else
#	define aby_win_dbg(...)
#endif
#if ABY_WIN_ENABLE_LOG_TRACE == 1
#	define aby_win_trc(msg, ...) ::aby::win::ILogger::get()->log(::aby::win::ELogLevel::trace, std::format(msg __VA_OPT__(, ) __VA_ARGS__))
#else
#	define aby_win_trc(...)
#endif
#if ABY_WIN_ENABLE_LOG_INFO == 1
#	define aby_win_log(msg, ...) ::aby::win::ILogger::get()->log(::aby::win::ELogLevel::info, std::format(msg __VA_OPT__(, ) __VA_ARGS__))
#else
#	define aby_win_log(...)
#endif
#if ABY_WIN_ENABLE_LOG_WARN == 1
#	define aby_win_wrn(msg, ...) ::aby::win::ILogger::get()->log(::aby::win::ELogLevel::warn, std::format(msg __VA_OPT__(, ) __VA_ARGS__))
#else
#	define aby_win_wrn(...)
#endif

#define aby_win_err(msg, ...) ::aby::win::ILogger::get()->log(::aby::win::ELogLevel::error, std::format(msg __VA_OPT__(, ) __VA_ARGS__))
#define aby_win_ftl(msg, ...) ::aby::win::ILogger::get()->log(::aby::win::ELogLevel::fatal, std::format(msg __VA_OPT__(, ) __VA_ARGS__))

#if ABY_WIN_ENABLE_ASSERT == 1
#	if defined(_MSC_VER)
#		define ABY_WIN_DEBUG_BREAK() __debugbreak()
#	elif defined(__clang__) || defined(__GNUC__)
#		if defined(_WIN32)
#			define ABY_WIN_DEBUG_BREAK() __builtin_debugtrap()
#		elif defined(__i386__) || defined(__x86_64__)
#			define ABY_WIN_DEBUG_BREAK() __asm__ __volatile__("int3")
#		elif defined(__aarch64__) || defined(__arm__)
#			define ABY_WIN_DEBUG_BREAK() __builtin_trap()
#		else
#			define ABY_WIN_DEBUG_BREAK() __builtin_trap()
#		endif
#	else
#		include <cstdlib>
#		define ABY_WIN_DEBUG_BREAK() std::exit(2)
#	endif
#else
#	define ABY_WIN_DEBUG_BREAK()
#endif

#if defined(_MSC_VER)
#	define ABY_WIN_FUNCTION_NAME __FUNCSIG__
#elif defined(__GNUC__) || defined(__clang__)
#	define ABY_WIN_FUNCTION_NAME __PRETTY_FUNCTION__
#else
#	define ABY_WIN_FUNCTION_NAME __func__
#endif

#define aby_win_assert(expr, ...)                         \
	do {                                                  \
		if (!(expr)) {                                    \
			aby_win_ftl("assertion failed: {}", #expr);   \
			aby_win_ftl("@ {}:({})", __FILE__, __LINE__); \
			__VA_OPT__(aby_win_ftl(__VA_ARGS__));         \
			ABY_WIN_DEBUG_BREAK();                        \
		}                                                 \
	} while (0)

namespace aby::win {

	enum class ELogLevel {
		debug,
		trace,
		info,
		warn,
		error,
		fatal,
	};

	class ILogger {
	public:
		static auto get() -> ILogger*;
		static auto set(std::unique_ptr<ILogger> logger) -> void;
		template <typename T>
		requires(std::derived_from<T, ILogger> && std::is_default_constructible_v<T>)
		static auto set() -> void {
			s_Logger = std::make_unique<T>();
		}

		virtual auto log(ELogLevel level, const std::string& msg) -> void = 0;
	private:
		static inline std::unique_ptr<ILogger> s_Logger;
	};

	class DefaultLogger : public ILogger {
	public:
		auto log(ELogLevel level, const std::string& msg) -> void override;
	};

} // namespace aby::win
