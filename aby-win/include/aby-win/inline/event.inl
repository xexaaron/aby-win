#pragma once
#include <type_traits>

#define __ABY_WIN_EVENT_BODY__(_type, _category, _name) \
	static constexpr auto static_type() -> EEvent {     \
		return ::aby::win::EEvent::_type;               \
	}                                                   \
	auto type() const -> EEvent override {              \
		return ::aby::win::EEvent::_type;               \
	}                                                   \
	auto name() const -> std::string_view override {    \
		return _name;                                   \
	}                                                   \
	auto category() const -> EEventCategory override {  \
		return ::aby::win::EEventCategory::_category;   \
	}                                                   \
	auto window() const -> uint32_t override {          \
		return m_WindowID;                              \
	}

#define ABY_WIN_DECLARE_EMPTY_EVENT(_cl_name, _type, _category)       \
	class ABY_WIN_API _cl_name##Event : public ::aby::win::Event {    \
	public:                                                           \
		_cl_name##Event(uint32_t window_id) : m_WindowID(window_id) { \
		}                                                             \
		__ABY_WIN_EVENT_BODY__(_type, _category, #_cl_name);          \
	private:                                                          \
		uint32_t m_WindowID;                                          \
	}

#define ABY_WIN_DECLARE_EVENT(_cl_name, _type, _category, _data_struct)                                                      \
	class ABY_WIN_API _cl_name##Event : public ::aby::win::Event {                                                           \
	public:                                                                                                                  \
		struct _cl_name##Data _data_struct;                                                                                  \
		template <typename... Args>                                                                                          \
		requires(std::is_constructible_v<_cl_name##Data, Args...>)                                                           \
		_cl_name##Event(uint32_t window_id, Args&&... args) : m_WindowID(window_id), m_Data{ std::forward<Args>(args)... } { \
		}                                                                                                                    \
		__ABY_WIN_EVENT_BODY__(_type, _category, #_cl_name);                                                                 \
		auto operator->() -> _cl_name##Data* {                                                                               \
			return &m_Data;                                                                                                  \
		}                                                                                                                    \
		auto operator->() const -> const _cl_name##Data* {                                                                   \
			return &m_Data;                                                                                                  \
		}                                                                                                                    \
	private:                                                                                                                 \
		uint32_t m_WindowID;                                                                                                 \
		_cl_name##Data m_Data;                                                                                               \
	}

namespace aby::win::detail {

	template <typename T>
	struct function_traits;

	// Free function
	template <typename R, typename Arg>
	struct function_traits<R(Arg)> {
		using argument_type = Arg;
	};

	// Function pointer
	template <typename R, typename Arg>
	struct function_traits<R (*)(Arg)> : function_traits<R(Arg)> {
	};

	// Function reference
	template <typename R, typename Arg>
	struct function_traits<R (&)(Arg)> : function_traits<R(Arg)> {
	};

	// Member function
	template <typename C, typename R, typename Arg>
	struct function_traits<R (C::*)(Arg)> : function_traits<R(Arg)> {
	};

	// Const member function
	template <typename C, typename R, typename Arg>
	struct function_traits<R (C::*)(Arg) const> : function_traits<R(Arg)> {
	};

	// Lambda / functor
	template <typename F>
	struct function_traits : function_traits<decltype(&F::operator())> {
	};

	template <typename F>
	using first_argument_t = typename function_traits<std::remove_cvref_t<F>>::argument_type;

} // namespace aby::win::detail
