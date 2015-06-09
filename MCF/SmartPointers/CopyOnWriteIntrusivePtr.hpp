// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_SMART_POINTERS_COPY_ON_WRITE_INTRUSIVE_PTR_HPP_
#define MCF_SMART_POINTERS_COPY_ON_WRITE_INTRUSIVE_PTR_HPP_

#include "PolyIntrusivePtr.hpp"

namespace MCF {

template<typename ObjectT>
using CopyOnWriteIntrusiveBase = PolyIntrusiveBase<ObjectT>;

template<typename ObjectT>
class CopyOnWriteIntrusivePtr {
public:
	using ElementType = typename PolyIntrusivePtr<ObjectT>::ElementType;
	using BuddyType = typename PolyIntrusivePtr<ObjectT>::BuddyType;

private:
	PolyIntrusivePtr<ObjectT> x_pObject;

public:
	explicit constexpr CopyOnWriteIntrusivePtr(ElementType *pElement = nullptr) noexcept
		: x_pObject(pElement)
	{
	}
	template<typename OtherT>
	CopyOnWriteIntrusivePtr(PolyIntrusivePtr<OtherT> rhs) noexcept
		: x_pObject(std::move(rhs))
	{
	}
	CopyOnWriteIntrusivePtr(const CopyOnWriteIntrusivePtr &rhs) noexcept
		: x_pObject(rhs.x_pObject)
	{
	}
	CopyOnWriteIntrusivePtr(CopyOnWriteIntrusivePtr &&rhs) noexcept
		: x_pObject(std::move(rhs.x_pObject))
	{
	}
	template<typename OtherT>
	CopyOnWriteIntrusivePtr &operator=(PolyIntrusivePtr<OtherT> rhs) noexcept {
		Reset(std::move(rhs));
		return *this;
	}
	CopyOnWriteIntrusivePtr &operator=(const CopyOnWriteIntrusivePtr &rhs) noexcept {
		Reset(rhs);
		return *this;
	}
	CopyOnWriteIntrusivePtr &operator=(CopyOnWriteIntrusivePtr &&rhs) noexcept {
		Reset(std::move(rhs));
		return *this;
	}

public:
	bool IsNonnull() const noexcept {
		return x_pObject.IsNonnull();
	}
	const ElementType *Get() const noexcept {
		return x_pObject.Get();
	}
	BuddyType *ReleaseBuddy() noexcept {
		return x_pObject.ReleaseBuddy();
	}
	const ElementType *Release() noexcept {
		return x_pObject.Release();
	}

	ElementType *TakeOver(){
		if(!x_pObject){
			return nullptr;
		}
		if(!x_pObject.IsUnique()){
			x_pObject = DynamicClone<ObjectT>(x_pObject);
		}
		return x_pObject.Get();
	}

	bool IsUnique() const noexcept {
		return x_pObject.IsUnique();
	}
	std::size_t GetSharedCount() const noexcept {
		return x_pObject.GetSharedCount();
	}

	CopyOnWriteIntrusivePtr &Reset(ElementType *pElement = nullptr) noexcept {
		x_pObject.Reset(pElement);
		return *this;
	}
	template<typename OtherT>
	CopyOnWriteIntrusivePtr &Reset(PolyIntrusivePtr<OtherT> rhs) noexcept {
		x_pObject.Reset(std::move(rhs));
		return *this;
	}
	CopyOnWriteIntrusivePtr &Reset(const CopyOnWriteIntrusivePtr &rhs) noexcept {
		x_pObject.Reset(rhs.x_pObject);
		return *this;
	}
	CopyOnWriteIntrusivePtr &Reset(CopyOnWriteIntrusivePtr &&rhs) noexcept {
		x_pObject.Reset(std::move(rhs.x_pObject));
		return *this;
	}

	void Swap(CopyOnWriteIntrusivePtr &rhs) noexcept {
		x_pObject.Swap(rhs.x_pObject);
	}

public:
	explicit operator bool() const noexcept {
		return IsNonnull();
	}
	explicit operator const ElementType *() const noexcept {
		return Get();
	}

	operator const PolyIntrusivePtr<ObjectT> &() const & noexcept {
		return x_pObject;
	}
	operator PolyIntrusivePtr<ObjectT> &() & noexcept {
		return x_pObject;
	}
	operator PolyIntrusivePtr<ObjectT> &&() && noexcept {
		return std::move(x_pObject);
	}

	template<typename TestT = ObjectT>
	std::enable_if_t<!std::is_void<TestT>::value, const ElementType> &operator*() const noexcept {
		ASSERT(IsNonnull());

		return *Get();
	}
	template<typename TestT = ObjectT>
	std::enable_if_t<!std::is_void<TestT>::value, const ElementType> *operator->() const noexcept {
		ASSERT(IsNonnull());

		return Get();
	}
};

#define MCF_SMART_POINTERS_DECLARE_TEMPLATE_PARAMETERS_	template<typename ObjectT>
#define MCF_SMART_POINTERS_INVOKE_TEMPLATE_PARAMETERS_	CopyOnWriteIntrusivePtr<ObjectT>
#include "_RationalAndSwap.hpp"

template<typename ObjectT, typename ...ParamsT>
CopyOnWriteIntrusivePtr<ObjectT> MakeCopyOnWriteIntrusive(ParamsT &&...vParams){
	static_assert(!std::is_array<ObjectT>::value, "ObjectT shall not be an array type.");
	static_assert(!std::is_reference<ObjectT>::value, "ObjectT shall not be a reference type.");

	return CopyOnWriteIntrusivePtr<ObjectT>(new ObjectT(std::forward<ParamsT>(vParams)...));
}

template<typename DstT, typename SrcT>
CopyOnWriteIntrusivePtr<DstT> StaticPointerCast(CopyOnWriteIntrusivePtr<SrcT> rhs) noexcept {
	return CopyOnWriteIntrusivePtr<DstT>(StaticPointerCast<PolyIntrusivePtr<SrcT>>(std::move(rhs)));
}
template<typename DstT, typename SrcT>
CopyOnWriteIntrusivePtr<DstT> DynamicPointerCast(CopyOnWriteIntrusivePtr<SrcT> rhs) noexcept {
	return CopyOnWriteIntrusivePtr<DstT>(DynamicPointerCast<PolyIntrusivePtr<SrcT>>(std::move(rhs)));
}
template<typename DstT, typename SrcT>
CopyOnWriteIntrusivePtr<DstT> ConstPointerCast(CopyOnWriteIntrusivePtr<SrcT> rhs) noexcept {
	return CopyOnWriteIntrusivePtr<DstT>(ConstPointerCast<PolyIntrusivePtr<SrcT>>(std::move(rhs)));
}

}

#endif
