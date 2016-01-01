// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_SMART_POINTERS_UNIQUE_PTR_HPP_
#define MCF_SMART_POINTERS_UNIQUE_PTR_HPP_

#include "../Utilities/Assert.hpp"
#include "../Function/Comparators.hpp"
#include "DefaultDeleter.hpp"
#include <utility>
#include <type_traits>
#include <cstddef>

namespace MCF {

template<typename ObjectT, class DeleterT = DefaultDeleter<ObjectT>>
class UniquePtr;

template<typename ObjectT, class DeleterT>
class UniquePtr {
	template<typename, class>
	friend class UniquePtr;

public:
	using Element = std::remove_extent_t<ObjectT>;
	using Deleter = DeleterT;

	static_assert(noexcept(Deleter()(std::declval<std::remove_cv_t<Element> *>())), "Deleter must not throw.");

private:
	Element *x_pElement;

public:
	constexpr UniquePtr(std::nullptr_t = nullptr) noexcept
		: x_pElement(nullptr)
	{
	}
	explicit UniquePtr(Element *rhs) noexcept
		: x_pElement(rhs)
	{
	}
	template<typename OtherObjectT, typename OtherDeleterT,
		std::enable_if_t<
			((!std::is_void<ObjectT>::value && !std::is_array<ObjectT>::value)
					? std::is_convertible<typename UniquePtr<OtherObjectT, OtherDeleterT>::Element *, Element *>::value
					: std::is_same<std::decay_t<OtherObjectT>, std::decay_t<ObjectT>>::value) &&
				std::is_convertible<typename UniquePtr<OtherObjectT, OtherDeleterT>::Deleter, Deleter>::value,
			int> = 0>
	UniquePtr(UniquePtr<OtherObjectT, OtherDeleterT> &&rhs) noexcept
		: x_pElement(rhs.Release())
	{
	}
	UniquePtr(UniquePtr &&rhs) noexcept
		: x_pElement(rhs.Release())
	{
	}
	UniquePtr &operator=(UniquePtr &&rhs) noexcept {
		return Reset(std::move(rhs));
	}
	~UniquePtr(){
		if(x_pElement){
			Deleter()(const_cast<std::remove_cv_t<Element> *>(x_pElement));
		}
	}

	UniquePtr(const UniquePtr &) = delete;
	UniquePtr &operator=(const UniquePtr &) = delete;

public:
	bool IsNonnull() const noexcept {
		return !!x_pElement;
	}
	Element *Get() const noexcept {
		return x_pElement;
	}
	Element *Release() noexcept {
		return std::exchange(x_pElement, nullptr);
	}

	UniquePtr &Reset(std::nullptr_t = nullptr) noexcept {
		UniquePtr().Swap(*this);
		return *this;
	}
	UniquePtr &Reset(Element *rhs) noexcept {
		UniquePtr(rhs).Swap(*this);
		return *this;
	}
	template<typename OtherObjectT, typename OtherDeleterT>
	UniquePtr &Reset(UniquePtr<OtherObjectT, OtherDeleterT> &&rhs) noexcept {
		UniquePtr(std::move(rhs)).Swap(*this);
		return *this;
	}
	UniquePtr &Reset(UniquePtr &&rhs) noexcept {
		rhs.Swap(*this);
		return *this;
	}

	void Swap(UniquePtr &rhs) noexcept {
		using std::swap;
		swap(x_pElement, rhs.x_pElement);
	}

public:
	explicit operator bool() const noexcept {
		return IsNonnull();
	}
	explicit operator Element *() const noexcept {
		return Get();
	}

	template<typename TestObjectT = ObjectT>
	auto operator*() const noexcept
		-> std::enable_if_t<
			!std::is_void<TestObjectT>::value && !std::is_array<TestObjectT>::value,
			Element> &
	{
		ASSERT(IsNonnull());

		return *Get();
	}
	template<typename TestObjectT = ObjectT>
	auto operator->() const noexcept
		-> std::enable_if_t<
			!std::is_void<TestObjectT>::value && !std::is_array<TestObjectT>::value,
			Element> *
	{
		ASSERT(IsNonnull());

		return Get();
	}
	template<typename TestObjectT = ObjectT>
	auto operator[](std::size_t uIndex) const noexcept
		-> std::enable_if_t<
			std::is_array<TestObjectT>::value,
			Element> &
	{
		ASSERT(IsNonnull());

		return Get()[uIndex];
	}
};

template<typename ObjectLhsT, typename ObjectRhsT, class DeleterT>
bool operator==(const UniquePtr<ObjectLhsT, DeleterT> &lhs, const UniquePtr<ObjectRhsT, DeleterT> &rhs) noexcept {
	return Equal()(lhs.Get(), rhs.Get());
}
template<typename ObjectLhsT, typename ObjectRhsT, class DeleterT>
bool operator==(const UniquePtr<ObjectLhsT, DeleterT> &lhs, typename UniquePtr<ObjectRhsT, DeleterT>::Element *rhs) noexcept {
	return Equal()(lhs.Get(), rhs);
}
template<typename ObjectLhsT, typename ObjectRhsT, class DeleterT>
bool operator==(typename UniquePtr<ObjectLhsT, DeleterT>::Element *lhs, const UniquePtr<ObjectRhsT, DeleterT> &rhs) noexcept {
	return Equal()(lhs, rhs.Get());
}

template<typename ObjectLhsT, typename ObjectRhsT, class DeleterT>
bool operator!=(const UniquePtr<ObjectLhsT, DeleterT> &lhs, const UniquePtr<ObjectRhsT, DeleterT> &rhs) noexcept {
	return Unequal()(lhs.Get(), rhs.Get());
}
template<typename ObjectLhsT, typename ObjectRhsT, class DeleterT>
bool operator!=(const UniquePtr<ObjectLhsT, DeleterT> &lhs, typename UniquePtr<ObjectRhsT, DeleterT>::Element *rhs) noexcept {
	return Unequal()(lhs.Get(), rhs);
}
template<typename ObjectLhsT, typename ObjectRhsT, class DeleterT>
bool operator!=(typename UniquePtr<ObjectLhsT, DeleterT>::Element *lhs, const UniquePtr<ObjectRhsT, DeleterT> &rhs) noexcept {
	return Unequal()(lhs, rhs.Get());
}

template<typename ObjectLhsT, typename ObjectRhsT, class DeleterT>
bool operator<(const UniquePtr<ObjectLhsT, DeleterT> &lhs, const UniquePtr<ObjectRhsT, DeleterT> &rhs) noexcept {
	return Less()(lhs.Get(), rhs.Get());
}
template<typename ObjectLhsT, typename ObjectRhsT, class DeleterT>
bool operator<(const UniquePtr<ObjectLhsT, DeleterT> &lhs, typename UniquePtr<ObjectRhsT, DeleterT>::Element *rhs) noexcept {
	return Less()(lhs.Get(), rhs);
}
template<typename ObjectLhsT, typename ObjectRhsT, class DeleterT>
bool operator<(typename UniquePtr<ObjectLhsT, DeleterT>::Element *lhs, const UniquePtr<ObjectRhsT, DeleterT> &rhs) noexcept {
	return Less()(lhs, rhs.Get());
}

template<typename ObjectLhsT, typename ObjectRhsT, class DeleterT>
bool operator>(const UniquePtr<ObjectLhsT, DeleterT> &lhs, const UniquePtr<ObjectRhsT, DeleterT> &rhs) noexcept {
	return Greater()(lhs.Get(), rhs.Get());
}
template<typename ObjectLhsT, typename ObjectRhsT, class DeleterT>
bool operator>(const UniquePtr<ObjectLhsT, DeleterT> &lhs, typename UniquePtr<ObjectRhsT, DeleterT>::Element *rhs) noexcept {
	return Greater()(lhs.Get(), rhs);
}
template<typename ObjectLhsT, typename ObjectRhsT, class DeleterT>
bool operator>(typename UniquePtr<ObjectLhsT, DeleterT>::Element *lhs, const UniquePtr<ObjectRhsT, DeleterT> &rhs) noexcept {
	return Greater()(lhs, rhs.Get());
}

template<typename ObjectLhsT, typename ObjectRhsT, class DeleterT>
bool operator<=(const UniquePtr<ObjectLhsT, DeleterT> &lhs, const UniquePtr<ObjectRhsT, DeleterT> &rhs) noexcept {
	return LessEqual()(lhs.Get(), rhs.Get());
}
template<typename ObjectLhsT, typename ObjectRhsT, class DeleterT>
bool operator<=(const UniquePtr<ObjectLhsT, DeleterT> &lhs, typename UniquePtr<ObjectRhsT, DeleterT>::Element *rhs) noexcept {
	return LessEqual()(lhs.Get(), rhs);
}
template<typename ObjectLhsT, typename ObjectRhsT, class DeleterT>
bool operator<=(typename UniquePtr<ObjectLhsT, DeleterT>::Element *lhs, const UniquePtr<ObjectRhsT, DeleterT> &rhs) noexcept {
	return LessEqual()(lhs, rhs.Get());
}

template<typename ObjectLhsT, typename ObjectRhsT, class DeleterT>
bool operator>=(const UniquePtr<ObjectLhsT, DeleterT> &lhs, const UniquePtr<ObjectRhsT, DeleterT> &rhs) noexcept {
	return GreaterEqual()(lhs.Get(), rhs.Get());
}
template<typename ObjectLhsT, typename ObjectRhsT, class DeleterT>
bool operator>=(const UniquePtr<ObjectLhsT, DeleterT> &lhs, typename UniquePtr<ObjectRhsT, DeleterT>::Element *rhs) noexcept {
	return GreaterEqual()(lhs.Get(), rhs);
}
template<typename ObjectLhsT, typename ObjectRhsT, class DeleterT>
bool operator>=(typename UniquePtr<ObjectLhsT, DeleterT>::Element *lhs, const UniquePtr<ObjectRhsT, DeleterT> &rhs) noexcept {
	return GreaterEqual()(lhs, rhs.Get());
}

template<typename ObjectT, class DeleterT>
void swap(UniquePtr<ObjectT, DeleterT> &lhs, UniquePtr<ObjectT, DeleterT> &rhs) noexcept {
	lhs.Swap(rhs);
}

template<typename ObjectT, typename DeleterT = DefaultDeleter<ObjectT>, typename ...ParamsT>
UniquePtr<ObjectT, DeleterT> MakeUnique(ParamsT &&...vParams){
	static_assert(!std::is_array<ObjectT>::value, "ObjectT shall not be an array type.");
	static_assert(!std::is_reference<ObjectT>::value, "ObjectT shall not be a reference type.");

	return UniquePtr<ObjectT, DeleterT>(new ObjectT(std::forward<ParamsT>(vParams)...));
}

template<typename DstT, typename SrcT, class DeleterT>
UniquePtr<DstT, DeleterT> StaticPointerCast(UniquePtr<SrcT, DeleterT> &&pSrc) noexcept {
	return UniquePtr<DstT, DeleterT>(static_cast<DstT *>(pSrc.Release()));
}
template<typename DstT, typename SrcT, class DeleterT>
UniquePtr<DstT, DeleterT> ConstPointerCast(UniquePtr<SrcT, DeleterT> &&pSrc) noexcept {
	return UniquePtr<DstT, DeleterT>(const_cast<DstT *>(pSrc.Release()));
}

template<typename DstT, typename SrcT, class DeleterT>
UniquePtr<DstT, DeleterT> DynamicPointerCast(UniquePtr<SrcT, DeleterT> &&pSrc) noexcept {
	const auto pTest = dynamic_cast<DstT *>(pSrc.Get());
	if(!pTest){
		return nullptr;
	}
	pSrc.Release();
	return UniquePtr<DstT, DeleterT>(pTest);
}

}

#endif
