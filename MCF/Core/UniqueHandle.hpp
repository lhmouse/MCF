// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013. LH_Mouse. All wrongs reserved.

#ifndef __MCF_UNIQUE_HANDLE_HPP__
#define __MCF_UNIQUE_HANDLE_HPP__

#include "Manipulaters.hpp"

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
	class UniqueHandle {
	private:
		typename MANIPULATER_T::template Rebind<HANDLE_T>::Type xm_Obj;
	public:
		constexpr UniqueHandle() noexcept : UniqueHandle(CLOSER_T()()) {
		}
		constexpr explicit UniqueHandle(HANDLE_T hObj) noexcept : xm_Obj(hObj) {
		}
		UniqueHandle(UniqueHandle &&rhs) noexcept : UniqueHandle(rhs.Release()) {
		}
		UniqueHandle &operator=(HANDLE_T hObj) noexcept {
			Reset(hObj);
			return *this;
		}
		UniqueHandle &operator=(UniqueHandle &&rhs) noexcept {
			Reset(std::move(rhs));
			return *this;
		}
		~UniqueHandle(){
			Reset();
		}

		UniqueHandle(const UniqueHandle &) = delete;
		void operator=(const UniqueHandle &) = delete;
	public:
		bool IsGood() const noexcept {
			return Get() != CLOSER_T()();
		}
		HANDLE_T Get() const noexcept {
			return xm_Obj.Get();
		}
		HANDLE_T Release() noexcept {
			return xm_Obj.Exchange(CLOSER_T()());
		}

		void Reset(HANDLE_T hObj = CLOSER_T()()) noexcept {
			const HANDLE_T hOld = xm_Obj.Exchange(hObj);
			if(hOld == hObj){
				return;
			}
			if(hOld != CLOSER_T()()){
				CLOSER_T()(hOld);
			}
		}
		void Reset(UniqueHandle &&rhs) noexcept {
			if(&rhs == this){
				return;
			}
			Reset(rhs.Release());
		}

		// rhs 使用原子置换，但是 *this 首先被置空，然后才获得 rhs 的值。
		// 因此中间会出现“闪断”。
		void Swap(UniqueHandle &rhs) noexcept {
			if(&rhs == this){
				return;
			}
			Reset(rhs.xm_Obj.Exchange(xm_Obj.Exchange(CLOSER_T()())));
		}
		void Swap(UniqueHandle &&rhs) noexcept {
			Swap(rhs);
		}
	public:
		explicit operator bool() const noexcept {
			return IsGood();
		}
		operator HANDLE_T() const noexcept {
			return Get();
		}

		bool operator==(const UniqueHandle &rhs) const noexcept {
			return Get() == rhs.Get();
		}
		bool operator!=(const UniqueHandle &rhs) const noexcept {
			return Get() != rhs.Get();
		}
		bool operator<(const UniqueHandle &rhs) const noexcept {
			return Get() < rhs.Get();
		}
		bool operator<=(const UniqueHandle &rhs) const noexcept {
			return Get() <= rhs.Get();
		}
		bool operator>(const UniqueHandle &rhs) const noexcept {
			return Get() > rhs.Get();
		}
		bool operator>=(const UniqueHandle &rhs) const noexcept {
			return Get() >= rhs.Get();
		}
	};
}

// Non-Thread-Safe
template<typename HANDLE_T, class CLOSER_T>
using UniqueHandleNTS = __MCF::UniqueHandle<HANDLE_T, CLOSER_T, __MCF::NormalManipulater<int>>;

// Thread-Safe
template<typename HANDLE_T, class CLOSER_T>
using UniqueHandleTS = __MCF::UniqueHandle<HANDLE_T, CLOSER_T, __MCF::AtomicManipulater<int>>;

// 出于习惯原因，UniqueHandle 是非线程安全版的。
template<typename HANDLE_T, class CLOSER_T>
using UniqueHandle = UniqueHandleNTS<HANDLE_T, CLOSER_T>;

}

#endif
