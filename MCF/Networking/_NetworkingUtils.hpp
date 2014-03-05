 // 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef __MCF_NETWORKING_UTILS_HPP__
#define __MCF_NETWORKING_UTILS_HPP__

#include "../Core/UniqueHandle.hpp"
#include "../Core/SharedHandle.hpp"
#include <winsock2.h>

namespace MCF {

struct SocketCloser {
	constexpr SOCKET operator()() const noexcept {
		return INVALID_SOCKET;
	}
	void operator()(SOCKET s) const noexcept {
		::closesocket(s);
	}
};

typedef UniqueHandle<SOCKET, SocketCloser> UniqueSocket;
typedef SharedHandle<SOCKET, SocketCloser> SharedSocket;

}

#endif
