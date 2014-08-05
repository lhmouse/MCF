// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_UDP_SERVER_HPP_
#define MCF_UDP_SERVER_HPP_

#include "../Core/Utilities.hpp"
#include "../Containers/VVector.hpp"
#include <memory>

namespace MCF {

class PeerInfo;
class UdpSender;

struct UdpPacket {
	std::unique_ptr<UdpSender> pSender;
	Vector<unsigned char> vecPayload;
};

class UdpServer : NO_COPY, ABSTRACT {
public:
	static std::unique_ptr<UdpServer> Create(
		const PeerInfo &piBoundOnto,
		bool bExclusive = true,
		bool bReuseAddr = false
	);

public:
	// 这两个函数是线程安全的。
	// 判断 pSender 是否为空来确定是否是有效的数据报。注意 UDP 报文可能长度为零。
	UdpPacket GetPacketTimeout(unsigned long long ullMilliSeconds);
	// 调用 Shutdown() 导致所有正在进行的和以后将会进行的 GetPeer() 都立即返回一个空指针。
	void Shutdown();

	// 用于多播。
	void JoinGroup(const PeerInfo &piGroup);
	void LeaveGroup(const PeerInfo &piGroup) noexcept;
};

}

#endif
