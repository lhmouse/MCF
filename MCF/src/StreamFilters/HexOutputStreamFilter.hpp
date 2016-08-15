// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_STREAM_FILTERS_HEX_OUTPUT_STREAM_FILTER_HPP_
#define MCF_STREAM_FILTERS_HEX_OUTPUT_STREAM_FILTER_HPP_

#include "AbstractOutputStreamFilter.hpp"

namespace MCF {

class HexOutputStreamFilter : public AbstractOutputStreamFilter {
private:
	StreamBuffer x_sbufPlain;

public:
	explicit HexOutputStreamFilter(PolyIntrusivePtr<AbstractOutputStream> pUnderlyingStream) noexcept
		: AbstractOutputStreamFilter(std::move(pUnderlyingStream))
	{
	}
	~HexOutputStreamFilter() override;

	HexOutputStreamFilter(HexOutputStreamFilter &&) noexcept = default;
	HexOutputStreamFilter &operator=(HexOutputStreamFilter &&) noexcept = default;

private:
	void X_FlushPlainBuffer(bool bForceFlushAll);

public:
	void Put(unsigned char byData) override;
	void Put(const void *pData, std::size_t uSize) override;
	void Flush(bool bHard) override;

	void Swap(HexOutputStreamFilter &rhs) noexcept {
		using std::swap;
		swap(y_vStream,   rhs.y_vStream);
		swap(x_sbufPlain, rhs.x_sbufPlain);
	}

public:
	friend void swap(HexOutputStreamFilter &lhs, HexOutputStreamFilter &rhs) noexcept {
		lhs.Swap(rhs);
	}
};

}

#endif
