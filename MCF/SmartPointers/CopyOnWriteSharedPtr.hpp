// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_SMART_POINTERS_COPY_ON_WRITE_SHARED_PTR_HPP_
#define MCF_SMART_POINTERS_COPY_ON_WRITE_SHARED_PTR_HPP_

#include "PolymorphicSharedPtr.hpp"

namespace MCF {

template<typename ObjectT>
using CopyOnWriteWeakPtr = PolymorphicWeakPtr<ObjectT>;

template<typename ObjectT>
class CopyOnWriteSharedPtr {
public:
	using Raw = typename PolymorphicSharedPtr<ObjectT>::Raw;
	using ElementType = typename PolymorphicSharedPtr<ObjectT>::ElementType;

private:
	PolymorphicSharedPtr<ObjectT> x_pObject;

public:
	explicit constexpr CopyOnWriteSharedPtr(std::nullptr_t = nullptr) noexcept
		: x_pObject(nullptr)
	{
	}
	template<typename OtherT>
	CopyOnWriteSharedPtr(PolymorphicSharedPtr<OtherT> rhs) noexcept
		: x_pObject(std::move(rhs))
	{
	}
	CopyOnWriteSharedPtr(const CopyOnWriteSharedPtr &rhs) noexcept
		: x_pObject(rhs.x_pObject)
	{
	}
	CopyOnWriteSharedPtr(CopyOnWriteSharedPtr &&rhs) noexcept
		: x_pObject(std::move(rhs.x_pObject))
	{
	}
	template<typename OtherT>
	CopyOnWriteSharedPtr &operator=(PolymorphicSharedPtr<OtherT> rhs) noexcept {
		Reset(std::move(rhs));
		return *this;
	}
	CopyOnWriteSharedPtr &operator=(const CopyOnWriteSharedPtr &rhs) noexcept {
		Reset(rhs);
		return *this;
	}
	CopyOnWriteSharedPtr &operator=(CopyOnWriteSharedPtr &&rhs) noexcept {
		Reset(std::move(rhs));
		return *this;
	}

public:
	bool IsOwning() const noexcept {
		return x_pObject.IsOwning();
	}
	const Raw *GetRaw() const noexcept {
		return x_pObject.GetRaw();
	}
	bool IsNonnull() const noexcept {
		return x_pObject.IsNonnull();
	}
	const ElementType *Get() const noexcept {
		return x_pObject.Get();
	}

	Raw *TakeOverRaw(){
		if(!x_pObject){
			return nullptr;
		}
		if(!x_pObject.IsUnique()){
			x_pObject = DynamicClone<ObjectT>(x_pObject);
		}
		return x_pObject.GetRaw();
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
	std::size_t GetWeakCount() const noexcept {
		return x_pObject.GetWeakCount();
	}

	template<typename OtherObjectT, class OtherDeleterT>
	bool IsSharedWith(const SharedPtr<OtherObjectT, OtherDeleterT> &rhs) const noexcept {
		return x_pObject.IsSharedWith(rhs);
	}
	template<typename OtherObjectT, class OtherDeleterT>
	bool IsSharedWith(const WeakPtr<OtherObjectT, OtherDeleterT> &rhs) const noexcept {
		return x_pObject.IsSharedWith(rhs);
	}

	CopyOnWriteSharedPtr &Reset(std::nullptr_t = nullptr) noexcept {
		x_pObject.Reset(nullptr);
		return *this;
	}
	template<typename OtherT>
	CopyOnWriteSharedPtr &Reset(PolymorphicSharedPtr<OtherT> rhs) noexcept {
		x_pObject.Reset(std::move(rhs));
		return *this;
	}
	CopyOnWriteSharedPtr &Reset(const CopyOnWriteSharedPtr &rhs) noexcept {
		x_pObject.Reset(rhs.x_pObject);
		return *this;
	}
	CopyOnWriteSharedPtr &Reset(CopyOnWriteSharedPtr &&rhs) noexcept {
		x_pObject.Reset(std::move(rhs.x_pObject));
		return *this;
	}

	void Swap(CopyOnWriteSharedPtr &rhs) noexcept {
		x_pObject.Swap(rhs.x_pObject);
	}

public:
	explicit operator bool() const noexcept {
		return IsNonnull();
	}
	explicit operator const ElementType *() const noexcept {
		return Get();
	}

	operator const PolymorphicSharedPtr<ObjectT> &() const & noexcept {
		return x_pObject;
	}
	operator PolymorphicSharedPtr<ObjectT> &() & noexcept {
		return x_pObject;
	}
	operator PolymorphicSharedPtr<ObjectT> &&() && noexcept {
		return std::move(x_pObject);
	}

	operator PolymorphicWeakPtr<ObjectT>() const noexcept {
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
	template<typename TestT = ObjectT>
	std::enable_if_t<std::is_array<TestT>::value, const ElementType> &operator[](std::size_t uIndex) const noexcept {
		ASSERT(IsNonnull());

		return Get()[uIndex];
	}
};

#define MCF_SMART_POINTERS_DECLARE_TEMPLATE_PARAMETERS_	template<typename ObjectT>
#define MCF_SMART_POINTERS_INVOKE_TEMPLATE_PARAMETERS_	CopyOnWriteSharedPtr<ObjectT>
#include "_RationalAndSwap.hpp"

template<typename ObjectT, typename ...ParamsT>
auto MakeCopyOnWriteShared(ParamsT &&...vParams){
	return CopyOnWriteSharedPtr<ObjectT>(MakePolymorphicShared<ObjectT>(std::forward<ParamsT>(vParams)...));
}

template<typename DstT, typename SrcT>
auto StaticPointerCast(CopyOnWriteSharedPtr<SrcT> rhs) noexcept {
	return CopyOnWriteSharedPtr<DstT>(StaticPointerCast<PolymorphicSharedPtr<SrcT>>(std::move(rhs)));
}
template<typename DstT, typename SrcT>
auto DynamicPointerCast(CopyOnWriteSharedPtr<SrcT> rhs) noexcept {
	return CopyOnWriteSharedPtr<DstT>(DynamicPointerCast<PolymorphicSharedPtr<SrcT>>(std::move(rhs)));
}
template<typename DstT, typename SrcT>
auto ConstPointerCast(CopyOnWriteSharedPtr<SrcT> rhs) noexcept {
	return CopyOnWriteSharedPtr<DstT>(ConstPointerCast<PolymorphicSharedPtr<SrcT>>(std::move(rhs)));
}

}

#endif
