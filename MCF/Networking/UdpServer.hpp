// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef __MCF_UDP_SERVER_HPP__
#define __MCF_UDP_SERVER_HPP__

#include "UdpPacket.hpp"
#include "../Core/NoCopy.hpp"
#include <functional>
#include <memory>
#include <cstddef>

namespace MCF {

class UdpServer : NO_COPY {
private:
	class xDelegate;

private:
	std::unique_ptr<xDelegate> xm_pDelegate;

public:
	UdpServer();
	UdpServer(UdpServer &&rhs) noexcept;
	UdpServer &operator=(UdpServer &&rhs) noexcept;
	~UdpServer();

public:
	bool IsRunning() const noexcept;
	void Start(const PeerInfo &vBoundOnto);
	void Stop() noexcept;

	UdpPacket GetPacketTimeout(unsigned long ulMilliSeconds) noexcept;
	UdpPacket GetPacket() noexcept;
};

}

#endif
