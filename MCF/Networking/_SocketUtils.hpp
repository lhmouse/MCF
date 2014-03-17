 // 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef __MCF_SOCKET_UTILS_HPP__
#define __MCF_SOCKET_UTILS_HPP__

#include "../Core/UniqueHandle.hpp"
#include "../Core/SharedHandle.hpp"
#include "../Core/Exception.hpp"
#include "../Core/NoCopy.hpp"
#include <winsock2.h>
#include <ws2tcpip.h>

namespace MCF {

namespace __MCF {
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
				MCF_THROW(::WSAGetLastError(), L"::WSAStartup() 失败。");
			}
		}
		~WSAInitializer(){
			::WSACleanup();
		}
	};
}

}

#endif
