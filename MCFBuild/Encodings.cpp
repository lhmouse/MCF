// Copyleft 2013, LH_Mouse. All wrongs reserved.

#include "PreCompiled.hpp"
#include <string>
using namespace MCFBuild;

namespace MCFBuild {
	std::string WcsToU8s(const std::wstring &src){
		std::string ret;
		const int nConvLen = ::WideCharToMultiByte(CP_UTF8, 0, src.c_str(), src.size(), nullptr, 0, nullptr, nullptr);
		if(nConvLen > 0){
			ret.resize(nConvLen);
			ret.resize(::WideCharToMultiByte(CP_UTF8, 0, src.c_str(), src.size(), &ret[0], nConvLen, nullptr, nullptr));
		}
		return std::move(ret);
	}
	std::wstring U8sToWcs(const std::string &src){
		std::wstring ret;
		const int nConvLen = ::MultiByteToWideChar(CP_UTF8, 0, src.c_str(), src.size(), nullptr, 0);
		if(nConvLen > 0){
			ret.resize(nConvLen);
			ret.resize(::MultiByteToWideChar(CP_UTF8, 0, src.c_str(), src.size(), &ret[0], nConvLen));
		}
		return std::move(ret);
	}
}
