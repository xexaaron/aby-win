#pragma once

#define __ABY_WIN_EVENT_BODY__(_type, _category, _name) \
	static auto static_type() -> EEvent {               \
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
	}

#define ABY_WIN_DECLARE_EMPTY_EVENT(_cl_name, _type, _category) \
	class _cl_name##Event : public ::aby::win::Event {          \
	public:                                                     \
		__ABY_WIN_EVENT_BODY__(_type, _category, #_cl_name);    \
	}

#define ABY_WIN_DECLARE_EVENT(_cl_name, _type, _category, _data_struct)           \
	class _cl_name##Event : public ::aby::win::Event {                            \
	public:                                                                       \
		struct _cl_name##Data _data_struct;                                       \
		template <typename... Args>                                               \
		requires(std::is_constructible_v<_cl_name##Data, Args...>)                \
		_cl_name##Event(Args&&... args) : m_Data{ std::forward<Args>(args)... } { \
		}                                                                         \
		__ABY_WIN_EVENT_BODY__(_type, _category, #_cl_name);                      \
		auto operator->() -> _cl_name##Data* {                                    \
			return &m_Data;                                                       \
		}                                                                         \
		auto operator->() const -> const _cl_name##Data* {                        \
			return &m_Data;                                                       \
		}                                                                         \
	private:                                                                      \
		_cl_name##Data m_Data;                                                    \
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
