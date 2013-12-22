// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013. LH_Mouse. All wrongs reserved.

#ifndef __MCF_SMART_HANDLES_HPP__
#define __MCF_SMART_HANDLES_HPP__

#include <utility>

namespace MCF {
/*
	struct HandleCloser {
		constexpr HANDLE_TYPE Null(){
			return NULL_HANDLE_CONSTANT;
		}
		void Close(HANDLE_TYPE h){
			CLOSE_HANDLE(h);
		}
	};
*/

	template<typename HANDLE_T, class CLOSER_T>
	class UniqueHandle {
	private:
		HANDLE_T xm_hObj;
	public:
		UniqueHandle() : xm_hObj(CLOSER_T().Null()) {
		}
		explicit UniqueHandle(HANDLE_T hObj) : UniqueHandle() {
			Reset(hObj);
		}
		UniqueHandle(UniqueHandle &&rhs) : UniqueHandle() {
			Swap(rhs);
		}
		UniqueHandle &operator=(HANDLE_T hObj){
			Reset(hObj);
			return *this;
		}
		UniqueHandle &operator=(UniqueHandle &&rhs){
			Swap(rhs);
			return *this;
		}
		~UniqueHandle(){
			Reset();
		}

		UniqueHandle(const UniqueHandle &rhs) = delete;
		void operator=(const UniqueHandle &rhs) = delete;
	public:
		bool IsGood() const {
			return Get() != CLOSER_T().Null();
		}
		HANDLE_T Get() const {
			return xm_hObj;
		}

		void Reset(HANDLE_T hObj = CLOSER_T().Null()){
			const HANDLE_T hOld = Release(hObj);
			if(hOld != CLOSER_T().Null()){
				CLOSER_T().Close(hOld);
			}
		}
		void Reset(UniqueHandle &&rhs){
			Swap(rhs);
		}
		HANDLE_T Release(HANDLE_T hObj = CLOSER_T().Null()){
			const HANDLE_T hOld = xm_hObj;
			xm_hObj = hObj;
			return hOld;
		}

		void Swap(UniqueHandle &rhs){
			std::swap(xm_hObj, rhs.xm_hObj);
		}
		void Swap(UniqueHandle &&rhs){
			Swap(rhs);
		}
	public:
		explicit operator bool() const {
			return IsGood();
		}
		operator HANDLE_T() const {
			return Get();
		}
	};

	template<typename HANDLE_T, class CLOSER_T>
	class WeakHandle;

	template<typename HANDLE_T, class CLOSER_T>
	class SharedHandle {
		friend class WeakHandle<HANDLE_T, CLOSER_T>;
	private:
		struct xSharedNode {
			unsigned long m_ulWeakRefCount;
			unsigned long m_ulRefCount;
			UniqueHandle<HANDLE_T, CLOSER_T> m_hObj;
		};
	private:
		xSharedNode *xm_pNode;
	public:
		SharedHandle() : xm_pNode(nullptr) {
		}
		explicit SharedHandle(HANDLE_T hObj) : SharedHandle() {
			Reset(hObj);
		}
		SharedHandle(const SharedHandle &rhs) : SharedHandle() {
			Reset(rhs);
		}
		SharedHandle(SharedHandle &&rhs) : SharedHandle() {
			Swap(rhs);
		}
		SharedHandle &operator=(HANDLE_T hObj){
			Reset(hObj);
			return *this;
		}
		SharedHandle &operator=(const SharedHandle &rhs){
			Reset(rhs);
			return *this;
		}
		SharedHandle &operator=(SharedHandle &&rhs){
			Swap(rhs);
			return *this;
		}
		~SharedHandle(){
			Reset();
		}
	private:
		void xJoin(xSharedNode *pNode){
			Reset();

			if((pNode != nullptr) && pNode->m_hObj.IsGood()){
				++pNode->m_ulWeakRefCount;
				++pNode->m_ulRefCount;
				xm_pNode = pNode;
			}
		}
	public:
		bool IsGood() const {
			return Get() != CLOSER_T().Null();
		}
		HANDLE_T Get() const {
			if(xm_pNode == nullptr){
				return CLOSER_T().Null();
			}
			return xm_pNode->m_hObj.Get();
		}

		void Reset(HANDLE_T hObj = CLOSER_T().Null()){
			const HANDLE_T hOld = Release(hObj);
			if(hOld != CLOSER_T().Null()){
				CLOSER_T().Close(hOld);
			}
		}
		void Reset(const SharedHandle &rhs){
			if((&rhs != this) && (xm_pNode != rhs.xm_pNode)){
				xJoin(rhs.xm_pNode);
			}
		}
		void Reset(SharedHandle &&rhs){
			Swap(rhs);
		}
		HANDLE_T Release(HANDLE_T hObj = CLOSER_T().Null()){
			HANDLE_T hOld = CLOSER_T().Null();
			if(hObj == CLOSER_T().Null()){
				if(xm_pNode != nullptr){
					if(--xm_pNode->m_ulRefCount == 0){
						hOld = xm_pNode->m_hObj.Release();
					}
					if(--xm_pNode->m_ulWeakRefCount == 0){
						delete xm_pNode;
					}
					xm_pNode = nullptr;
				}
			} else {
				if(xm_pNode == nullptr){
					xm_pNode = new xSharedNode;
				} else {
					if(--xm_pNode->m_ulRefCount == 0){
						hOld = xm_pNode->m_hObj.Release();
					}
					if(--xm_pNode->m_ulWeakRefCount != 0){
						xm_pNode = new xSharedNode;
					}
				}

				xm_pNode->m_ulWeakRefCount = 1;
				xm_pNode->m_ulRefCount = 1;
				xm_pNode->m_hObj.Reset(hObj);
			}
			return hOld;
		}

		void Swap(SharedHandle &rhs){
			std::swap(xm_pNode, rhs.xm_pNode);
		}
		void Swap(SharedHandle &&rhs){
			Swap(rhs);
		}
	public:
		explicit operator bool() const {
			return IsGood();
		}
		operator HANDLE_T() const {
			return Get();
		}
	};

	template<typename HANDLE_T, class CLOSER_T>
	class WeakHandle {
	public:
		typedef SharedHandle<HANDLE_T, CLOSER_T> StrongHandle;
	private:
		typename StrongHandle::xSharedNode *xm_pNode;
	public:
		WeakHandle() : xm_pNode(nullptr) {
		}
		WeakHandle(const StrongHandle &rhs) : WeakHandle() {
			Reset(rhs);
		}
		WeakHandle(const WeakHandle &rhs) : WeakHandle() {
			Reset(rhs);
		}
		WeakHandle(WeakHandle &&rhs) : WeakHandle() {
			Swap(rhs);
		}
		WeakHandle &operator=(const StrongHandle &rhs){
			Reset(rhs);
			return *this;
		}
		WeakHandle &operator=(const WeakHandle &rhs){
			Reset(rhs);
			return *this;
		}
		WeakHandle &operator=(WeakHandle &&rhs){
			Swap(rhs);
			return *this;
		}
		~WeakHandle(){
			Reset();
		}
	private:
		void xWeakJoin(typename StrongHandle::xSharedNode *pNode){
			Reset();

			if((pNode != nullptr) && pNode->m_hObj.IsGood()){
				++pNode->m_ulWeakRefCount;
				xm_pNode = pNode;
			}
		}
	public:
		StrongHandle Lock() const {
			StrongHandle hRet;
			hRet.xJoin(xm_pNode);
			return std::move(hRet);
		}

		void Reset(){
			if(xm_pNode != nullptr){
				if(--xm_pNode->m_ulWeakRefCount == 0){
					delete xm_pNode;
				}
				xm_pNode = nullptr;
			}
		}
		void Reset(const StrongHandle &rhs){
			xWeakJoin(rhs.xm_pNode);
		}
		void Reset(const WeakHandle &rhs){
			xWeakJoin(rhs.xm_pNode);
		}
		void Reset(WeakHandle &&rhs){
			Swap(rhs);
		}

		void Swap(WeakHandle &rhs){
			std::swap(xm_pNode, rhs.xm_pNode);
		}
		void Swap(WeakHandle &&rhs){
			Swap(rhs);
		}
	};
}

#endif
