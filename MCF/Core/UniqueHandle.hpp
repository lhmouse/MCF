// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_UNIQUE_HANDLE_HPP_
#define MCF_CORE_UNIQUE_HANDLE_HPP_

#include "../Utilities/Assert.hpp"
#include <utility>
#include <type_traits>
#include <cstddef>

namespace MCF {

template<class CloserT>
class UniqueHandle {
public:
	using Handle = std::decay_t<decltype(CloserT()())>;
	using Closer = CloserT;

	static_assert(std::is_scalar<Handle>::value, "Handle must be a scalar type.");
	static_assert(noexcept(Closer()(Handle())), "Handle closer must not throw.");

private:
	Handle x_hObject;

public:
	explicit UniqueHandle(Handle rhs = Closer()()) noexcept
		: x_hObject(rhs)
	{
	}
	UniqueHandle(UniqueHandle &&rhs) noexcept
		: UniqueHandle()
	{
		Reset(std::move(rhs));
	}
	UniqueHandle &operator=(UniqueHandle &&rhs) noexcept {
		return Reset(std::move(rhs));
	}
	~UniqueHandle(){
		Reset();
	}

	UniqueHandle(const UniqueHandle &) = delete;
	UniqueHandle &operator=(const UniqueHandle &) = delete;

public:
	bool IsNonnull() const noexcept {
		return x_hObject != Closer()();
	}
	Handle Get() const noexcept {
		return x_hObject;
	}
	Handle Exchange(Handle hNewObject) noexcept {
		return std::exchange(x_hObject, hNewObject);
	}

	Handle Release() noexcept {
		return Exchange(Closer()());
	}
	void Swap(UniqueHandle &rhs) noexcept {
		std::swap(x_hObject, rhs.x_hObject);
	}

	UniqueHandle &Reset(Handle hObject = Closer()()) noexcept {
		const auto hOldObject = Exchange(hObject);
		if(hOldObject != Closer()()){
			ASSERT(hOldObject != hObject);
			Closer()(hOldObject);
		}
		return *this;
	}
	UniqueHandle &Reset(UniqueHandle &&rhs) noexcept {
		return Reset(rhs.Release());
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
	return std::equal_to<void>()(lhs.Get(), rhs.Get());
}
template<class CloserT>
bool operator==(const UniqueHandle<CloserT> &lhs, typename UniqueHandle<CloserT>::Handle rhs) noexcept {
	return std::equal_to<void>()(lhs.Get(), rhs);
}
template<class CloserT>
bool operator==(typename UniqueHandle<CloserT>::Handle lhs, const UniqueHandle<CloserT> &rhs) noexcept {
	return std::equal_to<void>()(lhs, rhs.Get());
}

template<class CloserT>
bool operator!=(const UniqueHandle<CloserT> &lhs, const UniqueHandle<CloserT> &rhs) noexcept {
	return std::not_equal_to<void>()(lhs.Get(), rhs.Get());
}
template<class CloserT>
bool operator!=(const UniqueHandle<CloserT> &lhs, typename UniqueHandle<CloserT>::Handle rhs) noexcept {
	return std::not_equal_to<void>()(lhs.Get(), rhs);
}
template<class CloserT>
bool operator!=(typename UniqueHandle<CloserT>::Handle lhs, const UniqueHandle<CloserT> &rhs) noexcept {
	return std::not_equal_to<void>()(lhs, rhs.Get());
}

template<class CloserT>
bool operator<(const UniqueHandle<CloserT> &lhs, const UniqueHandle<CloserT> &rhs) noexcept {
	return std::less<void>()(lhs.Get(), rhs.Get());
}
template<class CloserT>
bool operator<(const UniqueHandle<CloserT> &lhs, typename UniqueHandle<CloserT>::Handle rhs) noexcept {
	return std::less<void>()(lhs.Get(), rhs);
}
template<class CloserT>
bool operator<(typename UniqueHandle<CloserT>::Handle lhs, const UniqueHandle<CloserT> &rhs) noexcept {
	return std::less<void>()(lhs, rhs.Get());
}

template<class CloserT>
bool operator>(const UniqueHandle<CloserT> &lhs, const UniqueHandle<CloserT> &rhs) noexcept {
	return std::greater<void>()(lhs.Get(), rhs.Get());
}
template<class CloserT>
bool operator>(const UniqueHandle<CloserT> &lhs, typename UniqueHandle<CloserT>::Handle rhs) noexcept {
	return std::greater<void>()(lhs.Get(), rhs);
}
template<class CloserT>
bool operator>(typename UniqueHandle<CloserT>::Handle lhs, const UniqueHandle<CloserT> &rhs) noexcept {
	return std::greater<void>()(lhs, rhs.Get());
}

template<class CloserT>
bool operator<=(const UniqueHandle<CloserT> &lhs, const UniqueHandle<CloserT> &rhs) noexcept {
	return std::less_equal<void>()(lhs.Get(), rhs.Get());
}
template<class CloserT>
bool operator<=(const UniqueHandle<CloserT> &lhs, typename UniqueHandle<CloserT>::Handle rhs) noexcept {
	return std::less_equal<void>()(lhs.Get(), rhs);
}
template<class CloserT>
bool operator<=(typename UniqueHandle<CloserT>::Handle lhs, const UniqueHandle<CloserT> &rhs) noexcept {
	return std::less_equal<void>()(lhs, rhs.Get());
}

template<class CloserT>
bool operator>=(const UniqueHandle<CloserT> &lhs, const UniqueHandle<CloserT> &rhs) noexcept {
	return std::greater_equal<void>()(lhs.Get(), rhs.Get());
}
template<class CloserT>
bool operator>=(const UniqueHandle<CloserT> &lhs, typename UniqueHandle<CloserT>::Handle rhs) noexcept {
	return std::greater_equal<void>()(lhs.Get(), rhs);
}
template<class CloserT>
bool operator>=(typename UniqueHandle<CloserT>::Handle lhs, const UniqueHandle<CloserT> &rhs) noexcept {
	return std::greater_equal<void>()(lhs, rhs.Get());
}

template<class CloserT>
void swap(UniqueHandle<CloserT> &lhs, UniqueHandle<CloserT> &rhs) noexcept {
	lhs.Swap(rhs);
}

}

#endif
