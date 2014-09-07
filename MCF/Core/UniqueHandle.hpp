// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_UNIQUE_HANDLE_HPP_
#define MCF_UNIQUE_HANDLE_HPP_

#include <utility>
#include <type_traits>
#include <cstddef>

namespace MCF {

template<class Closer>
class UniqueHandle {
public:
	typedef decltype(Closer()()) Handle;

	static_assert(std::is_scalar<Handle>::value, "Handle must be a scalar type.");
	static_assert(noexcept(Closer()(Handle())), "Handle closer must not throw.");

private:
	Handle xm_hObj;

public:
	constexpr UniqueHandle() noexcept
		: UniqueHandle(Closer()())
	{
	}
	constexpr explicit UniqueHandle(Handle hObj) noexcept
		: xm_hObj(hObj)
	{
	}
	UniqueHandle(UniqueHandle &&rhs) noexcept
		: UniqueHandle(rhs.Release())
	{
	}
	UniqueHandle &operator=(Handle hObj) noexcept {
		Reset(hObj);
		return *this;
	}
	UniqueHandle &operator=(UniqueHandle &&rhs) noexcept {
		Reset(std::move(rhs));
		return *this;
	}
	~UniqueHandle() noexcept {
		Reset();
	}

	UniqueHandle(const UniqueHandle &) = delete;
	void operator=(const UniqueHandle &) = delete;

public:
	bool IsGood() const noexcept {
		return Get() != Closer()();
	}
	Handle Get() const noexcept {
		return xm_hObj;
	}
	Handle Release() noexcept {
		return std::exchange(xm_hObj, Closer()());
	}

	void Reset(Handle hObj = Closer()()) noexcept {
		const auto hOld = std::exchange(xm_hObj, hObj);
		if(hOld != Closer()()){
			Closer()(hOld);
		}
	}
	void Reset(UniqueHandle &&rhs) noexcept {
		if(&rhs != this){
			Reset(rhs.Release());
		}
	}

	void Swap(UniqueHandle &rhs) noexcept {
		std::swap(xm_hObj, rhs.xm_hObj);
	}

public:
	explicit operator bool() const noexcept {
		return IsGood();
	}
	explicit operator Handle() const noexcept {
		return Get();
	}
};

#define MCF_UNIQUE_HANDLE_RATIONAL_OPERATOR_(op_type)	\
	template<class Closer>	\
	bool operator op_type (	\
		const UniqueHandle<Closer> &lhs,	\
		const UniqueHandle<Closer> &rhs	\
	) noexcept {	\
		return lhs.Get() op_type rhs.Get();	\
	}	\
	template<class Closer>	\
	bool operator op_type (	\
		decltype(Closer()()) lhs,	\
		const UniqueHandle<Closer> &rhs	\
	) noexcept {	\
		return lhs op_type rhs.Get();	\
	}	\
	template<class Closer>	\
	bool operator op_type (	\
		const UniqueHandle<Closer> &lhs,	\
		decltype(Closer()()) rhs	\
	) noexcept {	\
		return lhs.Get() op_type rhs;	\
	}

MCF_UNIQUE_HANDLE_RATIONAL_OPERATOR_(==)
MCF_UNIQUE_HANDLE_RATIONAL_OPERATOR_(!=)
MCF_UNIQUE_HANDLE_RATIONAL_OPERATOR_(<)
MCF_UNIQUE_HANDLE_RATIONAL_OPERATOR_(>)
MCF_UNIQUE_HANDLE_RATIONAL_OPERATOR_(<=)
MCF_UNIQUE_HANDLE_RATIONAL_OPERATOR_(>=)

#undef MCF_UNIQUE_HANDLE_RATIONAL_OPERATOR_

template<class Closer>
void swap(UniqueHandle<Closer> &lhs, UniqueHandle<Closer> &rhs) noexcept {
	lhs.Swap(rhs);
}

}

#endif
