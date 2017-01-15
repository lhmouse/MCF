// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#ifndef MCF_STREAMS_STANDARD_OUTPUT_STREAM_HPP_
#define MCF_STREAMS_STANDARD_OUTPUT_STREAM_HPP_

#include "AbstractOutputStream.hpp"

namespace MCF {

class StandardOutputStream : public AbstractOutputStream {
public:
	StandardOutputStream() noexcept = default;
	~StandardOutputStream() override;

public:
	void Put(unsigned char byData) override;
	void Put(const void *pData, std::size_t uSize) override;
	void Flush(bool bHard) override;

	void PutChar32(char32_t c32Data);
	void PutText(const wchar_t *pwcData, std::size_t uSize, bool bAppendNewLine);
	void PutNewLine(){
		PutText(L"", 0, true);
	}

	bool IsBuffered() const noexcept;
	void SetBuffered(bool bBuffered) noexcept;
};

}

#endif
