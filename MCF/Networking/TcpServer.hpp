// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef __MCF_TCP_SERVER_HPP__
#define __MCF_TCP_SERVER_HPP__

#include "TcpPeer.hpp"
#include "../Core/Utilities.hpp"
#include <memory>
#include <cstddef>

namespace MCF {

class TcpServer : NO_COPY {
private:
	class xDelegate;

private:
	std::unique_ptr<xDelegate> xm_pDelegate;

public:
	TcpServer();
	TcpServer(TcpServer &&rhs) noexcept;
	TcpServer &operator=(TcpServer &&rhs) noexcept;
	~TcpServer();

public:
	bool IsRunning() const noexcept;
	void Start(const PeerInfo &vBoundOnto);
	void Stop() noexcept;

	TcpPeer GetPeerTimeout(unsigned long ulMilliSeconds) noexcept;
	TcpPeer GetPeer() noexcept;
};

}

#endif
