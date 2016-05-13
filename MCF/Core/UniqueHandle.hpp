// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_UNIQUE_HANDLE_HPP_
#define MCF_CORE_UNIQUE_HANDLE_HPP_

#include "../Utilities/Assert.hpp"
#include "../Utilities/DeclVal.hpp"
#include "../Function/Comparators.hpp"
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
	static_assert(noexcept(Closer()(DeclVal<Handle>())), "Handle closer must not throw.");

private:
	Handle x_hObject;

private:
	void X_Dispose() noexcept {
		const auto hObject = x_hObject;
		if(hObject){
			Closer()(hObject);
		}
#ifndef NDEBUG
		__builtin_memset(&x_hObject, 0xEF, sizeof(x_hObject));
#endif
	}

public:
	constexpr UniqueHandle() noexcept
		: x_hObject(Closer()())
	{
	}
	explicit constexpr UniqueHandle(Handle rhs) noexcept
		: x_hObject(rhs)
	{
	}
	UniqueHandle(UniqueHandle &&rhs) noexcept
		: x_hObject(rhs.Release())
	{
	}
	UniqueHandle &operator=(UniqueHandle &&rhs) noexcept {
		return Reset(std::move(rhs));
	}
	~UniqueHandle(void){
		X_Dispose();
	}

	UniqueHandle(const UniqueHandle &) = delete;
	UniqueHandle &operator=(const UniqueHandle &) = delete;

public:
	constexpr bool IsNull() const noexcept {
		return x_hObject == Closer()();
	}
	constexpr Handle Get() const noexcept {
		return x_hObject;
	}
	Handle Release() noexcept {
		return std::exchange(x_hObject, Closer()());
	}

	UniqueHandle &Reset(Handle rhs = Closer()()) noexcept {
		UniqueHandle(rhs).Swap(*this);
		return *this;
	}
	UniqueHandle &Reset(UniqueHandle &&rhs) noexcept {
		UniqueHandle(std::move(rhs)).Swap(*this);
		return *this;
	}

	void Swap(UniqueHandle &rhs) noexcept {
		using std::swap;
		swap(x_hObject, rhs.x_hObject);
	}

public:
	explicit constexpr operator bool() const noexcept {
		return !IsNull();
	}
	explicit constexpr operator Handle() const noexcept {
		return Get();
	}

	template<class CloserRhsT>
	constexpr bool operator==(const UniqueHandle<CloserRhsT> &rhs) const noexcept {
		return Equal()(x_hObject, rhs.x_hObject);
	}
	constexpr bool operator==(Handle rhs) const noexcept {
		return Equal()(x_hObject, rhs);
	}
	friend constexpr bool operator==(Handle lhs, const UniqueHandle &rhs) noexcept {
		return Equal()(lhs, rhs.x_hObject);
	}

	template<class CloserRhsT>
	constexpr bool operator!=(const UniqueHandle<CloserRhsT> &rhs) const noexcept {
		return Unequal()(x_hObject, rhs.x_hObject);
	}
	constexpr bool operator!=(Handle rhs) const noexcept {
		return Unequal()(x_hObject, rhs);
	}
	friend constexpr bool operator!=(Handle lhs, const UniqueHandle &rhs) noexcept {
		return Unequal()(lhs, rhs.x_hObject);
	}

	template<class CloserRhsT>
	constexpr bool operator<(const UniqueHandle<CloserRhsT> &rhs) const noexcept {
		return Less()(x_hObject, rhs.x_hObject);
	}
	constexpr bool operator<(Handle rhs) const noexcept {
		return Less()(x_hObject, rhs);
	}
	friend constexpr bool operator<(Handle lhs, const UniqueHandle &rhs) noexcept {
		return Less()(lhs, rhs.x_hObject);
	}

	template<class CloserRhsT>
	constexpr bool operator>(const UniqueHandle<CloserRhsT> &rhs) const noexcept {
		return Greater()(x_hObject, rhs.x_hObject);
	}
	constexpr bool operator>(Handle rhs) const noexcept {
		return Greater()(x_hObject, rhs);
	}
	friend constexpr bool operator>(Handle lhs, const UniqueHandle &rhs) noexcept {
		return Greater()(lhs, rhs.x_hObject);
	}

	template<class CloserRhsT>
	constexpr bool operator<=(const UniqueHandle<CloserRhsT> &rhs) const noexcept {
		return LessEqual()(x_hObject, rhs.x_hObject);
	}
	constexpr bool operator<=(Handle rhs) const noexcept {
		return LessEqual()(x_hObject, rhs);
	}
	friend constexpr bool operator<=(Handle lhs, const UniqueHandle &rhs) noexcept {
		return LessEqual()(lhs, rhs.x_hObject);
	}

	template<class CloserRhsT>
	constexpr bool operator>=(const UniqueHandle<CloserRhsT> &rhs) const noexcept {
		return GreaterEqual()(x_hObject, rhs.x_hObject);
	}
	constexpr bool operator>=(Handle rhs) const noexcept {
		return GreaterEqual()(x_hObject, rhs);
	}
	friend constexpr bool operator>=(Handle lhs, const UniqueHandle &rhs) noexcept {
		return GreaterEqual()(lhs, rhs.x_hObject);
	}

	friend void swap(UniqueHandle &lhs, UniqueHandle &rhs) noexcept {
		lhs.Swap(rhs);
	}
};

}

#endif
