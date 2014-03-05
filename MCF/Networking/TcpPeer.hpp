// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef __MCF_TCP_PEER_HPP__
#define __MCF_TCP_PEER_HPP__

#include "../Core/NoCopy.hpp"
#include <memory>
#include <cstddef>
#include <cstdint>

namespace MCF {

struct PeerInfoIPv4 {
	std::uint8_t au8IP[4];
	std::uint16_t u16Port;
};

class TcpServer;
class TcpClient;

class TcpPeer : NO_COPY {
	friend class TcpServer;
	friend class TcpClient;
private:
	class xDelegate;
private:
	const std::unique_ptr<xDelegate> xm_pDelegate;
private:
	explicit TcpPeer(const void *pImpl);
public:
	~TcpPeer();
public:
	PeerInfoIPv4 GetPeerInfo() const noexcept;

	std::size_t Read(void *pData, std::size_t uSize);
	void ShutdownRead() noexcept;

	void Write(const void *pData, std::size_t uSize);
	void ShutdownWrite() noexcept;
};

}

#endif
