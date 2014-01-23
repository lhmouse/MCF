// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013. LH_Mouse. All wrongs reserved.

#ifndef __MCF_SHARED_HANDLE_HPP__
#define __MCF_SHARED_HANDLE_HPP__

#include <new>
#include <cstddef>

namespace MCF {

namespace __MCF {
	template<typename HANDLE_T, class CLOSER_T>
	class SharedNode {
	public:
		static SharedNode *Create(HANDLE_T hObj){
			return Recreate(nullptr, hObj);
		}
		static SharedNode *Recreate(SharedNode *pNode, HANDLE_T hObj){
			if(hObj == CLOSER_T()()){
				if(pNode && (pNode->xDropRef())){
					delete pNode;
				}
				return nullptr;
			}
			if(pNode && (pNode->xDropRef())){
				pNode->~SharedNode();
				new(pNode) SharedNode(hObj);
				return pNode;
			}
			return new SharedNode(hObj);
		}
		static SharedNode *AddWeakRef(SharedNode *pNode) noexcept {
			if(pNode && pNode->xAddWeakRef()){
				return pNode;
			}
			return nullptr;
		}
		static SharedNode *AddRef(SharedNode *pNode) noexcept {
			if(pNode && pNode->xAddRef()){
				return pNode;
			}
			return nullptr;
		}
		static void DropRef(SharedNode *pNode) noexcept {
			if(pNode && pNode->xDropRef()){
				delete pNode;
			}
		}
		static void DropWeakRef(SharedNode *pNode) noexcept {
			if(pNode && pNode->xDropWeakRef()){
				delete pNode;
			}
		}

		static const HANDLE_T *ToPHandle(SharedNode *pNode){
			if(pNode){
				return (const HANDLE_T *)((std::intptr_t)pNode + offsetof(SharedNode, xm_hObj));
			}
			return nullptr;
		}
		static SharedNode *FromPHandle(const HANDLE_T *pHandle){
			if(pHandle){
				return (SharedNode *)((std::intptr_t)pHandle - offsetof(SharedNode, xm_hObj));
			}
			return nullptr;
		}
	private:
		HANDLE_T xm_hObj;
		volatile std::size_t xm_uWeakCount;
		volatile std::size_t xm_uCount;
#ifndef NDEBUG
		// 用于在手动调用 AddRef() 和 DropRef() 时检测多次释放。
		SharedNode *xm_pDebugInfo;
#endif
	private:
		explicit constexpr SharedNode(HANDLE_T hObj) noexcept
			: xm_hObj(hObj), xm_uWeakCount(1), xm_uCount(1)
#ifndef NDEBUG
			, xm_pDebugInfo(this)
#endif
		{
		}
#ifndef NDEBUG
		~SharedNode(){
			ASSERT(xm_pDebugInfo == this);

			xm_pDebugInfo = nullptr;
		}
#endif
	private:
		void xValidate() const noexcept {
#ifndef NDEBUG
			if(xm_pDebugInfo != this){
				__MCF_Bail(L"SharedNodeNTS::xValidate() 失败：侦测到堆损坏或二次释放。");
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

			ASSERT(xm_uCount != 0);

			if(__atomic_sub_fetch(&xm_uCount, 1, __ATOMIC_RELAXED) == 0){
				CLOSER_T()(xm_hObj);
				xm_hObj = CLOSER_T()();
			}
			return xDropWeakRef();
		}
		bool xDropWeakRef() noexcept {
			xValidate();

			ASSERT(xm_uWeakCount != 0);

			const bool bRet = (__atomic_sub_fetch(&xm_uWeakCount, 1, __ATOMIC_RELAXED) == 0);

			ASSERT(!bRet || (xm_uCount == 0));

			return bRet;
		}
	public:
		HANDLE_T Get() const noexcept {
			xValidate();

			return xm_hObj;
		}
	};
}

template<typename HANDLE_T, class CLOSER_T>
class SharedHandle;

template<typename HANDLE_T, class CLOSER_T>
class WeakHandle {
	friend class SharedHandle<HANDLE_T, CLOSER_T>;
private:
	typedef __MCF::SharedNode<HANDLE_T, CLOSER_T> xSharedNode;
	typedef SharedHandle<HANDLE_T, CLOSER_T> xStrongHandle;
private:
	xSharedNode *xm_pNode;
private:
	WeakHandle(xSharedNode *pNode) noexcept : xm_pNode(pNode) {
	}
public:
	constexpr WeakHandle() noexcept : xm_pNode() {
	}
	WeakHandle(const xStrongHandle &rhs) noexcept : WeakHandle(xSharedNode::AddWeakRef(rhs.xm_pNode)) {
	}
	WeakHandle(const WeakHandle &rhs) noexcept : WeakHandle(xSharedNode::AddWeakRef(rhs.xm_pNode)) {
	}
	WeakHandle(WeakHandle &&rhs) noexcept : WeakHandle(rhs.xm_pNode) {
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
	~WeakHandle(){
		Reset();
	}
public:
	xStrongHandle Lock() const noexcept {
		return xStrongHandle(xSharedNode::AddRef(xm_pNode));
	}

	void Reset() noexcept {
		xSharedNode::DropWeakRef(xm_pNode);
		xm_pNode = nullptr;
	}
	void Reset(const xStrongHandle &rhs) noexcept {
		xSharedNode::DropWeakRef(xm_pNode);
		xm_pNode = xSharedNode::AddWeakRef(rhs.xm_pNode);
	}
	void Reset(const WeakHandle &rhs) noexcept {
		if(&rhs == this){
			return;
		}
		xSharedNode::DropWeakRef(xm_pNode);
		xm_pNode = xSharedNode::AddWeakRef(rhs.xm_pNode);
	}
	void Reset(WeakHandle &&rhs) noexcept {
		if(&rhs == this){
			return;
		}
		xSharedNode::DropWeakRef(xm_pNode);
		xm_pNode = rhs.xm_pNode;
		rhs.xm_pNode = nullptr;
	}

	void Swap(WeakHandle &rhs) noexcept {
		if(&rhs == this){
			return;
		}
		std::swap(xm_pNode, rhs.xm_pNode);
	}
};

template<typename HANDLE_T, class CLOSER_T>
class SharedHandle {
	friend class WeakHandle<HANDLE_T, CLOSER_T>;
private:
	typedef __MCF::SharedNode<HANDLE_T, CLOSER_T> xSharedNode;
	typedef WeakHandle<HANDLE_T, CLOSER_T> xWeakHandle;
public:
	static void AddRef(const HANDLE_T *pHandle) noexcept {
		xSharedNode::AddRef(xSharedNode::FromPHandle(pHandle));
	}
	static void DropRef(const HANDLE_T *pHandle) noexcept {
		xSharedNode::DropRef(xSharedNode::FromPHandle(pHandle));
	}
private:
	xSharedNode *xm_pNode;
private:
	SharedHandle(xSharedNode *pNode) noexcept : xm_pNode(pNode) {
	}
public:
	constexpr SharedHandle() noexcept : xm_pNode() {
	}
	constexpr explicit SharedHandle(HANDLE_T hObj) noexcept : SharedHandle(xSharedNode::Create(hObj)) {
	}
	SharedHandle(const xWeakHandle &rhs) noexcept : SharedHandle(xSharedNode::AddRef(rhs.xm_pNode)) {
	}
	SharedHandle(const SharedHandle &rhs) noexcept : SharedHandle(xSharedNode::AddRef(rhs.xm_pNode)) {
	}
	SharedHandle(SharedHandle &&rhs) noexcept : SharedHandle(rhs.xm_pNode) {
		rhs.xm_pNode = nullptr;
	}
	SharedHandle &operator=(HANDLE_T hObj){
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
	~SharedHandle(){
		Reset();
	}
public:
	bool IsGood() const noexcept {
		return Get() != CLOSER_T()();
	}
	HANDLE_T Get() const noexcept {
		return xm_pNode ? xm_pNode->Get() : CLOSER_T()();
	}
	const HANDLE_T *AddRef() const noexcept {
		return xSharedNode::ToPHandle(xSharedNode::AddRef(xm_pNode));
	}

	void Reset() noexcept {
		xSharedNode::DropRef(xm_pNode);
		xm_pNode = nullptr;
	}
	void Reset(HANDLE_T hObj){
		xm_pNode = xSharedNode::Recreate(xm_pNode, hObj);
	}
	void Reset(const xWeakHandle &rhs) noexcept {
		xSharedNode::DropRef(xm_pNode);
		xm_pNode = xSharedNode::AddRef(rhs.xm_pNode);
	}
	void Reset(const SharedHandle &rhs) noexcept {
		if(&rhs == this){
			return;
		}
		xSharedNode::DropRef(xm_pNode);
		xm_pNode = xSharedNode::AddRef(rhs.xm_pNode);
	}
	void Reset(SharedHandle &&rhs) noexcept {
		if(&rhs == this){
			return;
		}
		xSharedNode::DropRef(xm_pNode);
		xm_pNode = rhs.xm_pNode;
		rhs.xm_pNode = nullptr;
	}

	void Swap(SharedHandle &rhs) noexcept {
		if(&rhs == this){
			return;
		}
		std::swap(xm_pNode, rhs.xm_pNode);
	}
public:
	explicit operator bool() const noexcept {
		return IsGood();
	}
	operator HANDLE_T() const noexcept {
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
};

}

#endif
