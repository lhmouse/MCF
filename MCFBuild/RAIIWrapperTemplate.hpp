// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2012 - 2013. LH_Mouse. All wrongs reserved.

#ifndef __MCF_RAII_WRAPPER_TEMPLATE_HPP__
#define __MCF_RAII_WRAPPER_TEMPLATE_HPP__

#include <utility>
#include <memory>

namespace MCF {
	template<typename HANDLE_T, class CLOSER_T>
	class RAIIWrapper {
	private:
		const CLOSER_T xm_Closer;
		HANDLE_T xm_hObject;
	public:
		explicit RAIIWrapper(HANDLE_T hObject = CLOSER_T::Null()) : xm_Closer(), xm_hObject(hObject) {
		}
		RAIIWrapper(RAIIWrapper &&rhs) : xm_Closer(), xm_hObject(rhs.Release()) {
		}
		RAIIWrapper &operator=(RAIIWrapper &&rhs){
			Reset(rhs.Release());
			return *this;
		}
		~RAIIWrapper(){
			Reset(CLOSER_T::Null());
		}

		RAIIWrapper(const RAIIWrapper &rhs) = delete;
		void operator=(const RAIIWrapper &rhs) = delete;
	public:
		bool IsGood() const {
			return Get() != CLOSER_T::Null();
		}
		HANDLE_T Get() const {
			return xm_hObject;
		}
		void Reset(HANDLE_T hObj = CLOSER_T::Null()){
			const HANDLE_T hOld = xm_hObject;
			xm_hObject = hObj;
			if(hOld != CLOSER_T::Null()){
				xm_Closer(hOld);
			}
		}

		HANDLE_T Release(){
			const HANDLE_T hOld = xm_hObject;
			xm_hObject = CLOSER_T::Null();
			return hOld;
		}
	public:
		operator bool() const {
			return IsGood();
		}
		operator HANDLE_T() const {
			return Get();
		}
		RAIIWrapper &operator=(HANDLE_T hObj){
			Reset(hObj);
			return *this;
		}
	};
}

#endif
