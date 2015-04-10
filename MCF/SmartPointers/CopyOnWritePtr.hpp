// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_SMART_POINTERS_COPY_ON_WRITE_PTR_HPP_
#define MCF_SMART_POINTERS_COPY_ON_WRITE_PTR_HPP_

#include "PolymorphicIntrusivePtr.hpp"
#include "../Core/Exception.hpp"

namespace MCF {

template<typename ObjectT>
using CopyOnWriteBase = PolymorphicIntrusiveBase<ObjectT>;

template<typename ObjectT>
class CopyOnWritePtr {
public:
	using ElementType = typename PolymorphicIntrusivePtr<ObjectT>::ElementType;
	using BuddyType = typename PolymorphicIntrusivePtr<ObjectT>::BuddyType;

private:
	PolymorphicIntrusivePtr<ObjectT> x_pObject;

public:
	constexpr explicit CopyOnWritePtr(ElementType *pElement = nullptr) noexcept
		: x_pObject(pElement)
	{
	}
	template<typename OtherT>
	CopyOnWritePtr(PolymorphicIntrusivePtr<OtherT> rhs) noexcept
		: x_pObject(std::move(rhs))
	{
	}
	CopyOnWritePtr(const CopyOnWritePtr &rhs) noexcept
		: x_pObject(rhs.x_pObject)
	{
	}
	CopyOnWritePtr(CopyOnWritePtr &&rhs) noexcept
		: x_pObject(std::move(rhs.x_pObject))
	{
	}
	template<typename OtherT>
	CopyOnWritePtr &operator=(PolymorphicIntrusivePtr<OtherT> rhs) noexcept {
		Reset(std::move(rhs));
		return *this;
	}
	CopyOnWritePtr &operator=(const CopyOnWritePtr &rhs) noexcept {
		Reset(rhs);
		return *this;
	}
	CopyOnWritePtr &operator=(CopyOnWritePtr &&rhs) noexcept {
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
	auto ReleaseBuddy() noexcept {
		return x_pObject.ReleaseBuddy();
	}
	const ElementType *Release() noexcept {
		return x_pObject.Release();
	}
	ElementType *TakeOver(){
		if(!x_pObject){
			return nullptr;
		}
		if(x_pObject.GetSharedCount() != 1){
			x_pObject = DynamicClone<ObjectT>(x_pObject);
		}
		return x_pObject.Get();
	}

	std::size_t GetSharedCount() const noexcept {
		return x_pObject.GetSharedCount();
	}
	CopyOnWritePtr Share() const noexcept {
		return CopyOnWritePtr(*this);
	}

	CopyOnWritePtr &Reset(ElementType *pElement = nullptr) noexcept {
		x_pObject.Reset(pElement);
		return *this;
	}
	template<typename OtherT>
	CopyOnWritePtr &Reset(PolymorphicIntrusivePtr<OtherT> rhs) noexcept {
		x_pObject.Reset(std::move(rhs));
		return *this;
	}
	CopyOnWritePtr &Reset(const CopyOnWritePtr &rhs) noexcept {
		x_pObject.Reset(rhs.x_pObject);
		return *this;
	}
	CopyOnWritePtr &Reset(CopyOnWritePtr &&rhs) noexcept {
		x_pObject.Reset(std::move(rhs.x_pObject));
		return *this;
	}

	void Swap(CopyOnWritePtr &rhs) noexcept {
		std::swap(x_pObject, rhs.x_pObject);
	}

public:
	explicit operator bool() const noexcept {
		return IsNonnull();
	}
	explicit operator const ElementType *() const noexcept {
		return Get();
	}

	operator const PolymorphicIntrusivePtr<ObjectT> &() const noexcept {
		return x_pObject;
	}
	operator PolymorphicIntrusivePtr<ObjectT> &() noexcept {
		return x_pObject;
	}

	template<typename TestT = ObjectT>
	std::enable_if_t<!std::is_void<TestT>::value && !std::is_array<TestT>::value, const ElementType> &operator*() const noexcept {
		ASSERT(IsNonnull());

		return *Get();
	}
	template<typename TestT = ObjectT>
	std::enable_if_t<!std::is_void<TestT>::value && !std::is_array<TestT>::value, const ElementType> *operator->() const noexcept {
		ASSERT(IsNonnull());

		return Get();
	}
};

template<typename ObjectT>
bool operator==(const CopyOnWritePtr<ObjectT> &lhs, const CopyOnWritePtr<ObjectT> &rhs) noexcept {
	return lhs.Get() == rhs.Get();
}
template<typename ObjectT>
bool operator==(const CopyOnWritePtr<ObjectT> &lhs, ObjectT *rhs) noexcept {
	return lhs.Get() == rhs;
}
template<typename ObjectT>
bool operator==(ObjectT *lhs, const CopyOnWritePtr<ObjectT> &rhs) noexcept {
	return lhs == rhs.Get();
}

template<typename ObjectT>
bool operator!=(const CopyOnWritePtr<ObjectT> &lhs, const CopyOnWritePtr<ObjectT> &rhs) noexcept {
	return lhs.Get() != rhs.Get();
}
template<typename ObjectT>
bool operator!=(const CopyOnWritePtr<ObjectT> &lhs, ObjectT *rhs) noexcept {
	return lhs.Get() != rhs;
}
template<typename ObjectT>
bool operator!=(ObjectT *lhs, const CopyOnWritePtr<ObjectT> &rhs) noexcept {
	return lhs != rhs.Get();
}

template<typename ObjectT>
bool operator<(const CopyOnWritePtr<ObjectT> &lhs, const CopyOnWritePtr<ObjectT> &rhs) noexcept {
	return lhs.Get() < rhs.Get();
}
template<typename ObjectT>
bool operator<(const CopyOnWritePtr<ObjectT> &lhs, ObjectT *rhs) noexcept {
	return lhs.Get() < rhs;
}
template<typename ObjectT>
bool operator<(ObjectT *lhs, const CopyOnWritePtr<ObjectT> &rhs) noexcept {
	return lhs < rhs.Get();
}

template<typename ObjectT>
bool operator>(const CopyOnWritePtr<ObjectT> &lhs, const CopyOnWritePtr<ObjectT> &rhs) noexcept {
	return lhs.Get() > rhs.Get();
}
template<typename ObjectT>
bool operator>(const CopyOnWritePtr<ObjectT> &lhs, ObjectT *rhs) noexcept {
	return lhs.Get() > rhs;
}
template<typename ObjectT>
bool operator>(ObjectT *lhs, const CopyOnWritePtr<ObjectT> &rhs) noexcept {
	return lhs > rhs.Get();
}

template<typename ObjectT>
bool operator<=(const CopyOnWritePtr<ObjectT> &lhs, const CopyOnWritePtr<ObjectT> &rhs) noexcept {
	return lhs.Get() <= rhs.Get();
}
template<typename ObjectT>
bool operator<=(const CopyOnWritePtr<ObjectT> &lhs, ObjectT *rhs) noexcept {
	return lhs.Get() <= rhs;
}
template<typename ObjectT>
bool operator<=(ObjectT *lhs, const CopyOnWritePtr<ObjectT> &rhs) noexcept {
	return lhs <= rhs.Get();
}

template<typename ObjectT>
bool operator>=(const CopyOnWritePtr<ObjectT> &lhs, const CopyOnWritePtr<ObjectT> &rhs) noexcept {
	return lhs.Get() >= rhs.Get();
}
template<typename ObjectT>
bool operator>=(const CopyOnWritePtr<ObjectT> &lhs, ObjectT *rhs) noexcept {
	return lhs.Get() >= rhs;
}
template<typename ObjectT>
bool operator>=(ObjectT *lhs, const CopyOnWritePtr<ObjectT> &rhs) noexcept {
	return lhs >= rhs.Get();
}

template<typename ObjectT>
void swap(CopyOnWritePtr<ObjectT> &lhs, CopyOnWritePtr<ObjectT> &rhs) noexcept {
	lhs.Swap(rhs);
}

template<typename ObjectT, typename ...ParamsT>
auto MakeCopyOnWrite(ParamsT &&...vParams){
	static_assert(!std::is_array<ObjectT>::value, "ObjectT shall not be an array type.");

	return CopyOnWritePtr<ObjectT>(new ObjectT(std::forward<ParamsT>(vParams)...));
}

template<typename DstT, typename SrcT>
auto StaticPointerCast(CopyOnWritePtr<SrcT> rhs) noexcept {
	return CopyOnWritePtr<DstT>(StaticPointerCast<PolymorphicIntrusivePtr<SrcT>>(std::move(rhs)));
}
template<typename DstT, typename SrcT>
auto DynamicPointerCast(IntrusivePtr<SrcT> rhs) noexcept {
	return CopyOnWritePtr<DstT>(DynamicPointerCast<PolymorphicIntrusivePtr<SrcT>>(std::move(rhs)));
}
template<typename DstT, typename SrcT>
auto ConstPointerCast(IntrusivePtr<SrcT> rhs) noexcept {
	return CopyOnWritePtr<DstT>(ConstPointerCast<PolymorphicIntrusivePtr<SrcT>>(std::move(rhs)));
}

}

#endif
