// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014 LH_Mouse. All wrongs reserved.

#ifndef MCF_UDP_SENDER_HPP_
#define MCF_UDP_SENDER_HPP_

#include "../Core/Utilities.hpp"
#include "PeerInfo.hpp"
#include <memory>
#include <cstddef>

namespace MCF {

class UdpSender : NO_COPY, ABSTRACT {
public:
	static std::unique_ptr<UdpSender> Create(const PeerInfo &piServerInfo);
	static std::unique_ptr<UdpSender> Create(const PeerInfo &piServerInfo, const PeerInfo &piLocalInfo);

	static std::unique_ptr<UdpSender> CreateNoThrow(const PeerInfo &piServerInfo);
	static std::unique_ptr<UdpSender> CreateNoThrow(const PeerInfo &piServerInfo, const PeerInfo &piLocalInfo);

public:
	const PeerInfo &GetPeerInfo() const noexcept;
	const PeerInfo &GetLocalInfo() const noexcept;

	void Send(const void *pData, std::size_t uSize);
};

}

#endif
