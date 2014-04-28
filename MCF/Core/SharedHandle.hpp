// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef MCF_ShaRED_HANDLE_HPP_
#define MCF_ShaRED_HANDLE_HPP_

#include "../../MCFCRT/stdc/ext/assert.h"
#include "../../MCFCRT/stdc/ext/offset_of.h"
#include "../../MCFCRT/env/bail.h"
#include <new>
#include <utility>
#include <cstddef>

namespace MCF {

template<class Closer_t>
class SharedHandle;

template<class Closer_t>
class WeakHandle {
	friend SharedHandle<Closer_t>;

public:
	typedef decltype(Closer_t()()) Handle;

	static_assert(std::is_scalar<Handle>::value, "Handle must be a scalar type.");
	static_assert(noexcept(Closer_t()(Handle())), "Handle closer must not throw.");

private:
	typedef SharedHandle<Closer_t> xStrongHandle;

	class xSharedNode {
	public:
		static xSharedNode *Create(Handle hObj){
			return Recreate(nullptr, hObj);
		}
		static xSharedNode *Recreate(xSharedNode *pNode, Handle hObj){
			if(hObj == Closer_t()()){
				if(pNode && (pNode->xDropRef())){
					delete pNode;
				}
				return nullptr;
			}

			if(pNode && (pNode->xDropRef())){
				pNode->~xSharedNode();
				new(pNode) xSharedNode(hObj);
				return pNode;
			}
			xSharedNode *pNewNode;
			try {
				pNewNode = new xSharedNode(hObj);
			} catch(...){
				Closer_t()(hObj);
				throw;
			}
			return pNewNode;
		}
		static xSharedNode *AddWeakRef(xSharedNode *pNode) noexcept {
			if(pNode && pNode->xAddWeakRef()){
				return pNode;
			}
			return nullptr;
		}
		static xSharedNode *AddRef(xSharedNode *pNode) noexcept {
			if(pNode && pNode->xAddRef()){
				return pNode;
			}
			return nullptr;
		}
		static void DropRef(xSharedNode *pNode) noexcept {
			if(pNode && pNode->xDropRef()){
				delete pNode;
			}
		}
		static void DropWeakRef(xSharedNode *pNode) noexcept {
			if(pNode && pNode->xDropWeakRef()){
				delete pNode;
			}
		}

		static const Handle *ToPHandle(xSharedNode *pNode){
			if(!pNode){
				return nullptr;
			}
			return &(pNode->xm_hObj);
		}
		static xSharedNode *FromPHandle(const Handle *pHandle){
			if(!pHandle){
				return nullptr;
			}
			return DOWN_CAST(xSharedNode, xm_hObj, pHandle);
		}

	private:
		Handle xm_hObj;
		volatile std::size_t xm_uWeakCount;
		volatile std::size_t xm_uCount;

#ifndef NDEBUG
		// 用于在手动调用 AddRef() 和 DropRef() 时检测多次释放。
		xSharedNode *xm_pDebugInfo;
#endif

	private:
		explicit constexpr xSharedNode(Handle hObj) noexcept
			: xm_hObj(hObj), xm_uWeakCount(1), xm_uCount(1)
#ifndef NDEBUG
			, xm_pDebugInfo(this)
#endif
		{
		}
#ifndef NDEBUG
		~xSharedNode() noexcept {
			ASSERT(xm_pDebugInfo == this);

			xm_pDebugInfo = nullptr;
		}
#endif

	private:
		void xValidate() const noexcept {
#ifndef NDEBUG
			if(xm_pDebugInfo != this){
				MCF_CRT_Bail(L"xSharedNode::xValidate() 失败：侦测到堆损坏或二次释放。");
			}
#endif
		}

		bool xAddWeakRef() noexcept {
			xValidate();

			std::size_t uOldWeakCount = __atomic_load_n(&xm_uWeakCount, __ATOMIC_RELAXED);
			for(;;){
				if(uOldWeakCount == 0){
					return false;
				}
				if(__atomic_compare_exchange_n(&xm_uWeakCount, &uOldWeakCount, uOldWeakCount + 1, true, __ATOMIC_RELAXED, __ATOMIC_RELAXED)){
					return true;
				}
			}
		}
		bool xAddRef() noexcept {
			xValidate();

			if(!xAddWeakRef()){
				return false;
			}
			std::size_t uOldCount = __atomic_load_n(&xm_uCount, __ATOMIC_RELAXED);
			for(;;){
				if(uOldCount == 0){
					xDropWeakRef();
					return false;
				}
				if(__atomic_compare_exchange_n(&xm_uCount, &uOldCount, uOldCount + 1, true, __ATOMIC_RELAXED, __ATOMIC_RELAXED)){
					return true;
				}
			}
		}
		bool xDropRef() noexcept {
			xValidate();

			ASSERT(__atomic_load_n(&xm_uCount, __ATOMIC_RELAXED) != 0);

			if(__atomic_sub_fetch(&xm_uCount, 1, __ATOMIC_RELAXED) == 0){
				Closer_t()(xm_hObj);
				xm_hObj = Closer_t()();
			}
			return xDropWeakRef();
		}
		bool xDropWeakRef() noexcept {
			xValidate();

			ASSERT(__atomic_load_n(&xm_uWeakCount, __ATOMIC_RELAXED) != 0);
			const bool bRet = (__atomic_sub_fetch(&xm_uWeakCount, 1, __ATOMIC_RELAXED) == 0);
			ASSERT(!bRet || (__atomic_load_n(&xm_uCount, __ATOMIC_RELAXED) == 0));

			return bRet;
		}

	public:
		Handle Get() const noexcept {
			xValidate();

			return xm_hObj;
		}
		std::size_t GetRefCount() const noexcept {
			return __atomic_load_n(&xm_uCount, __ATOMIC_RELAXED);
		}
	};

private:
	xSharedNode *xm_pNode;

private:
	WeakHandle(xSharedNode *pNode) noexcept
		: xm_pNode(pNode)
	{
	}

public:
	constexpr WeakHandle() noexcept
		: xm_pNode()
	{
	}
	WeakHandle(const xStrongHandle &rhs) noexcept
		: WeakHandle(xSharedNode::AddWeakRef(rhs.xm_pNode))
	{
	}
	WeakHandle(const WeakHandle &rhs) noexcept
		: WeakHandle(xSharedNode::AddWeakRef(rhs.xm_pNode))
	{
	}
	WeakHandle(WeakHandle &&rhs) noexcept
		: WeakHandle(rhs.xm_pNode)
	{
		rhs.xm_pNode = nullptr;
	}
	WeakHandle &operator=(const xStrongHandle &rhs) noexcept {
		Reset(rhs);
		return *this;
	}
	WeakHandle &operator=(const WeakHandle &rhs) noexcept {
		Reset(rhs);
		return *this;
	}
	WeakHandle &operator=(WeakHandle &&rhs) noexcept {
		Reset(std::move(rhs));
		return *this;
	}
	~WeakHandle() noexcept {
		Reset();
	}

private:
	Handle xGetUnowned() const noexcept {
		return xm_pNode ? xm_pNode->Get() : Closer_t()();
	}

public:
	std::size_t GetRefCount() const noexcept {
		return xm_pNode ? xm_pNode->GetRefCount() : 0;
	}
	bool IsAlive() const noexcept {
		return GetRefCount() != 0;
	}
	xStrongHandle Lock() const noexcept {
		return xStrongHandle(xSharedNode::AddRef(xm_pNode));
	}

	void Reset() noexcept {
		xSharedNode::DropWeakRef(xm_pNode);
		xm_pNode = nullptr;
	}
	void Reset(const xStrongHandle &rhs) noexcept {
		if(xm_pNode != rhs.xm_pNode){
			xSharedNode::DropWeakRef(xm_pNode);
			xm_pNode = xSharedNode::AddWeakRef(rhs.xm_pNode);
		}
	}
	void Reset(const WeakHandle &rhs) noexcept {
		if((&rhs != this) && (xm_pNode != rhs.xm_pNode)){
			xSharedNode::DropWeakRef(xm_pNode);
			xm_pNode = xSharedNode::AddWeakRef(rhs.xm_pNode);
		}
	}
	void Reset(WeakHandle &&rhs) noexcept {
		if(&rhs != this){
			xSharedNode::DropWeakRef(xm_pNode);
			xm_pNode = rhs.xm_pNode;
			rhs.xm_pNode = nullptr;
		}
	}

	void Swap(WeakHandle &rhs) noexcept {
		std::swap(xm_pNode, rhs.xm_pNode);
	}

public:
	bool operator==(const WeakHandle &rhs) const noexcept {
		return xGetUnowned() == rhs.xGetUnowned();
	}
	bool operator!=(const WeakHandle &rhs) const noexcept {
		return xGetUnowned() != rhs.xGetUnowned();
	}
	bool operator<(const WeakHandle &rhs) const noexcept {
		return xGetUnowned() < rhs.xGetUnowned();
	}
	bool operator<=(const WeakHandle &rhs) const noexcept {
		return xGetUnowned() <= rhs.xGetUnowned();
	}
	bool operator>(const WeakHandle &rhs) const noexcept {
		return xGetUnowned() > rhs.xGetUnowned();
	}
	bool operator>=(const WeakHandle &rhs) const noexcept {
		return xGetUnowned() >= rhs.xGetUnowned();
	}

	bool operator==(Handle rhs) const noexcept {
		return xGetUnowned() == rhs;
	}
	bool operator!=(Handle rhs) const noexcept {
		return xGetUnowned() != rhs;
	}
	bool operator<(Handle rhs) const noexcept {
		return xGetUnowned() < rhs;
	}
	bool operator<=(Handle rhs) const noexcept {
		return xGetUnowned() <= rhs;
	}
	bool operator>(Handle rhs) const noexcept {
		return xGetUnowned() > rhs;
	}
	bool operator>=(Handle rhs) const noexcept {
		return xGetUnowned() >= rhs;
	}
};

template<class Handle_t, class Closer_t>
auto operator==(Handle_t lhs, const WeakHandle<Closer_t> &rhs) noexcept
	-> typename std::enable_if<
		std::is_same<Handle_t, typename WeakHandle<Closer_t>::Handle>::value,
		bool
	>::type
{
	return rhs == lhs;
}
template<class Handle_t, class Closer_t>
auto operator!=(Handle_t lhs, const WeakHandle<Closer_t> &rhs) noexcept
	-> typename std::enable_if<
		std::is_same<Handle_t, typename WeakHandle<Closer_t>::Handle>::value,
		bool
	>::type
{
	return rhs != lhs;
}
template<class Handle_t, class Closer_t>
auto operator<(Handle_t lhs, const WeakHandle<Closer_t> &rhs) noexcept
	-> typename std::enable_if<
		std::is_same<Handle_t, typename WeakHandle<Closer_t>::Handle>::value,
		bool
	>::type
{
	return rhs > lhs;
}
template<class Handle_t, class Closer_t>
auto operator<=(Handle_t lhs, const WeakHandle<Closer_t> &rhs) noexcept
	-> typename std::enable_if<
		std::is_same<Handle_t, typename WeakHandle<Closer_t>::Handle>::value,
		bool
	>::type
{
	return rhs >= lhs;
}
template<class Handle_t, class Closer_t>
auto operator>(Handle_t lhs, const WeakHandle<Closer_t> &rhs) noexcept
	-> typename std::enable_if<
		std::is_same<Handle_t, typename WeakHandle<Closer_t>::Handle>::value,
		bool
	>::type
{
	return rhs < lhs;
}
template<class Handle_t, class Closer_t>
auto operator>=(Handle_t lhs, const WeakHandle<Closer_t> &rhs) noexcept
	-> typename std::enable_if<
		std::is_same<Handle_t, typename WeakHandle<Closer_t>::Handle>::value,
		bool
	>::type
{
	return rhs <= lhs;
}

template<class Closer_t>
class SharedHandle {
	friend WeakHandle<Closer_t>;

public:
	typedef decltype(Closer_t()()) Handle;

	static_assert(std::is_scalar<Handle>::value, "Handle must be a scalar type.");
	static_assert(noexcept(Closer_t()(Handle())), "Handle closer must not throw.");

private:
	typedef WeakHandle<Closer_t>				xWeakHandle;
	typedef typename xWeakHandle::xSharedNode	xSharedNode;

public:
	static void AddRef(const Handle *pHandle) noexcept {
		xSharedNode::AddRef(xSharedNode::FromPHandle(pHandle));
	}
	static void DropRef(const Handle *pHandle) noexcept {
		xSharedNode::DropRef(xSharedNode::FromPHandle(pHandle));
	}

private:
	xSharedNode *xm_pNode;

private:
	explicit SharedHandle(xSharedNode *pNode) noexcept
		: xm_pNode(pNode)
	{
	}

public:
	constexpr SharedHandle() noexcept
		: xm_pNode()
	{
	}
	constexpr explicit SharedHandle(Handle hObj)
		: SharedHandle(xSharedNode::Create(hObj))
	{
	}
	explicit SharedHandle(const xWeakHandle &rhs) noexcept
		: SharedHandle(xSharedNode::AddRef(rhs.xm_pNode))
	{
	}
	SharedHandle(const SharedHandle &rhs) noexcept
		: SharedHandle(xSharedNode::AddRef(rhs.xm_pNode))
	{
	}
	SharedHandle(SharedHandle &&rhs) noexcept
		: SharedHandle(rhs.xm_pNode)
	{
		rhs.xm_pNode = nullptr;
	}
	SharedHandle &operator=(Handle hObj){
		Reset(hObj);
		return *this;
	}
	SharedHandle &operator=(const xWeakHandle &rhs) noexcept {
		Reset(rhs);
		return *this;
	}
	SharedHandle &operator=(const SharedHandle &rhs) noexcept {
		Reset(rhs);
		return *this;
	}
	SharedHandle &operator=(SharedHandle &&rhs) noexcept {
		Reset(std::move(rhs));
		return *this;
	}
	~SharedHandle() noexcept {
		Reset();
	}

public:
	bool IsGood() const noexcept {
		return Get() != Closer_t()();
	}
	Handle Get() const noexcept {
		return xm_pNode ? xm_pNode->Get() : Closer_t()();
	}
	std::size_t GetRefCount() const noexcept {
		return xm_pNode ? xm_pNode->GetRefCount() : 0;
	}
	const Handle *AddRef() const noexcept {
		return xSharedNode::ToPHandle(xSharedNode::AddRef(xm_pNode));
	}

	void Reset() noexcept {
		xSharedNode::DropRef(xm_pNode);
		xm_pNode = nullptr;
	}
	void Reset(Handle hObj){
		xm_pNode = xSharedNode::Recreate(xm_pNode, hObj);
	}
	void Reset(const xWeakHandle &rhs) noexcept {
		if(xm_pNode != rhs.xm_pNode){
			xSharedNode::DropRef(xm_pNode);
			xm_pNode = xSharedNode::AddRef(rhs.xm_pNode);
		}
	}
	void Reset(const SharedHandle &rhs) noexcept {
		if((&rhs != this) && (xm_pNode != rhs.xm_pNode)){
			xSharedNode::DropRef(xm_pNode);
			xm_pNode = xSharedNode::AddRef(rhs.xm_pNode);
		}
	}
	void Reset(SharedHandle &&rhs) noexcept {
		if(&rhs != this){
			xSharedNode::DropRef(xm_pNode);
			xm_pNode = rhs.xm_pNode;
			rhs.xm_pNode = nullptr;
		}
	}

	void Swap(SharedHandle &rhs) noexcept {
		std::swap(xm_pNode, rhs.xm_pNode);
	}

public:
	explicit operator bool() const noexcept {
		return IsGood();
	}
	explicit operator Handle() const noexcept {
		return Get();
	}

	bool operator==(const SharedHandle &rhs) const noexcept {
		return Get() == rhs.Get();
	}
	bool operator!=(const SharedHandle &rhs) const noexcept {
		return Get() != rhs.Get();
	}
	bool operator<(const SharedHandle &rhs) const noexcept {
		return Get() < rhs.Get();
	}
	bool operator<=(const SharedHandle &rhs) const noexcept {
		return Get() <= rhs.Get();
	}
	bool operator>(const SharedHandle &rhs) const noexcept {
		return Get() > rhs.Get();
	}
	bool operator>=(const SharedHandle &rhs) const noexcept {
		return Get() >= rhs.Get();
	}

	bool operator==(const xWeakHandle &rhs) const noexcept {
		return rhs == Get();
	}
	bool operator!=(const xWeakHandle &rhs) const noexcept {
		return rhs != Get();
	}
	bool operator<(const xWeakHandle &rhs) const noexcept {
		return rhs > Get();
	}
	bool operator<=(const xWeakHandle &rhs) const noexcept {
		return rhs >= Get();
	}
	bool operator>(const xWeakHandle &rhs) const noexcept {
		return rhs < Get();
	}
	bool operator>=(const xWeakHandle &rhs) const noexcept {
		return rhs <= Get();
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
auto operator==(Handle_t lhs, const SharedHandle<Closer_t> &rhs) noexcept
	-> typename std::enable_if<
		std::is_same<Handle_t, typename SharedHandle<Closer_t>::Handle>::value,
		bool
	>::type
{
	return rhs == lhs;
}
template<class Handle_t, class Closer_t>
auto operator!=(Handle_t lhs, const SharedHandle<Closer_t> &rhs) noexcept
	-> typename std::enable_if<
		std::is_same<Handle_t, typename SharedHandle<Closer_t>::Handle>::value,
		bool
	>::type
{
	return rhs != lhs;
}
template<class Handle_t, class Closer_t>
auto operator<(Handle_t lhs, const SharedHandle<Closer_t> &rhs) noexcept
	-> typename std::enable_if<
		std::is_same<Handle_t, typename SharedHandle<Closer_t>::Handle>::value,
		bool
	>::type
{
	return rhs > lhs;
}
template<class Handle_t, class Closer_t>
auto operator<=(Handle_t lhs, const SharedHandle<Closer_t> &rhs) noexcept
	-> typename std::enable_if<
		std::is_same<Handle_t, typename SharedHandle<Closer_t>::Handle>::value,
		bool
	>::type
{
	return rhs >= lhs;
}
template<class Handle_t, class Closer_t>
auto operator>(Handle_t lhs, const SharedHandle<Closer_t> &rhs) noexcept
	-> typename std::enable_if<
		std::is_same<Handle_t, typename SharedHandle<Closer_t>::Handle>::value,
		bool
	>::type
{
	return rhs < lhs;
}
template<class Handle_t, class Closer_t>
auto operator>=(Handle_t lhs, const SharedHandle<Closer_t> &rhs) noexcept
	-> typename std::enable_if<
		std::is_same<Handle_t, typename SharedHandle<Closer_t>::Handle>::value,
		bool
	>::type
{
	return rhs <= lhs;
}

template<class Closer_t>
bool operator==(const WeakHandle<Closer_t> &lhs, const SharedHandle<Closer_t> &rhs) noexcept {
	return lhs == rhs.Get();
}
template<class Closer_t>
bool operator!=(const WeakHandle<Closer_t> &lhs, const SharedHandle<Closer_t> &rhs) noexcept {
	return lhs != rhs.Get();
}
template<class Closer_t>
bool operator<(const WeakHandle<Closer_t> &lhs, const SharedHandle<Closer_t> &rhs) noexcept {
	return lhs < rhs.Get();
}
template<class Closer_t>
bool operator<=(const WeakHandle<Closer_t> &lhs, const SharedHandle<Closer_t> &rhs) noexcept {
	return lhs <= rhs.Get();
}
template<class Closer_t>
bool operator>(const WeakHandle<Closer_t> &lhs, const SharedHandle<Closer_t> &rhs) noexcept {
	return lhs > rhs.Get();
}
template<class Closer_t>
bool operator>=(const WeakHandle<Closer_t> &lhs, const SharedHandle<Closer_t> &rhs) noexcept {
	return lhs >= rhs.Get();
}

}

#endif
