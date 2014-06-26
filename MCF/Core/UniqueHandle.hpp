// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef MCF_UNIQUE_HANDLE_HPP_
#define MCF_UNIQUE_HANDLE_HPP_

#include <utility>
#include <type_traits>
#include <cstddef>

namespace MCF {

template<class Closer_t>
class UniqueHandle {
public:
	typedef decltype(Closer_t()()) Handle;

	static_assert(std::is_scalar<Handle>::value, "Handle must be a scalar type.");
	static_assert(noexcept(Closer_t()(Handle())), "Handle closer must not throw.");

private:
	Handle xm_hObj;

public:
	constexpr UniqueHandle() noexcept
		: UniqueHandle(Closer_t()())
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
		return Get() != Closer_t()();
	}
	Handle Get() const noexcept {
		return xm_hObj;
	}
	Handle Release() noexcept {
		return std::exchange(xm_hObj, Closer_t()());
	}

	void Reset(Handle hObj = Closer_t()()) noexcept {
		const auto hOld = std::exchange(xm_hObj, hObj);
		if(hOld != Closer_t()()){
			Closer_t()(hOld);
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

	bool operator==(const UniqueHandle &rhs) const noexcept {
		return Get() == rhs.Get();
	}
	bool operator!=(const UniqueHandle &rhs) const noexcept {
		return Get() != rhs.Get();
	}
	bool operator<(const UniqueHandle &rhs) const noexcept {
		return Get() < rhs.Get();
	}
	bool operator<=(const UniqueHandle &rhs) const noexcept {
		return Get() <= rhs.Get();
	}
	bool operator>(const UniqueHandle &rhs) const noexcept {
		return Get() > rhs.Get();
	}
	bool operator>=(const UniqueHandle &rhs) const noexcept {
		return Get() >= rhs.Get();
	}

	bool operator==(Handle rhs) const noexcept {
		return Get() == rhs;
	}
	bool operator!=(Handle rhs) const noexcept {
		return Get() != rhs;
	}
	bool operator<(Handle rhs) const noexcept {
		return Get() < rhs;
	}
	bool operator<=(Handle rhs) const noexcept {
		return Get() <= rhs;
	}
	bool operator>(Handle rhs) const noexcept {
		return Get() > rhs;
	}
	bool operator>=(Handle rhs) const noexcept {
		return Get() >= rhs;
	}
};

template<class Handle_t, class Closer_t>
auto operator==(Handle_t lhs, const UniqueHandle<Closer_t> &rhs) noexcept
	-> typename std::enable_if<
		std::is_same<Handle_t, typename UniqueHandle<Closer_t>::Handle>::value,
		bool
	>::type
{
	return lhs == rhs.Get();
}
template<class Handle_t, class Closer_t>
auto operator!=(Handle_t lhs, const UniqueHandle<Closer_t> &rhs) noexcept
	-> typename std::enable_if<
		std::is_same<Handle_t, typename UniqueHandle<Closer_t>::Handle>::value,
		bool
	>::type
{
	return lhs != rhs.Get();
}
template<class Handle_t, class Closer_t>
auto operator<(Handle_t lhs, const UniqueHandle<Closer_t> &rhs) noexcept
	-> typename std::enable_if<
		std::is_same<Handle_t, typename UniqueHandle<Closer_t>::Handle>::value,
		bool
	>::type
{
	return lhs < rhs.Get();
}
template<class Handle_t, class Closer_t>
auto operator<=(Handle_t lhs, const UniqueHandle<Closer_t> &rhs) noexcept
	-> typename std::enable_if<
		std::is_same<Handle_t, typename UniqueHandle<Closer_t>::Handle>::value,
		bool
	>::type
{
	return lhs <= rhs.Get();
}
template<class Handle_t, class Closer_t>
auto operator>(Handle_t lhs, const UniqueHandle<Closer_t> &rhs) noexcept
	-> typename std::enable_if<
		std::is_same<Handle_t, typename UniqueHandle<Closer_t>::Handle>::value,
		bool
	>::type
{
	return lhs > rhs.Get();
}
template<class Handle_t, class Closer_t>
auto operator>=(Handle_t lhs, const UniqueHandle<Closer_t> &rhs) noexcept
	-> typename std::enable_if<
		std::is_same<Handle_t, typename UniqueHandle<Closer_t>::Handle>::value,
		bool
	>::type
{
	return lhs >= rhs.Get();
}

}

#endif
