// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_STREAM_FILTERS_HEX_HPP_
#define MCF_STREAM_FILTERS_HEX_HPP_

#include "StreamFilterBase.hpp"

namespace MCF {

class HexEncoder : public StreamFilterBase {
private:
	const bool x_bUpperCase;

public:
	explicit HexEncoder(bool bUpperCase = true) noexcept
		: x_bUpperCase(bUpperCase)
	{
	}

protected:
	void XDoInit() override;
	void XDoUpdate(const void *pData, std::size_t uSize) override;
	void XDoFinalize() override;
};

class HexDecoder : public StreamFilterBase {
private:
	int x_nHigh;

protected:
	void XDoInit() override;
	void XDoUpdate(const void *pData, std::size_t uSize) override;
	void XDoFinalize() override;
};

}

#endif
