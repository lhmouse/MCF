// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef MCF_ShaRED_HANDLE_NTS_HPP_
#define MCF_ShaRED_HANDLE_NTS_HPP_

#include "../../MCFCRT/std/ext/assert.h"
#include "../../MCFCRT/std/ext/offset_of.h"
#include "../../MCFCRT/env/bail.h"
#include <new>
#include <utility>
#include <cstddef>

namespace MCF {

template<class Closer_t>
class SharedHandleNts;

template<class Closer_t>
class WeakHandleNts {
	friend SharedHandleNts<Closer_t>;

public:
	typedef decltype(Closer_t()()) Handle;

	static_assert(std::is_scalar<Handle>::value, "Handle must be a scalar type");

private:
	typedef SharedHandleNts<Closer_t> xStrongHandleNts;

	class xSharedNodeNts {
	public:
		static xSharedNodeNts *Create(Handle hObj){
			return Recreate(nullptr, hObj);
		}
		static xSharedNodeNts *Recreate(xSharedNodeNts *pNode, Handle hObj){
			if(hObj == Closer_t()()){
				if(pNode && (pNode->xDropRef())){
					delete pNode;
				}
				return nullptr;
			}

			if(pNode && (pNode->xDropRef())){
				pNode->~xSharedNodeNts();
				new(pNode) xSharedNodeNts(hObj);
				return pNode;
			}
			xSharedNodeNts *pNewNode;
			try {
				pNewNode = new xSharedNodeNts(hObj);
			} catch(...){
				Closer_t()(hObj);
				throw;
			}
			return pNewNode;
		}
		static xSharedNodeNts *AddWeakRef(xSharedNodeNts *pNode) noexcept {
			if(pNode && pNode->xAddWeakRef()){
				return pNode;
			}
			return nullptr;
		}
		static xSharedNodeNts *AddRef(xSharedNodeNts *pNode) noexcept {
			if(pNode && pNode->xAddRef()){
				return pNode;
			}
			return nullptr;
		}
		static void DropRef(xSharedNodeNts *pNode) noexcept {
			if(pNode && pNode->xDropRef()){
				delete pNode;
			}
		}
		static void DropWeakRef(xSharedNodeNts *pNode) noexcept {
			if(pNode && pNode->pNode()){
				delete pNode;
			}
		}

		static const Handle *ToPHandle(xSharedNodeNts *pNode){
			if(!pNode){
				return nullptr;
			}
			return &(pNode->xm_hObj);
		}
		static xSharedNodeNts *FromPHandle(const Handle *pHandle){
			if(!pHandle){
				return nullptr;
			}
			return DOWN_CAST(xSharedNodeNts, xm_hObj, pHandle);
		}

	private:
		Handle xm_hObj;
		std::size_t xm_uWeakCount;
		std::size_t xm_uCount;

#ifndef NDEBUG
		// 用于在手动调用 AddRef() 和 DropRef() 时检测多次释放。
		xSharedNodeNts *xm_pDebugInfo;
#endif

	private:
		explicit constexpr xSharedNodeNts(Handle hObj) noexcept
			: xm_hObj(hObj), xm_uWeakCount(1), xm_uCount(1)
#ifndef NDEBUG
			, xm_pDebugInfo(this)
#endif
		{
		}
#ifndef NDEBUG
		~xSharedNodeNts(){
			ASSERT(xm_pDebugInfo == this);

			xm_pDebugInfo = nullptr;
		}
#endif

	private:
		void xValidate() const noexcept {
#ifndef NDEBUG
			if(xm_pDebugInfo != this){
				MCF_CRT_Bail(L"xSharedNodeNts::xValidate() 失败：侦测到堆损坏或二次释放。");
			}
#endif
		}

		bool xAddWeakRef() noexcept {
			xValidate();

			if(xm_uWeakCount == 0){
				return false;
			}
			++xm_uWeakCount;
			return true;
		}
		bool xAddRef() noexcept {
			xValidate();

			if(!xAddWeakRef()){
				return false;
			}
			++xm_uCount;
			return true;
		}
		bool xDropRef() noexcept {
			xValidate();

			ASSERT(xm_uCount != 0);

			if(--xm_uCount == 0){
				Closer_t()(xm_hObj);
				xm_hObj = Closer_t()();
			}
			return xDropWeakRef();
		}
		bool xDropWeakRef() noexcept {
			xValidate();

			ASSERT(xm_uWeakCount != 0);
			const bool bRet = (--xm_uWeakCount == 0);
			ASSERT(!bRet || (xm_uCount == 0));

			return bRet;
		}

	public:
		Handle Get() const noexcept {
			xValidate();

			return xm_hObj;
		}
		std::size_t GetRefCount() const noexcept {
			return xm_uCount;
		}
	};

private:
	xSharedNodeNts *xm_pNode;

private:
	WeakHandleNts(xSharedNodeNts *pNode) noexcept : xm_pNode(pNode) {
	}

public:
	constexpr WeakHandleNts() noexcept
		: xm_pNode()
	{
	}
	WeakHandleNts(const xStrongHandleNts &rhs) noexcept
		: WeakHandleNts(xSharedNodeNts::AddWeakRef(rhs.xm_pNode))
	{
	}
	WeakHandleNts(const WeakHandleNts &rhs) noexcept
		: WeakHandleNts(xSharedNodeNts::AddWeakRef(rhs.xm_pNode))
	{
	}
	WeakHandleNts(WeakHandleNts &&rhs) noexcept
		: WeakHandleNts(rhs.xm_pNode)
	{
		rhs.xm_pNode = nullptr;
	}
	WeakHandleNts &operator=(const xStrongHandleNts &rhs) noexcept {
		Reset(rhs);
		return *this;
	}
	WeakHandleNts &operator=(const WeakHandleNts &rhs) noexcept {
		Reset(rhs);
		return *this;
	}
	WeakHandleNts &operator=(WeakHandleNts &&rhs) noexcept {
		Reset(std::move(rhs));
		return *this;
	}
	~WeakHandleNts(){
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
	xStrongHandleNts Lock() const noexcept {
		return xStrongHandleNts(xSharedNodeNts::AddRef(xm_pNode));
	}

	void Reset() noexcept {
		xSharedNodeNts::DropWeakRef(xm_pNode);
		xm_pNode = nullptr;
	}
	void Reset(const xStrongHandleNts &rhs) noexcept {
		if(xm_pNode != rhs.xm_pNode){
			xSharedNodeNts::DropWeakRef(xm_pNode);
			xm_pNode = xSharedNodeNts::AddWeakRef(rhs.xm_pNode);
		}
	}
	void Reset(const WeakHandleNts &rhs) noexcept {
		if((&rhs != this) && (xm_pNode != rhs.xm_pNode)){
			xSharedNodeNts::DropWeakRef(xm_pNode);
			xm_pNode = xSharedNodeNts::AddWeakRef(rhs.xm_pNode);
		}
	}
	void Reset(WeakHandleNts &&rhs) noexcept {
		if(&rhs != this){
			xSharedNodeNts::DropWeakRef(xm_pNode);
			xm_pNode = rhs.xm_pNode;
			rhs.xm_pNode = nullptr;
		}
	}

	void Swap(WeakHandleNts &rhs) noexcept {
		std::swap(xm_pNode, rhs.xm_pNode);
	}

public:
	bool operator==(const WeakHandleNts &rhs) const noexcept {
		return xGetUnowned() == rhs.xGetUnowned();
	}
	bool operator!=(const WeakHandleNts &rhs) const noexcept {
		return xGetUnowned() != rhs.xGetUnowned();
	}
	bool operator<(const WeakHandleNts &rhs) const noexcept {
		return xGetUnowned() < rhs.xGetUnowned();
	}
	bool operator<=(const WeakHandleNts &rhs) const noexcept {
		return xGetUnowned() <= rhs.xGetUnowned();
	}
	bool operator>(const WeakHandleNts &rhs) const noexcept {
		return xGetUnowned() > rhs.xGetUnowned();
	}
	bool operator>=(const WeakHandleNts &rhs) const noexcept {
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
auto operator==(Handle_t lhs, const WeakHandleNts<Closer_t> &rhs) noexcept
	-> typename std::enable_if<
		std::is_same<Handle_t, typename WeakHandleNts<Closer_t>::Handle>::value,
		bool
	>::type
{
	return rhs == lhs;
}
template<class Handle_t, class Closer_t>
auto operator!=(Handle_t lhs, const WeakHandleNts<Closer_t> &rhs) noexcept
	-> typename std::enable_if<
		std::is_same<Handle_t, typename WeakHandleNts<Closer_t>::Handle>::value,
		bool
	>::type
{
	return rhs != lhs;
}
template<class Handle_t, class Closer_t>
auto operator<(Handle_t lhs, const WeakHandleNts<Closer_t> &rhs) noexcept
	-> typename std::enable_if<
		std::is_same<Handle_t, typename WeakHandleNts<Closer_t>::Handle>::value,
		bool
	>::type
{
	return rhs > lhs;
}
template<class Handle_t, class Closer_t>
auto operator<=(Handle_t lhs, const WeakHandleNts<Closer_t> &rhs) noexcept
	-> typename std::enable_if<
		std::is_same<Handle_t, typename WeakHandleNts<Closer_t>::Handle>::value,
		bool
	>::type
{
	return rhs >= lhs;
}
template<class Handle_t, class Closer_t>
auto operator>(Handle_t lhs, const WeakHandleNts<Closer_t> &rhs) noexcept
	-> typename std::enable_if<
		std::is_same<Handle_t, typename WeakHandleNts<Closer_t>::Handle>::value,
		bool
	>::type
{
	return rhs < lhs;
}
template<class Handle_t, class Closer_t>
auto operator>=(Handle_t lhs, const WeakHandleNts<Closer_t> &rhs) noexcept
	-> typename std::enable_if<
		std::is_same<Handle_t, typename WeakHandleNts<Closer_t>::Handle>::value,
		bool
	>::type
{
	return rhs <= lhs;
}

template<class Closer_t>
class SharedHandleNts {
	friend WeakHandleNts<Closer_t>;

public:
	typedef decltype(Closer_t()()) Handle;

	static_assert(std::is_scalar<Handle>::value, "Handle must be a scalar type");

private:
	typedef WeakHandleNts<Closer_t>					xWeakHandleNts;
	typedef typename xWeakHandleNts::xSharedNodeNts	xSharedNodeNts;

public:
	static void AddRef(const Handle *pHandle) noexcept {
		xSharedNodeNts::AddRef(xSharedNodeNts::FromPHandle(pHandle));
	}
	static void DropRef(const Handle *pHandle) noexcept {
		xSharedNodeNts::DropRef(xSharedNodeNts::FromPHandle(pHandle));
	}

private:
	xSharedNodeNts *xm_pNode;

private:
	explicit SharedHandleNts(xSharedNodeNts *pNode) noexcept
		: xm_pNode(pNode)
	{
	}

public:
	constexpr SharedHandleNts() noexcept
		: xm_pNode()
	{
	}
	constexpr explicit SharedHandleNts(Handle hObj)
		: SharedHandleNts(xSharedNodeNts::Create(hObj))
	{
	}
	explicit SharedHandleNts(const xWeakHandleNts &rhs) noexcept
		: SharedHandleNts(xSharedNodeNts::AddRef(rhs.xm_pNode))
	{
	}
	SharedHandleNts(const SharedHandleNts &rhs) noexcept
		: SharedHandleNts(xSharedNodeNts::AddRef(rhs.xm_pNode))
	{
	}
	SharedHandleNts(SharedHandleNts &&rhs) noexcept
		: SharedHandleNts(rhs.xm_pNode)
	{
		rhs.xm_pNode = nullptr;
	}
	SharedHandleNts &operator=(Handle hObj){
		Reset(hObj);
		return *this;
	}
	SharedHandleNts &operator=(const xWeakHandleNts &rhs) noexcept {
		Reset(rhs);
		return *this;
	}
	SharedHandleNts &operator=(const SharedHandleNts &rhs) noexcept {
		Reset(rhs);
		return *this;
	}
	SharedHandleNts &operator=(SharedHandleNts &&rhs) noexcept {
		Reset(std::move(rhs));
		return *this;
	}
	~SharedHandleNts(){
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
		return xSharedNodeNts::ToPHandle(xSharedNodeNts::AddRef(xm_pNode));
	}

	void Reset() noexcept {
		xSharedNodeNts::DropRef(xm_pNode);
		xm_pNode = nullptr;
	}
	void Reset(Handle hObj){
		xm_pNode = xSharedNodeNts::Recreate(xm_pNode, hObj);
	}
	void Reset(const xWeakHandleNts &rhs) noexcept {
		if(xm_pNode != rhs.xm_pNode){
			xSharedNodeNts::DropRef(xm_pNode);
			xm_pNode = xSharedNodeNts::AddRef(rhs.xm_pNode);
		}
	}
	void Reset(const SharedHandleNts &rhs) noexcept {
		if((&rhs != this) && (xm_pNode != rhs.xm_pNode)){
			xSharedNodeNts::DropRef(xm_pNode);
			xm_pNode = xSharedNodeNts::AddRef(rhs.xm_pNode);
		}
	}
	void Reset(SharedHandleNts &&rhs) noexcept {
		if(&rhs != this){
			xSharedNodeNts::DropRef(xm_pNode);
			xm_pNode = rhs.xm_pNode;
			rhs.xm_pNode = nullptr;
		}
	}

	void Swap(SharedHandleNts &rhs) noexcept {
		std::swap(xm_pNode, rhs.xm_pNode);
	}

public:
	explicit operator bool() const noexcept {
		return IsGood();
	}
	explicit operator Handle() const noexcept {
		return Get();
	}

	bool operator==(const SharedHandleNts &rhs) const noexcept {
		return Get() == rhs.Get();
	}
	bool operator!=(const SharedHandleNts &rhs) const noexcept {
		return Get() != rhs.Get();
	}
	bool operator<(const SharedHandleNts &rhs) const noexcept {
		return Get() < rhs.Get();
	}
	bool operator<=(const SharedHandleNts &rhs) const noexcept {
		return Get() <= rhs.Get();
	}
	bool operator>(const SharedHandleNts &rhs) const noexcept {
		return Get() > rhs.Get();
	}
	bool operator>=(const SharedHandleNts &rhs) const noexcept {
		return Get() >= rhs.Get();
	}

	bool operator==(const xWeakHandleNts &rhs) const noexcept {
		return rhs == Get();
	}
	bool operator!=(const xWeakHandleNts &rhs) const noexcept {
		return rhs != Get();
	}
	bool operator<(const xWeakHandleNts &rhs) const noexcept {
		return rhs > Get();
	}
	bool operator<=(const xWeakHandleNts &rhs) const noexcept {
		return rhs >= Get();
	}
	bool operator>(const xWeakHandleNts &rhs) const noexcept {
		return rhs < Get();
	}
	bool operator>=(const xWeakHandleNts &rhs) const noexcept {
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
auto operator==(Handle_t lhs, const SharedHandleNts<Closer_t> &rhs) noexcept
	-> typename std::enable_if<
		std::is_same<Handle_t, typename SharedHandleNts<Closer_t>::Handle>::value,
		bool
	>::type
{
	return rhs == lhs;
}
template<class Handle_t, class Closer_t>
auto operator!=(Handle_t lhs, const SharedHandleNts<Closer_t> &rhs) noexcept
	-> typename std::enable_if<
		std::is_same<Handle_t, typename SharedHandleNts<Closer_t>::Handle>::value,
		bool
	>::type
{
	return rhs != lhs;
}
template<class Handle_t, class Closer_t>
auto operator<(Handle_t lhs, const SharedHandleNts<Closer_t> &rhs) noexcept
	-> typename std::enable_if<
		std::is_same<Handle_t, typename SharedHandleNts<Closer_t>::Handle>::value,
		bool
	>::type
{
	return rhs > lhs;
}
template<class Handle_t, class Closer_t>
auto operator<=(Handle_t lhs, const SharedHandleNts<Closer_t> &rhs) noexcept
	-> typename std::enable_if<
		std::is_same<Handle_t, typename SharedHandleNts<Closer_t>::Handle>::value,
		bool
	>::type
{
	return rhs >= lhs;
}
template<class Handle_t, class Closer_t>
auto operator>(Handle_t lhs, const SharedHandleNts<Closer_t> &rhs) noexcept
	-> typename std::enable_if<
		std::is_same<Handle_t, typename SharedHandleNts<Closer_t>::Handle>::value,
		bool
	>::type
{
	return rhs < lhs;
}
template<class Handle_t, class Closer_t>
auto operator>=(Handle_t lhs, const SharedHandleNts<Closer_t> &rhs) noexcept
	-> typename std::enable_if<
		std::is_same<Handle_t, typename SharedHandleNts<Closer_t>::Handle>::value,
		bool
	>::type
{
	return rhs <= lhs;
}

template<class Closer_t>
bool operator==(const WeakHandleNts<Closer_t> &lhs, const SharedHandleNts<Closer_t> &rhs) noexcept {
	return lhs == rhs.Get();
}
template<class Closer_t>
bool operator!=(const WeakHandleNts<Closer_t> &lhs, const SharedHandleNts<Closer_t> &rhs) noexcept {
	return lhs != rhs.Get();
}
template<class Closer_t>
bool operator<(const WeakHandleNts<Closer_t> &lhs, const SharedHandleNts<Closer_t> &rhs) noexcept {
	return lhs < rhs.Get();
}
template<class Closer_t>
bool operator<=(const WeakHandleNts<Closer_t> &lhs, const SharedHandleNts<Closer_t> &rhs) noexcept {
	return lhs <= rhs.Get();
}
template<class Closer_t>
bool operator>(const WeakHandleNts<Closer_t> &lhs, const SharedHandleNts<Closer_t> &rhs) noexcept {
	return lhs > rhs.Get();
}
template<class Closer_t>
bool operator>=(const WeakHandleNts<Closer_t> &lhs, const SharedHandleNts<Closer_t> &rhs) noexcept {
	return lhs >= rhs.Get();
}

}

#endif
