// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_SOCKET_UTILS_HPP_
#define MCF_SOCKET_UTILS_HPP_

#include "../Core/UniqueHandle.hpp"
#include "../Core/SharedHandle.hpp"
#include "../Core/Exception.hpp"
#include "../Core/Utilities.hpp"
#include <winsock2.h>
#include <ws2tcpip.h>

namespace MCF {

namespace Impl {
	struct SocketCloser {
		constexpr SOCKET operator()() const noexcept {
			return INVALID_SOCKET;
		}
		void operator()(SOCKET s) const noexcept {
			::closesocket(s);
		}
	};

	typedef UniqueHandle<SocketCloser> UniqueSocket;
	typedef SharedHandle<SocketCloser> SharedSocket;

	struct WSAInitializer : NO_COPY {
		WSAInitializer(){
			WSADATA vWsaData;
			if(::WSAStartup(MAKEWORD(2, 2), &vWsaData)){
				MCF_THROW(::GetLastError(), L"::WSAStartup() 失败。"_wso);
			}
		}
		~WSAInitializer() noexcept {
			::WSACleanup();
		}
	};
}

}

#endif
