// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_STREAM_FILTERS_LZMA_HPP_
#define MCF_STREAM_FILTERS_LZMA_HPP_

#include "StreamFilterBase.hpp"
#include "../Core/Exception.hpp"
#include "../SmartPointers/UniquePtr.hpp"

namespace MCF {

class LzmaEncoder : public StreamFilterBase {
private:
	class xDelegate;

private:
	const unsigned xm_uLevel;
	const unsigned long xm_ulDictSize;

	UniquePtr<xDelegate> xm_pDelegate;

public:
	explicit LzmaEncoder(unsigned uLevel = 6, unsigned long ulDictSize = 1ul << 23) noexcept;
	~LzmaEncoder();

protected:
	void xDoInit() override;
	void xDoUpdate(const void *pData, std::size_t uSize) override;
	void xDoFinalize() override;
};

class LzmaDecoder : public StreamFilterBase {
private:
	class xDelegate;

private:
	UniquePtr<xDelegate> xm_pDelegate;

public:
	LzmaDecoder() noexcept;
	~LzmaDecoder();

protected:
	void xDoInit() override;
	void xDoUpdate(const void *pData, std::size_t uSize) override;
	void xDoFinalize() override;
};

class LzmaError : public Exception {
private:
	long xm_lLzmaError;

public:
	LzmaError(const char *pszFile, unsigned long ulLine, const char *pszMessage, long lLzmaError) noexcept;
	~LzmaError() override;

public:
	long GetLzmaError() const noexcept {
		return xm_lLzmaError;
	}
};

}

#endif
