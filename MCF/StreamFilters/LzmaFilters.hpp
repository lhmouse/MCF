// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_LZMA_FILTERS_HPP_
#define MCF_LZMA_FILTERS_HPP_

#include "StreamFilterBase.hpp"
#include <memory>

namespace MCF {

class LzmaEncoder : public StreamFilterBase {
public:
	static std::unique_ptr<LzmaEncoder> Create(
		unsigned int uLevel = 6, unsigned long ulDictSize = (1ul << 23)
	);
};

class LzmaDecoder : public StreamFilterBase {
public:
	static std::unique_ptr<LzmaDecoder> Create();
};

}

#endif
