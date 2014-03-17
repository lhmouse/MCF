// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef __MCF_TCP_PEER_HPP__
#define __MCF_TCP_PEER_HPP__

#include "../Core/NoCopy.hpp"
#include "PeerInfo.hpp"
#include <memory>
#include <cstddef>

namespace MCF {

class TcpServer;

class TcpPeer : NO_COPY {
	friend class TcpServer;

private:
	class xDelegate;

private:
	std::unique_ptr<xDelegate> xm_pDelegate;

private:
	TcpPeer(void *ppSocket, const void *pSockAddr, std::size_t uSockAddrLen);

public:
	TcpPeer() noexcept;
	explicit TcpPeer(const PeerInfo &vServerInfo);
	TcpPeer(TcpPeer &&rhs) noexcept;
	TcpPeer &operator=(TcpPeer &&rhs) noexcept;
	~TcpPeer();

public:
	bool IsConnected() const noexcept;
	const PeerInfo &GetPeerInfo() const;
	unsigned long ConnectNoThrow(const PeerInfo &vServerInfo);
	void Connect(const PeerInfo &vServerInfo);
	void Disconnect() noexcept;

	std::size_t Read(void *pData, std::size_t uSize);
	void ShutdownRead() noexcept;

	void Write(const void *pData, std::size_t uSize, bool bNoDelay = false);
	void ShutdownWrite() noexcept;

public:
	explicit operator bool() const noexcept {
		return IsConnected();
	}
};

}

#endif
