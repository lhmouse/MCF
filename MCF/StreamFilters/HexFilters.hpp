// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_STREAM_FILTERS_HEX_FILTERS_HPP_
#define MCF_STREAM_FILTERS_HEX_FILTERS_HPP_

#include "StreamFilterBase.hpp"

namespace MCF {

class HexEncoder : public StreamFilterBase {
private:
	const bool xm_bUpperCase;

public:
	explicit HexEncoder(bool bUpperCase = true) noexcept;

public:
	void Update(const void *pData, std::size_t uSize) override;
};

class HexDecoder : public StreamFilterBase {
private:
	int xm_nHigh;

public:
	HexDecoder() noexcept;

public:
	void Abort() noexcept override;
	void Update(const void *pData, std::size_t uSize) override;
	void Finalize() override;
};

}

#endif
