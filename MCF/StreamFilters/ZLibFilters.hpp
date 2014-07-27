// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014 LH_Mouse. All wrongs reserved.

#ifndef MCF_ZLIB_FILTERS_HPP_
#define MCF_ZLIB_FILTERS_HPP_

#include "StreamFilterBase.hpp"
#include <memory>

namespace MCF {

class ZLibEncoder : public StreamFilterBase {
public:
	static std::unique_ptr<ZLibEncoder> Create(bool bRaw = false, unsigned int uLevel = 6);
};

class ZLibDecoder : public StreamFilterBase {
public:
	static std::unique_ptr<ZLibDecoder> Create(bool bRaw = false);
};

}

#endif
