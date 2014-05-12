// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef MCF_TCP_PEER_HPP_
#define MCF_TCP_PEER_HPP_

#include "../Core/Utilities.hpp"
#include "PeerInfo.hpp"
#include <memory>
#include <cstddef>

namespace MCF {

class TcpPeer : NO_COPY, ABSTRACT {
public:
	static std::unique_ptr<TcpPeer> Connect(const PeerInfo &vServerInfo);
	static std::unique_ptr<TcpPeer> ConnectNoThrow(const PeerInfo &vServerInfo);

public:
	const PeerInfo &GetPeerInfo() const noexcept;

	std::size_t Read(void *pData, std::size_t uSize);
	void ShutdownRead() noexcept;

	void SetNoDelay(bool bNoDelay);
	void Write(const void *pData, std::size_t uSize);
	void ShutdownWrite() noexcept;
};

}

#endif
