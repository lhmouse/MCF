// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_STREAM_FILTERS_ZLIB_FILTERS_HPP_
#define MCF_STREAM_FILTERS_ZLIB_FILTERS_HPP_

#include "StreamFilterBase.hpp"
#include "../Core/Exception.hpp"
#include <memory>

namespace MCF {

class ZLibEncoder : public StreamFilterBase {
private:
	class xDelegate;

private:
	const std::unique_ptr<xDelegate> xm_pDelegate;

public:
	explicit ZLibEncoder(bool bRaw = false, unsigned uLevel = 6);
	~ZLibEncoder();

public:
	void Abort() noexcept override;
	void Update(const void *pData, std::size_t uSize) override;
	void Finalize() override;
};

class ZLibDecoder : public StreamFilterBase {
private:
	class xDelegate;

private:
	const std::unique_ptr<xDelegate> xm_pDelegate;

public:
	explicit ZLibDecoder(bool bRaw = false);
	~ZLibDecoder();

public:
	void Abort() noexcept override;
	void Update(const void *pData, std::size_t uSize) override;
	void Finalize() override;
};

class ZLibError : public Exception {
private:
	const long xm_lZLibError;

public:
	ZLibError(const char *pszFile, unsigned long ulLine,
		const char *pszMessage, long lZLibError) noexcept;
	~ZLibError() override;

public:
	long GetZLibError() const noexcept {
		return xm_lZLibError;
	}
};

}

#endif
