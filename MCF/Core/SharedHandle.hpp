// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013. LH_Mouse. All wrongs reserved.

#ifndef __MCF_SHARED_HANDLE_HPP__
#define __MCF_SHARED_HANDLE_HPP__

#include "Manipulaters.hpp"
#include <new>
#include <cassert>
#include <cstddef>

namespace MCF {

namespace __MCF {
/*
	struct HandleCloser {
		constexpr HANDLE_T operator()() const {
			return NULL_HANDLE_VALUE;
		}
		void operator()(HANDLE_T h) const {
			CLOSE_HANDLE(h);
		}
	};
*/
	template<typename HANDLE_T, class CLOSER_T, class MANIPULATER_T>
	class SharedNode {
	public:
		static SharedNode *Create(HANDLE_T hObj){
			return Recreate(nullptr, hObj);
		}
		static SharedNode *Recreate(SharedNode *pNode, HANDLE_T hObj){
			if(hObj == CLOSER_T()()){
				if((pNode != nullptr) && (pNode->xDropRef())){
					delete pNode;
				}
				return nullptr;
			}
			if((pNode != nullptr) && (pNode->xDropRef())){
				pNode->~SharedNode();
				new(pNode) SharedNode(hObj);
				return pNode;
			}
			return new SharedNode(hObj);
		}
		static SharedNode *AddWeakRef(SharedNode *pNode) noexcept {
			if(pNode == nullptr){
				return nullptr;
			}
			return pNode->xAddWeakRef() ? pNode : nullptr;
		}
		static SharedNode *AddRef(SharedNode *pNode) noexcept {
			if(pNode == nullptr){
				return nullptr;
			}
			return pNode->xAddRef() ? pNode : nullptr;
		}
		static void DropRef(SharedNode *pNode) noexcept {
			if(pNode == nullptr){
				return;
			}
			if(pNode->xDropRef()){
				delete pNode;
			}
		}
		static void DropWeakRef(SharedNode *pNode) noexcept {
			if(pNode == nullptr){
				return;
			}
			if(pNode->xDropWeakRef()){
				delete pNode;
			}
		}
	private:
		HANDLE_T xm_hObj;
		typename MANIPULATER_T::template Rebind<std::size_t>::Type xm_WeakCount;
		typename MANIPULATER_T::template Rebind<std::size_t>::Type xm_StrongCount;
	private:
		explicit SharedNode(HANDLE_T hObj) : xm_hObj(hObj), xm_WeakCount(1), xm_StrongCount(1) { }
	private:
		bool xAddWeakRef() noexcept {
			std::size_t uOldCount;
			do {
				uOldCount = xm_WeakCount.Get();
				if(uOldCount == 0){
					return false;
				}
			} while(!xm_WeakCount.CompareExchange(uOldCount, uOldCount + 1));

			assert(xm_WeakCount.Get() > 1);

			return true;
		}
		bool xAddRef() noexcept {
			if(!xAddWeakRef()){
				return false;
			}

			std::size_t uOldCount;
			do {
				uOldCount = xm_StrongCount.Get();
				if(uOldCount == 0){
					xDropWeakRef();
					return false;
				}
			} while(!xm_StrongCount.CompareExchange(uOldCount, uOldCount + 1));

			assert(xm_StrongCount.Get() > 1);

			return true;
		}
		bool xDropRef() noexcept {
			assert(xm_StrongCount.Get() != 0);

			if(--xm_StrongCount == 0){
				CLOSER_T()(xm_hObj);
				xm_hObj = CLOSER_T()();
			}
			return xDropWeakRef();
		}
		bool xDropWeakRef() noexcept {
			assert(xm_WeakCount.Get() != 0);

			const bool bRet = (--xm_WeakCount == 0);

			assert(!(bRet && (xm_StrongCount.Get() != 0)));

			return bRet;
		}
	public:
		HANDLE_T Get() const noexcept {
			return xm_hObj;
		}
	};

	template<typename HANDLE_T, class CLOSER_T, class MANIPULATER_T>
	class SharedHandle;

	template<typename HANDLE_T, class CLOSER_T, class MANIPULATER_T>
	class WeakHandle {
		friend class SharedHandle<HANDLE_T, CLOSER_T, MANIPULATER_T>;
	private:
		typedef SharedNode<HANDLE_T, CLOSER_T, MANIPULATER_T> xSharedNode;
		typedef SharedHandle<HANDLE_T, CLOSER_T, MANIPULATER_T> xStrongHandle;
	private:
		typename MANIPULATER_T::template Rebind<xSharedNode *>::Type xm_pNode;
	private:
		WeakHandle(xSharedNode *pNode) noexcept : xm_pNode(pNode) {
		}
	public:
		constexpr WeakHandle() noexcept : WeakHandle((xSharedNode *)nullptr) {
		}
		WeakHandle(const xStrongHandle &rhs) noexcept : WeakHandle(xSharedNode::AddWeakRef(rhs.xm_pNode.Get())) {
		}
		WeakHandle(const WeakHandle &rhs) noexcept : WeakHandle(xSharedNode::AddWeakRef(rhs.xm_pNode.Get())) {
		}
		WeakHandle(WeakHandle &&rhs) noexcept : WeakHandle(rhs.xReleaseNode()) {
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
			return xStrongHandle(xSharedNode::AddRef(xm_pNode.Get()));
		}

		void Reset() noexcept {
			xSharedNode::DropWeakRef(xm_pNode.Exchange(nullptr));
		}
		void Reset(const xStrongHandle &rhs) noexcept {
			xSharedNode::DropWeakRef(xm_pNode.Exchange(xSharedNode::AddWeakRef(rhs.xm_pNode.Get())));
		}
		void Reset(const WeakHandle &rhs) noexcept {
			if(&rhs == this){
				return;
			}
			xSharedNode::DropWeakRef(xm_pNode.Exchange(xSharedNode::AddWeakRef(rhs.xm_pNode.Get())));
		}
		void Reset(WeakHandle &&rhs) noexcept {
			if(&rhs == this){
				return;
			}
			xSharedNode::DropWeakRef(xm_pNode.Exchange(rhs.xm_pNode.Exchange(nullptr)));
		}

		// rhs 使用原子置换，但是 *this 首先被置空，然后才获得 rhs 的值。
		// 因此中间会出现“闪断”。
		void Swap(WeakHandle &rhs) noexcept {
			if(&rhs == this){
				return;
			}
			xSharedNode::DropWeakRef(xm_pNode.Exchange(rhs.xm_pNode.Exchange(xm_pNode.Exchange(nullptr))));
		}
		void Swap(WeakHandle &&rhs) noexcept {
			Swap(rhs);
		}
	};

	template<typename HANDLE_T, class CLOSER_T, class MANIPULATER_T>
	class SharedHandle {
		friend class WeakHandle<HANDLE_T, CLOSER_T, MANIPULATER_T>;
	private:
		typedef SharedNode<HANDLE_T, CLOSER_T, MANIPULATER_T> xSharedNode;
		typedef WeakHandle<HANDLE_T, CLOSER_T, MANIPULATER_T> xWeakHandle;
	private:
		typename MANIPULATER_T::template Rebind<xSharedNode *>::Type xm_pNode;
	private:
		SharedHandle(xSharedNode *pNode) noexcept : xm_pNode(pNode) {
		}
	public:
		constexpr SharedHandle() noexcept : SharedHandle((xSharedNode *)nullptr) {
		}
		constexpr explicit SharedHandle(HANDLE_T hObj) noexcept : SharedHandle(xSharedNode::Create(hObj)) {
		}
		SharedHandle(const xWeakHandle &rhs) noexcept : SharedHandle(xSharedNode::AddRef(rhs.xm_pNode.Get())) {
		}
		SharedHandle(const SharedHandle &rhs) noexcept : SharedHandle(xSharedNode::AddRef(rhs.xm_pNode.Get())) {
		}
		SharedHandle(SharedHandle &&rhs) noexcept : SharedHandle(rhs.xm_pNode.Exchange(nullptr)) {
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
			const auto pNode = xm_pNode.Get();
			if(pNode == nullptr){
				return CLOSER_T()();
			}
			return pNode->Get();
		}

		void Reset(HANDLE_T hObj = CLOSER_T()()){
			xm_pNode = xSharedNode::Recreate(xm_pNode.Exchange(nullptr), hObj);
		}
		void Reset(const xWeakHandle &rhs) noexcept {
			xSharedNode::DropRef(xm_pNode.Exchange(xSharedNode::AddRef(rhs.xm_pNode.Get())));
		}
		void Reset(const SharedHandle &rhs) noexcept {
			if(&rhs == this){
				return;
			}
			xSharedNode::DropRef(xm_pNode.Exchange(xSharedNode::AddRef(rhs.xm_pNode.Get())));
		}
		void Reset(SharedHandle &&rhs) noexcept {
			if(&rhs == this){
				return;
			}
			xSharedNode::DropRef(xm_pNode.Exchange(rhs.xm_pNode.Exchange(nullptr)));
		}

		// rhs 使用原子置换，但是 *this 首先被置空，然后才获得 rhs 的值。
		// 因此中间会出现“闪断”。
		void Swap(SharedHandle &rhs) noexcept {
			if(&rhs == this){
				return;
			}
			xSharedNode::DropRef(xm_pNode.Exchange(rhs.xm_pNode.Exchange(xm_pNode.Exchange(nullptr))));
		}
		void Swap(SharedHandle &&rhs) noexcept {
			Swap(rhs);
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

// Non-Thread-Safe
template<typename HANDLE_T, class CLOSER_T>
using WeakHandleNTS = __MCF::WeakHandle<HANDLE_T, CLOSER_T, __MCF::NormalManipulater<int>>;

template<typename HANDLE_T, class CLOSER_T>
using SharedHandleNTS = __MCF::SharedHandle<HANDLE_T, CLOSER_T, __MCF::NormalManipulater<int>>;

// Thread-Safe
template<typename HANDLE_T, class CLOSER_T>
using WeakHandleTS = __MCF::WeakHandle<HANDLE_T, CLOSER_T, __MCF::AtomicManipulater<int>>;

template<typename HANDLE_T, class CLOSER_T>
using SharedHandleTS = __MCF::SharedHandle<HANDLE_T, CLOSER_T, __MCF::AtomicManipulater<int>>;

// 出于习惯原因，WeakHandle 和 SharedHandle 是部分线程安全版的。
template<typename HANDLE_T, class CLOSER_T>
using WeakHandle = WeakHandleTS<HANDLE_T, CLOSER_T>;

template<typename HANDLE_T, class CLOSER_T>
using SharedHandle = SharedHandleTS<HANDLE_T, CLOSER_T>;

}

#endif
