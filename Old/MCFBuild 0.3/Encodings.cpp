// Copyleft 2013, LH_Mouse. All wrongs reserved.

#include "General.hpp"
using namespace MCFBuild;

namespace MCFBuild {
	vstring WcsToU8s(const wvstring &src){
		vstring ret;
		const int nConvLen = ::WideCharToMultiByte(CP_UTF8, 0, src.c_str(), src.size(), nullptr, 0, nullptr, nullptr);
		if(nConvLen > 0){
			ret.resize(nConvLen);
			ret.resize(::WideCharToMultiByte(CP_UTF8, 0, src.c_str(), src.size(), &ret[0], nConvLen, nullptr, nullptr));
		}
		return std::move(ret);
	}
	wvstring U8sToWcs(const vstring &src){
		wvstring ret;
		const int nConvLen = ::MultiByteToWideChar(CP_UTF8, 0, src.c_str(), src.size(), nullptr, 0);
		if(nConvLen > 0){
			ret.resize(nConvLen);
			ret.resize(::MultiByteToWideChar(CP_UTF8, 0, src.c_str(), src.size(), &ret[0], nConvLen));
		}
		return std::move(ret);
	}
	wvstring U8sToWcs(const std::string &src){
		wvstring ret;
		const int nConvLen = ::MultiByteToWideChar(CP_UTF8, 0, src.c_str(), src.size(), nullptr, 0);
		if(nConvLen > 0){
			ret.resize(nConvLen);
			ret.resize(::MultiByteToWideChar(CP_UTF8, 0, src.c_str(), src.size(), &ret[0], nConvLen));
		}
		return std::move(ret);
	}
}
