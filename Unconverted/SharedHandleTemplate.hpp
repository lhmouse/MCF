// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2012 - 2013. LH_Mouse. All wrongs reserved.

#ifndef __MCF_SHARED_HANDLE_TEMPLATE_HPP__
#define __MCF_SHARED_HANDLE_TEMPLATE_HPP__

#include "StdMCF.hpp"

namespace MCF {
	template<typename HANDLE_TYPE, class HANDLE_CLOSER>
	class SharedHandleTemplate {
	private:
		typedef struct xtagContents {
			unsigned long ulRefCount;
			HANDLE hObject;
			HANDLE_CLOSER HandleCloser;
		} xCONTENTS;
	private:
		constexpr static HANDLE_TYPE xGetNullHandle();
	private:
		mutable long xm_lToken;
		xCONTENTS *xm_pContents;
	public:
		SharedHandleTemplate() : xm_lToken(1), xm_pContents(nullptr) {
		}
		SharedHandleTemplate(HANDLE_TYPE hObject, HANDLE_CLOSER &&HandleCloser) : xm_lToken(1), xm_pContents(nullptr) {
			xContract(hObject, std::move(HandleCloser));
		}
		SharedHandleTemplate(const SharedHandleTemplate &src) : xm_lToken(1), xm_pContents(nullptr) {
			src.xLock();
				xJoin(src);
			src.xUnlock();
		}
		SharedHandleTemplate(SharedHandleTemplate &&src) : xm_lToken(1), xm_pContents(nullptr) {
			Swap(std::move(src));
		}
		SharedHandleTemplate &operator=(std::pair<HANDLE_TYPE, HANDLE_CLOSER> &&Contents){
			Assign(Contents.first, std::move(Contents.second));
			return *this;
		}
		SharedHandleTemplate &operator=(const SharedHandleTemplate &src){
			Assign(src);
			return *this;
		}
		SharedHandleTemplate &operator=(SharedHandleTemplate &&src){
			Assign(std::move(src));
			return *this;
		}
		~SharedHandleTemplate(){
			xTidy();
		}
	private:
		inline __attribute__((always_inline)) void xLock() const {
			while(::InterlockedExchange(&xm_lToken, 0) == 0){
				// 忙等。
			}
		}
		inline __attribute__((always_inline)) void xUnlock() const {
			xm_lToken = 1;
		}

		inline __attribute__((always_inline)) void xContract(HANDLE hObject, HANDLE_CLOSER &&HandleCloser){
			if(hObject != xGetNullHandle()){
				xm_pContents = new xCONTENTS;
				xm_pContents->ulRefCount	= 1;
				xm_pContents->hObject		= hObject;
				xm_pContents->HandleCloser	= std::move(HandleCloser);
			}
		}
		inline __attribute__((always_inline)) void xJoin(const SharedHandleTemplate &src){
			if(src.xm_pContents != nullptr){
				xm_pContents = src.xm_pContents;
				::InterlockedIncrement(&xm_pContents->ulRefCount);
			}
		}
		inline __attribute__((always_inline)) void xTidy(){
			if(xm_pContents != nullptr){
				if(::InterlockedDecrement(&xm_pContents->ulRefCount) == 0){
					xm_pContents->HandleCloser(xm_pContents->hObject);
					delete xm_pContents;
				}
				xm_pContents = nullptr;
			}
		}
	public:
		HANDLE_TYPE Get() const {
			if(xm_pContents == nullptr){
				return xGetNullHandle();
			}
			return xm_pContents->hObject;
		}
		bool IsNull() const {
			return Get() != xGetNullHandle();
		}
		void Assign(HANDLE_TYPE hNewObject, HANDLE_CLOSER &&HandleCloser){
			xLock();
				xTidy();
				xContract(hNewObject, std::move(HandleCloser));
			xUnlock();
		}
		void Assign(const SharedHandleTemplate &src){
			if(this != &src){
				xLock();
				src.xLock();
					xTidy();
					xJoin(src);
				src.xUnlock();
				xUnlock();
			}
		}
		void Assign(SharedHandleTemplate &&src){
			Swap(std::move(src));
		}
		void Close(){
			xLock();
				xTidy();
			xUnlock();
		}
		void Swap(SharedHandleTemplate &src){
			Swap(std::move(src));
		}
		void Swap(SharedHandleTemplate &&src){
			if(this != &src){
				xLock();
					src.xLock();
					std::swap(xm_pContents, src.xm_pContents);
				xUnlock();
			}
		}
	public:
		operator HANDLE_TYPE() const {
			return Get();
		}
		operator bool() const {
			return IsNull();
		}
	};

	template<typename HANDLE_TYPE, class HANDLE_CLOSER>
	inline SharedHandleTemplate<HANDLE_TYPE, HANDLE_CLOSER> MakeSharedHandle(HANDLE_TYPE hObject, HANDLE_CLOSER &&HandleCloser){
		return SharedHandleTemplate<HANDLE_TYPE, HANDLE_CLOSER>(hObject, std::forward<HANDLE_CLOSER>(HandleCloser));
	}
}

#endif
