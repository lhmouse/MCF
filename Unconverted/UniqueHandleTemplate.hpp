// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2012 - 2013. LH_Mouse. All wrongs reserved.

#ifndef __MCF_UNIQUE_HANDLE_TEMPLATE_HPP__
#define __MCF_UNIQUE_HANDLE_TEMPLATE_HPP__

#include "StdMCF.hpp"

namespace MCF {
	template<typename HANDLE_TYPE, class HANDLE_CLOSER>
	class UniqueHandleTemplate {
	private:
		constexpr static HANDLE_TYPE xGetNullHandle();
	private:
		HANDLE_CLOSER xm_HandleCloser;
		HANDLE_TYPE xm_hObject;
	public:
		UniqueHandleTemplate() : xm_HandleCloser(), xm_hObject(xGetNullHandle()) {
		}
		UniqueHandleTemplate(HANDLE_TYPE hObject, HANDLE_CLOSER &&HandleCloser) : xm_HandleCloser(), xm_hObject(xGetNullHandle()) {
			xContract(hObject, std::move(HandleCloser));
		}
		UniqueHandleTemplate(UniqueHandleTemplate &&src) : xm_HandleCloser(), xm_hObject(xGetNullHandle()) {
			Swap(std::move(src));
		}
		UniqueHandleTemplate &operator=(std::pair<HANDLE_TYPE, HANDLE_CLOSER> &&Contents){
			Assign(Contents.first, std::move(Contents.second));
			return *this;
		}
		UniqueHandleTemplate &operator=(UniqueHandleTemplate &&src){
			Assign(std::move(src));
			return *this;
		}
		~UniqueHandleTemplate(){
			xTidy();
		}

		UniqueHandleTemplate(const UniqueHandleTemplate &src) = delete;
		void operator=(const UniqueHandleTemplate &src) = delete;
	private:
		inline __attribute__((always_inline)) void xContract(HANDLE hObject, HANDLE_CLOSER &&HandleCloser){
			xm_hObject		= hObject;
			xm_HandleCloser	= std::move(HandleCloser);
		}
		inline __attribute__((always_inline)) HANDLE_TYPE xExtract(){
			const HANDLE_TYPE hRet = xm_hObject;
			xm_hObject = xGetNullHandle();
			return hRet;
		}
		inline __attribute__((always_inline)) void xTidy(){
			if(xm_hObject != xGetNullHandle()){
				xm_HandleCloser(xm_hObject);
				xm_hObject = xGetNullHandle();
			}
		}
	public:
		HANDLE_TYPE Get() const {
			return xm_hObject;
		}
		bool IsNull() const {
			return Get() != xGetNullHandle();
		}
		void Assign(HANDLE_TYPE hNewObject, HANDLE_CLOSER &&HandleCloser){
			xTidy();
			xContract(hNewObject, std::move(HandleCloser));
		}
		void Assign(UniqueHandleTemplate &&src){
			Swap(std::move(src));
		}
		HANDLE_TYPE Release(){
			return xExtract();
		}
		void Close(){
			xTidy();
		}
		void Swap(UniqueHandleTemplate &src){
			Swap(std::move(src));
		}
		void Swap(UniqueHandleTemplate &&src){
			if(this != &src){
				std::swap(xm_hObject, src.xm_hObject);
				std::swap(xm_HandleCloser, src.xm_HandleCloser);
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
	inline UniqueHandleTemplate<HANDLE_TYPE, HANDLE_CLOSER> MakeUniqueHandle(HANDLE_TYPE hObject, HANDLE_CLOSER &&HandleCloser){
		return UniqueHandleTemplate<HANDLE_TYPE, HANDLE_CLOSER>(hObject, std::forward<HANDLE_CLOSER>(HandleCloser));
	}
}

#endif
