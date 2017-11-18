// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#ifndef MCF_STREAMS_STANDARD_INPUT_STREAM_HPP_
#define MCF_STREAMS_STANDARD_INPUT_STREAM_HPP_

#include "AbstractInputStream.hpp"

namespace MCF {

class StandardInputStream : public AbstractInputStream {
public:
	StandardInputStream() noexcept = default;
	~StandardInputStream() override;

public:
	int Peek() override;
	int Get() override;
	bool Discard() override;
	std::size_t Peek(void *pData, std::size_t uSize) override;
	std::size_t Get(void *pData, std::size_t uSize) override;
	std::size_t Discard(std::size_t uSize) override;
	void Invalidate() override;

	long PeekChar32();
	long GetChar32();
	bool DiscardChar32();
	std::size_t PeekText(wchar_t *pwcData, std::size_t uSize, bool bSingleLine);
	std::size_t GetText(wchar_t *pwcData, std::size_t uSize, bool bSingleLine);
	std::size_t DiscardText(std::size_t uSize, bool bSingleLine);

	bool IsEchoing() const noexcept;
	bool SetEchoing(bool bEchoing) noexcept;
};

}

#endif
