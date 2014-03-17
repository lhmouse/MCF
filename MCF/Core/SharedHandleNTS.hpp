// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef __MCF_SHARED_HANDLE_NTS_HPP__
#define __MCF_SHARED_HANDLE_NTS_HPP__

#include "../../MCFCRT/c/ext/assert.h"
#include "../../MCFCRT/c/ext/offset_of.h"
#include "../../MCFCRT/env/bail.h"
#include <new>
#include <utility>
#include <cstddef>

namespace MCF {

namespace __MCF {
	template<class Closer_t>
	class SharedNodeNTS {
	public:
		typedef decltype(Closer_t()()) Handle;

	public:
		static SharedNodeNTS *Create(Handle hObj){
			return Recreate(nullptr, hObj);
		}
		static SharedNodeNTS *Recreate(SharedNodeNTS *pNode, Handle hObj){
			if(hObj == Closer_t()()){
				if(pNode && (pNode->xDropRef())){
					delete pNode;
				}
				return nullptr;
			}

			if(pNode && (pNode->xDropRef())){
				pNode->~SharedNodeNTS();
				new(pNode) SharedNodeNTS(hObj);
				return pNode;
			}
			SharedNodeNTS *pNewNode;
			try {
				pNewNode = new SharedNodeNTS(hObj);
			} catch(...){
				Closer_t()(hObj);
				throw;
			}
			return pNewNode;
		}
		static SharedNodeNTS *AddWeakRef(SharedNodeNTS *pNode) noexcept {
			if(pNode && pNode->xAddWeakRef()){
				return pNode;
			}
			return nullptr;
		}
		static SharedNodeNTS *AddRef(SharedNodeNTS *pNode) noexcept {
			if(pNode && pNode->xAddRef()){
				return pNode;
			}
			return nullptr;
		}
		static void DropRef(SharedNodeNTS *pNode) noexcept {
			if(pNode && pNode->xDropRef()){
				delete pNode;
			}
		}
		static void DropWeakRef(SharedNodeNTS *pNode) noexcept {
			if(pNode && pNode->pNode()){
				delete pNode;
			}
		}

		static const Handle *ToPHandle(SharedNodeNTS *pNode){
			if(pNode){
				return (const Handle *)((std::intptr_t)pNode + OFFSET_OF(SharedNodeNTS, xm_hObj));
			}
			return nullptr;
		}
		static SharedNodeNTS *FromPHandle(const Handle *pHandle){
			if(pHandle){
				return (SharedNodeNTS *)((std::intptr_t)pHandle - OFFSET_OF(SharedNodeNTS, xm_hObj));
			}
			return nullptr;
		}

	private:
		Handle xm_hObj;
		std::size_t xm_uWeakCount;
		std::size_t xm_uCount;
#ifndef NDEBUG
		// 用于在手动调用 AddRef() 和 DropRef() 时检测多次释放。
		SharedNodeNTS *xm_pDebugInfo;
#endif

	private:
		explicit constexpr SharedNodeNTS(Handle hObj) noexcept
			: xm_hObj(hObj), xm_uWeakCount(1), xm_uCount(1)
#ifndef NDEBUG
			, xm_pDebugInfo(this)
#endif
		{
		}
#ifndef NDEBUG
		~SharedNodeNTS(){
			ASSERT(xm_pDebugInfo == this);

			xm_pDebugInfo = nullptr;
		}
#endif

	private:
		void xValidate() const noexcept {
#ifndef NDEBUG
			if(xm_pDebugInfo != this){
				__MCF_CRT_Bail(L"SharedNodeNTS::xValidate() 失败：侦测到堆损坏或二次释放。");
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
}

template<class Closer_t>
class SharedHandleNTS;

template<class Closer_t>
class WeakHandleNTS {
	friend class SharedHandleNTS<Closer_t>;

public:
	typedef decltype(Closer_t()()) Handle;

private:
	typedef __MCF::SharedNodeNTS<Closer_t> xSharedNodeNTS;
	typedef SharedHandleNTS<Closer_t> xStrongHandleNTS;

private:
	xSharedNodeNTS *xm_pNode;

private:
	WeakHandleNTS(xSharedNodeNTS *pNode) noexcept : xm_pNode(pNode) {
	}

public:
	constexpr WeakHandleNTS() noexcept : xm_pNode() {
	}
	WeakHandleNTS(const xStrongHandleNTS &rhs) noexcept : WeakHandleNTS(xSharedNodeNTS::AddWeakRef(rhs.xm_pNode)) {
	}
	WeakHandleNTS(const WeakHandleNTS &rhs) noexcept : WeakHandleNTS(xSharedNodeNTS::AddWeakRef(rhs.xm_pNode)) {
	}
	WeakHandleNTS(WeakHandleNTS &&rhs) noexcept : WeakHandleNTS(rhs.xm_pNode) {
		rhs.xm_pNode = nullptr;
	}
	WeakHandleNTS &operator=(const xStrongHandleNTS &rhs) noexcept {
		Reset(rhs);
		return *this;
	}
	WeakHandleNTS &operator=(const WeakHandleNTS &rhs) noexcept {
		Reset(rhs);
		return *this;
	}
	WeakHandleNTS &operator=(WeakHandleNTS &&rhs) noexcept {
		Reset(std::move(rhs));
		return *this;
	}
	~WeakHandleNTS(){
		Reset();
	}

public:
	xStrongHandleNTS Lock() const noexcept {
		return xStrongHandleNTS(xSharedNodeNTS::AddRef(xm_pNode));
	}

	void Reset() noexcept {
		xSharedNodeNTS::DropWeakRef(xm_pNode);
		xm_pNode = nullptr;
	}
	void Reset(const xStrongHandleNTS &rhs) noexcept {
		xSharedNodeNTS::DropWeakRef(xm_pNode);
		xm_pNode = xSharedNodeNTS::AddWeakRef(rhs.xm_pNode);
	}
	void Reset(const WeakHandleNTS &rhs) noexcept {
		if(&rhs == this){
			return;
		}
		xSharedNodeNTS::DropWeakRef(xm_pNode);
		xm_pNode = xSharedNodeNTS::AddWeakRef(rhs.xm_pNode);
	}
	void Reset(WeakHandleNTS &&rhs) noexcept {
		if(&rhs == this){
			return;
		}
		xSharedNodeNTS::DropWeakRef(xm_pNode);
		xm_pNode = rhs.xm_pNode;
		rhs.xm_pNode = nullptr;
	}

	void Swap(WeakHandleNTS &rhs) noexcept {
		if(&rhs == this){
			return;
		}
		std::swap(xm_pNode, rhs.xm_pNode);
	}
};

template<class Closer_t>
class SharedHandleNTS {
	friend class WeakHandleNTS<Closer_t>;

public:
	typedef decltype(Closer_t()()) Handle;

private:
	typedef __MCF::SharedNodeNTS<Closer_t> xSharedNodeNTS;
	typedef WeakHandleNTS<Closer_t> xWeakHandleNTS;

public:
	static void AddRef(const Handle *pHandle) noexcept {
		xSharedNodeNTS::AddRef(xSharedNodeNTS::FromPHandle(pHandle));
	}
	static void DropRef(const Handle *pHandle) noexcept {
		xSharedNodeNTS::DropRef(xSharedNodeNTS::FromPHandle(pHandle));
	}

private:
	xSharedNodeNTS *xm_pNode;

private:
	SharedHandleNTS(xSharedNodeNTS *pNode) noexcept : xm_pNode(pNode) {
	}

public:
	constexpr SharedHandleNTS() noexcept : xm_pNode() {
	}
	constexpr explicit SharedHandleNTS(Handle hObj) noexcept : SharedHandleNTS(xSharedNodeNTS::Create(hObj)) {
	}
	explicit SharedHandleNTS(const xWeakHandleNTS &rhs) noexcept : SharedHandleNTS(xSharedNodeNTS::AddRef(rhs.xm_pNode)) {
	}
	SharedHandleNTS(const SharedHandleNTS &rhs) noexcept : SharedHandleNTS(xSharedNodeNTS::AddRef(rhs.xm_pNode)) {
	}
	SharedHandleNTS(SharedHandleNTS &&rhs) noexcept : SharedHandleNTS(rhs.xm_pNode) {
		rhs.xm_pNode = nullptr;
	}
	SharedHandleNTS &operator=(Handle hObj){
		Reset(hObj);
		return *this;
	}
	SharedHandleNTS &operator=(const xWeakHandleNTS &rhs) noexcept {
		Reset(rhs);
		return *this;
	}
	SharedHandleNTS &operator=(const SharedHandleNTS &rhs) noexcept {
		Reset(rhs);
		return *this;
	}
	SharedHandleNTS &operator=(SharedHandleNTS &&rhs) noexcept {
		Reset(std::move(rhs));
		return *this;
	}
	~SharedHandleNTS(){
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
		return xSharedNodeNTS::ToPHandle(xSharedNodeNTS::AddRef(xm_pNode));
	}

	void Reset() noexcept {
		xSharedNodeNTS::DropRef(xm_pNode);
		xm_pNode = nullptr;
	}
	void Reset(Handle hObj){
		xm_pNode = xSharedNodeNTS::Recreate(xm_pNode, hObj);
	}
	void Reset(const xWeakHandleNTS &rhs) noexcept {
		xSharedNodeNTS::DropRef(xm_pNode);
		xm_pNode = xSharedNodeNTS::AddRef(rhs.xm_pNode);
	}
	void Reset(const SharedHandleNTS &rhs) noexcept {
		if(&rhs == this){
			return;
		}
		xSharedNodeNTS::DropRef(xm_pNode);
		xm_pNode = xSharedNodeNTS::AddRef(rhs.xm_pNode);
	}
	void Reset(SharedHandleNTS &&rhs) noexcept {
		if(&rhs == this){
			return;
		}
		xSharedNodeNTS::DropRef(xm_pNode);
		xm_pNode = rhs.xm_pNode;
		rhs.xm_pNode = nullptr;
	}

	void Swap(SharedHandleNTS &rhs) noexcept {
		if(&rhs == this){
			return;
		}
		std::swap(xm_pNode, rhs.xm_pNode);
	}

public:
	explicit operator bool() const noexcept {
		return IsGood();
	}
	explicit operator Handle() const noexcept {
		return Get();
	}

	bool operator==(const SharedHandleNTS &rhs) const noexcept {
		return Get() == rhs.Get();
	}
	bool operator!=(const SharedHandleNTS &rhs) const noexcept {
		return Get() != rhs.Get();
	}
	bool operator<(const SharedHandleNTS &rhs) const noexcept {
		return Get() < rhs.Get();
	}
	bool operator<=(const SharedHandleNTS &rhs) const noexcept {
		return Get() <= rhs.Get();
	}
	bool operator>(const SharedHandleNTS &rhs) const noexcept {
		return Get() > rhs.Get();
	}
	bool operator>=(const SharedHandleNTS &rhs) const noexcept {
		return Get() >= rhs.Get();
	}
};

}

#endif
