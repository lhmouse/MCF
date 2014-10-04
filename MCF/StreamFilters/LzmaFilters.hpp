// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_STREAM_FILTERS_LZMA_FILTERS_HPP_
#define MCF_STREAM_FILTERS_LZMA_FILTERS_HPP_

#include "StreamFilterBase.hpp"
#include <memory>

namespace MCF {

class LzmaEncoder : CONCRETE(StreamFilterBase) {
private:
	class xDelegate;

private:
	const std::unique_ptr<xDelegate> xm_pDelegate;

public:
	explicit LzmaEncoder(unsigned uLevel = 6, unsigned long ulDictSize = 1ul << 23);
	~LzmaEncoder() noexcept;

public:
	void Abort() noexcept override;
	void Update(const void *pData, std::size_t uSize) override;
	void Finalize() override;
};

class LzmaDecoder : CONCRETE(StreamFilterBase) {
private:
	class xDelegate;

private:
	const std::unique_ptr<xDelegate> xm_pDelegate;

public:
	LzmaDecoder();
	~LzmaDecoder() noexcept;

public:
	void Abort() noexcept override;
	void Update(const void *pData, std::size_t uSize) override;
	void Finalize() override;
};

}

#endif
