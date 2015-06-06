// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_VARIANT_HPP_
#define MCF_CORE_VARIANT_HPP_

#include <utility>
#include <type_traits>
#include <cstddef>

namespace MCF {

template<class CloserT>
class UniqueHandle {
public:
	using Handle = std::decay_t<decltype(CloserT()())>;

	static_assert(std::is_scalar<Handle>::value, "Handle must be a scalar type.");
	static_assert(noexcept(CloserT()(Handle())), "Handle closer must not throw.");

private:
	Handle x_hObject;

public:
	constexpr UniqueHandle() noexcept
		: UniqueHandle(CloserT()())
	{
	}
	constexpr explicit UniqueHandle(Handle hObject) noexcept
		: x_hObject(hObject)
	{
	}
	UniqueHandle(UniqueHandle &&rhs) noexcept
		: UniqueHandle()
	{
		Reset(std::move(rhs));
	}
	UniqueHandle &operator=(UniqueHandle &&rhs) noexcept {
		Reset(std::move(rhs));
		return *this;
	}
	~UniqueHandle(){
		Reset();
	}

	UniqueHandle(const UniqueHandle &) = delete;
	UniqueHandle &operator=(const UniqueHandle &) = delete;

public:
	bool IsNonnull() const noexcept {
		return Get() != CloserT()();
	}
	Handle Get() const noexcept {
		return x_hObject;
	}
	Handle Release() noexcept {
		return std::exchange(x_hObject, CloserT()());
	}

	UniqueHandle &Reset(Handle hObject = CloserT()()) noexcept {
		const auto hOld = std::exchange(x_hObject, hObject);
		if(hOld != CloserT()()){
			ASSERT(hOld != hObject);
			CloserT()(hOld);
		}
		return *this;
	}
	UniqueHandle &Reset(UniqueHandle &&rhs) noexcept {
		return Reset(rhs.Release());
	}

	void Swap(UniqueHandle &rhs) noexcept {
		std::swap(x_hObject, rhs.x_hObject);
	}

public:
	explicit operator bool() const noexcept {
		return IsNonnull();
	}
	explicit operator Handle() const noexcept {
		return Get();
	}
};

template<class CloserT>
bool operator==(const UniqueHandle<CloserT> &lhs, const UniqueHandle<CloserT> &rhs) noexcept {
	return lhs.Get() == rhs.Get();
}
template<class CloserT>
bool operator==(const UniqueHandle<CloserT> &lhs, typename UniqueHandle<CloserT>::Handle rhs) noexcept {
	return lhs.Get() == rhs;
}
template<class CloserT>
bool operator==(typename UniqueHandle<CloserT>::Handle lhs, const UniqueHandle<CloserT> &rhs) noexcept {
	return lhs == rhs.Get();
}

template<class CloserT>
bool operator!=(const UniqueHandle<CloserT> &lhs, const UniqueHandle<CloserT> &rhs) noexcept {
	return lhs.Get() != rhs.Get();
}
template<class CloserT>
bool operator!=(const UniqueHandle<CloserT> &lhs, typename UniqueHandle<CloserT>::Handle rhs) noexcept {
	return lhs.Get() != rhs;
}
template<class CloserT>
bool operator!=(typename UniqueHandle<CloserT>::Handle lhs, const UniqueHandle<CloserT> &rhs) noexcept {
	return lhs != rhs.Get();
}

template<class CloserT>
bool operator<(const UniqueHandle<CloserT> &lhs, const UniqueHandle<CloserT> &rhs) noexcept {
	return lhs.Get() < rhs.Get();
}
template<class CloserT>
bool operator<(const UniqueHandle<CloserT> &lhs, typename UniqueHandle<CloserT>::Handle rhs) noexcept {
	return lhs.Get() < rhs;
}
template<class CloserT>
bool operator<(typename UniqueHandle<CloserT>::Handle lhs, const UniqueHandle<CloserT> &rhs) noexcept {
	return lhs < rhs.Get();
}

template<class CloserT>
bool operator>(const UniqueHandle<CloserT> &lhs, const UniqueHandle<CloserT> &rhs) noexcept {
	return lhs.Get() > rhs.Get();
}
template<class CloserT>
bool operator>(const UniqueHandle<CloserT> &lhs, typename UniqueHandle<CloserT>::Handle rhs) noexcept {
	return lhs.Get() > rhs;
}
template<class CloserT>
bool operator>(typename UniqueHandle<CloserT>::Handle lhs, const UniqueHandle<CloserT> &rhs) noexcept {
	return lhs > rhs.Get();
}

template<class CloserT>
bool operator<=(const UniqueHandle<CloserT> &lhs, const UniqueHandle<CloserT> &rhs) noexcept {
	return lhs.Get() <= rhs.Get();
}
template<class CloserT>
bool operator<=(const UniqueHandle<CloserT> &lhs, typename UniqueHandle<CloserT>::Handle rhs) noexcept {
	return lhs.Get() <= rhs;
}
template<class CloserT>
bool operator<=(typename UniqueHandle<CloserT>::Handle lhs, const UniqueHandle<CloserT> &rhs) noexcept {
	return lhs <= rhs.Get();
}

template<class CloserT>
bool operator>=(const UniqueHandle<CloserT> &lhs, const UniqueHandle<CloserT> &rhs) noexcept {
	return lhs.Get() >= rhs.Get();
}
template<class CloserT>
bool operator>=(const UniqueHandle<CloserT> &lhs, typename UniqueHandle<CloserT>::Handle rhs) noexcept {
	return lhs.Get() >= rhs;
}
template<class CloserT>
bool operator>=(typename UniqueHandle<CloserT>::Handle lhs, const UniqueHandle<CloserT> &rhs) noexcept {
	return lhs >= rhs.Get();
}

template<class CloserT>
void swap(UniqueHandle<CloserT> &lhs, UniqueHandle<CloserT> &rhs) noexcept {
	lhs.Swap(rhs);
}

}

#endif
