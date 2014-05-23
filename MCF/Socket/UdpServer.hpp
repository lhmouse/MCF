// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef MCF_UDP_SERVER_HPP_
#define MCF_UDP_SERVER_HPP_

#include "PeerInfo.hpp"
#include "UdpPacket.hpp"
#include <memory>

namespace MCF {

class UdpServer : NO_COPY, ABSTRACT {
public:
	static std::unique_ptr<UdpServer> Create(
		const PeerInfo &piBoundOnto,
		bool bExclusive = true,
		bool bReuseAddr = false
	);

public:
	// 这两个函数是线程安全的。
	std::unique_ptr<UdpPacket> GetPacketTimeout(unsigned long long ullMilliSeconds);
	// 调用 Shutdown() 导致所有正在进行的和以后将会进行的 GetPeer() 都立即返回一个空指针。
	void Shutdown();

	// 用于多播。
	void JoinGroup(const PeerInfo &piGroup);
	void LeaveGroup(const PeerInfo &piGroup) noexcept;
};

}

#endif
