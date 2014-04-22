// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef MCF_TCP_SERVER_HPP_
#define MCF_TCP_SERVER_HPP_

#include "TcpPeer.hpp"
#include "../Core/Utilities.hpp"
#include <memory>

namespace MCF {

class TcpServer : NO_COPY, ABSTRACT {
public:
	static std::unique_ptr<TcpServer> Create(const PeerInfo &vBoundOnto);

public:
	std::unique_ptr<TcpPeer> GetPeerTimeout(unsigned long ulMilliSeconds) noexcept;
	std::unique_ptr<TcpPeer> GetPeer() noexcept;
};

}

#endif
