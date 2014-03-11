// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef __MCF_HTTP_CLIENT_HPP__
#define __MCF_HTTP_CLIENT_HPP__

#include "../Core/NoCopy.hpp"
#include "PeerInfo.hpp"
#include <memory>
#include <cstddef>
#include <cstdint>

namespace MCF {

class HttpClient : NO_COPY {
private:
	class xDelegate;

private:
	std::unique_ptr<xDelegate> xm_pDelegate;

public:

};

}

#endif
