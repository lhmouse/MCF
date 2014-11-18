// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_UNIQUE_HANDLE_HPP_
#define MCF_CORE_UNIQUE_HANDLE_HPP_

#include <utility>
#include <type_traits>
#include <cstddef>

namespace MCF {

template<class CloserT>
class UniqueHandle {
public:
	using Handle = decltype(CloserT()());

	static_assert(std::is_scalar<Handle>::value, "Handle must be a scalar type.");
	static_assert(noexcept(CloserT()(Handle())), "Handle closer must not throw.");

private:
	Handle xm_hObj;

public:
	constexpr UniqueHandle() noexcept
		: UniqueHandle(CloserT()())
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
	UniqueHandle &operator=(UniqueHandle &&rhs) noexcept {
		Reset(std::move(rhs));
		return *this;
	}
	~UniqueHandle(){
		Reset();
	}

	UniqueHandle(const UniqueHandle &) = delete;
	void operator=(const UniqueHandle &) = delete;

public:
	bool IsGood() const noexcept {
		return Get() != CloserT()();
	}
	Handle Get() const noexcept {
		return xm_hObj;
	}
	Handle Release() noexcept {
		return std::exchange(xm_hObj, CloserT()());
	}

	void Reset(Handle hObj = CloserT()()) noexcept {
		const auto hOld = std::exchange(xm_hObj, hObj);
		if(hOld != CloserT()()){
			CloserT()(hOld);
		}
	}
	void Reset(UniqueHandle &&rhs) noexcept {
		ASSERT(&rhs != this);
		Reset(rhs.Release());
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

template<class CloserT>
bool operator==(const UniqueHandle<CloserT> &lhs, const UniqueHandle<CloserT> &rhs) noexcept {
	return lhs.get() == rhs.get();
}
template<class CloserT>
bool operator==(const UniqueHandle<CloserT> &lhs, decltype(CloserT()()) rhs) noexcept {
	return lhs.get() == rhs;
}
template<class CloserT>
bool operator==(decltype(CloserT()()) lhs, const UniqueHandle<CloserT> &rhs) noexcept {
	return lhs == rhs.get();
}

template<class CloserT>
bool operator!=(const UniqueHandle<CloserT> &lhs, const UniqueHandle<CloserT> &rhs) noexcept {
	return lhs.get() != rhs.get();
}
template<class CloserT>
bool operator!=(const UniqueHandle<CloserT> &lhs, decltype(CloserT()()) rhs) noexcept {
	return lhs.get() != rhs;
}
template<class CloserT>
bool operator!=(decltype(CloserT()()) lhs, const UniqueHandle<CloserT> &rhs) noexcept {
	return lhs != rhs.get();
}

template<class CloserT>
bool operator<(const UniqueHandle<CloserT> &lhs, const UniqueHandle<CloserT> &rhs) noexcept {
	return lhs.get() < rhs.get();
}
template<class CloserT>
bool operator<(const UniqueHandle<CloserT> &lhs, decltype(CloserT()()) rhs) noexcept {
	return lhs.get() < rhs;
}
template<class CloserT>
bool operator<(decltype(CloserT()()) lhs, const UniqueHandle<CloserT> &rhs) noexcept {
	return lhs < rhs.get();
}

template<class CloserT>
bool operator>(const UniqueHandle<CloserT> &lhs, const UniqueHandle<CloserT> &rhs) noexcept {
	return lhs.get() > rhs.get();
}
template<class CloserT>
bool operator>(const UniqueHandle<CloserT> &lhs, decltype(CloserT()()) rhs) noexcept {
	return lhs.get() > rhs;
}
template<class CloserT>
bool operator>(decltype(CloserT()()) lhs, const UniqueHandle<CloserT> &rhs) noexcept {
	return lhs > rhs.get();
}

template<class CloserT>
bool operator<=(const UniqueHandle<CloserT> &lhs, const UniqueHandle<CloserT> &rhs) noexcept {
	return lhs.get() <= rhs.get();
}
template<class CloserT>
bool operator<=(const UniqueHandle<CloserT> &lhs, decltype(CloserT()()) rhs) noexcept {
	return lhs.get() <= rhs;
}
template<class CloserT>
bool operator<=(decltype(CloserT()()) lhs, const UniqueHandle<CloserT> &rhs) noexcept {
	return lhs <= rhs.get();
}

template<class CloserT>
bool operator>=(const UniqueHandle<CloserT> &lhs, const UniqueHandle<CloserT> &rhs) noexcept {
	return lhs.get() >= rhs.get();
}
template<class CloserT>
bool operator>=(const UniqueHandle<CloserT> &lhs, decltype(CloserT()()) rhs) noexcept {
	return lhs.get() >= rhs;
}
template<class CloserT>
bool operator>=(decltype(CloserT()()) lhs, const UniqueHandle<CloserT> &rhs) noexcept {
	return lhs >= rhs.get();
}

template<class CloserT>
void swap(UniqueHandle<CloserT> &lhs, UniqueHandle<CloserT> &rhs) noexcept {
	lhs.Swap(rhs);
}

}

#endif
