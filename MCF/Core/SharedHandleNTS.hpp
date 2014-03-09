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
	template<typename HANDLE_T, class CLOSER_T>
	class SharedNodeNTS {
	public:
		static SharedNodeNTS *Create(HANDLE_T hObj){
			return Recreate(nullptr, hObj);
		}
		static SharedNodeNTS *Recreate(SharedNodeNTS *pNode, HANDLE_T hObj){
			if(hObj == CLOSER_T()()){
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
				CLOSER_T()(hObj);
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

		static const HANDLE_T *ToPHandle(SharedNodeNTS *pNode){
			if(pNode){
				return (const HANDLE_T *)((std::intptr_t)pNode + OFFSET_OF(SharedNodeNTS, xm_hObj));
			}
			return nullptr;
		}
		static SharedNodeNTS *FromPHandle(const HANDLE_T *pHandle){
			if(pHandle){
				return (SharedNodeNTS *)((std::intptr_t)pHandle - OFFSET_OF(SharedNodeNTS, xm_hObj));
			}
			return nullptr;
		}

	private:
		HANDLE_T xm_hObj;
		std::size_t xm_uWeakCount;
		std::size_t xm_uCount;
#ifndef NDEBUG
		// 用于在手动调用 AddRef() 和 DropRef() 时检测多次释放。
		SharedNodeNTS *xm_pDebugInfo;
#endif

	private:
		explicit constexpr SharedNodeNTS(HANDLE_T hObj) noexcept
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
				CLOSER_T()(xm_hObj);
				xm_hObj = CLOSER_T()();
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
		HANDLE_T Get() const noexcept {
			xValidate();

			return xm_hObj;
		}
		std::size_t GetRefCount() const noexcept {
			return xm_uCount;
		}
	};
}

template<typename HANDLE_T, class CLOSER_T>
class SharedHandleNTS;

template<typename HANDLE_T, class CLOSER_T>
class WeakHandleNTS {
	friend class SharedHandleNTS<HANDLE_T, CLOSER_T>;

private:
	typedef __MCF::SharedNodeNTS<HANDLE_T, CLOSER_T> xSharedNodeNTS;
	typedef SharedHandleNTS<HANDLE_T, CLOSER_T> xStrongHandleNTS;

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

template<typename HANDLE_T, class CLOSER_T>
class SharedHandleNTS {
	friend class WeakHandleNTS<HANDLE_T, CLOSER_T>;

private:
	typedef __MCF::SharedNodeNTS<HANDLE_T, CLOSER_T> xSharedNodeNTS;
	typedef WeakHandleNTS<HANDLE_T, CLOSER_T> xWeakHandleNTS;

public:
	static void AddRef(const HANDLE_T *pHandle) noexcept {
		xSharedNodeNTS::AddRef(xSharedNodeNTS::FromPHandle(pHandle));
	}
	static void DropRef(const HANDLE_T *pHandle) noexcept {
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
	constexpr explicit SharedHandleNTS(HANDLE_T hObj) noexcept : SharedHandleNTS(xSharedNodeNTS::Create(hObj)) {
	}
	explicit SharedHandleNTS(const xWeakHandleNTS &rhs) noexcept : SharedHandleNTS(xSharedNodeNTS::AddRef(rhs.xm_pNode)) {
	}
	SharedHandleNTS(const SharedHandleNTS &rhs) noexcept : SharedHandleNTS(xSharedNodeNTS::AddRef(rhs.xm_pNode)) {
	}
	SharedHandleNTS(SharedHandleNTS &&rhs) noexcept : SharedHandleNTS(rhs.xm_pNode) {
		rhs.xm_pNode = nullptr;
	}
	SharedHandleNTS &operator=(HANDLE_T hObj){
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
		return Get() != CLOSER_T()();
	}
	HANDLE_T Get() const noexcept {
		return xm_pNode ? xm_pNode->Get() : CLOSER_T()();
	}
	std::size_t GetRefCount() const noexcept {
		return xm_pNode ? xm_pNode->GetRefCount() : 0;
	}
	const HANDLE_T *AddRef() const noexcept {
		return xSharedNodeNTS::ToPHandle(xSharedNodeNTS::AddRef(xm_pNode));
	}

	void Reset() noexcept {
		xSharedNodeNTS::DropRef(xm_pNode);
		xm_pNode = nullptr;
	}
	void Reset(HANDLE_T hObj){
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
	explicit operator HANDLE_T() const noexcept {
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
