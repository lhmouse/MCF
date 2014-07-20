// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014 LH_Mouse. All wrongs reserved.

#ifndef MCF_TCP_SERVER_HPP_
#define MCF_TCP_SERVER_HPP_

#include "TcpPeer.hpp"
#include "../Core/Utilities.hpp"
#include <memory>

namespace MCF {

class TcpServer : NO_COPY, ABSTRACT {
public:
	static std::unique_ptr<TcpServer> Create(
		const PeerInfo &piBoundOnto,
		bool bExclusive = true,
		bool bReuseAddr = false
	);

public:
	// 这些函数都是线程安全的。
	std::unique_ptr<TcpPeer> GetPeerTimeout(unsigned long long ullMilliSeconds);
	std::unique_ptr<TcpPeer> GetPeer();
	// 调用 Shutdown() 导致所有正在进行的和以后将会进行的 GetPeer() 都立即返回一个空指针。
	void Shutdown();
};

}

#endif
