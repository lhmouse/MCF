// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_STREAM_FILTERS_HEX_HPP_
#define MCF_STREAM_FILTERS_HEX_HPP_

#include "StreamFilterBase.hpp"

namespace MCF {

class HexEncoder : public StreamFilterBase {
private:
	const bool xm_bUpperCase;

public:
	explicit HexEncoder(bool bUpperCase = true) noexcept
		: xm_bUpperCase(bUpperCase)
	{
	}

protected:
	void xDoInit() override;
	void xDoUpdate(const void *pData, std::size_t uSize) override;
	void xDoFinalize() override;
};

class HexDecoder : public StreamFilterBase {
private:
	int xm_nHigh;

protected:
	void xDoInit() override;
	void xDoUpdate(const void *pData, std::size_t uSize) override;
	void xDoFinalize() override;
};

}

#endif
