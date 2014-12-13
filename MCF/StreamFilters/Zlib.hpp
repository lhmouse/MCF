// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_STREAM_FILTERS_ZLIB_HPP_
#define MCF_STREAM_FILTERS_ZLIB_HPP_

#include "StreamFilterBase.hpp"
#include "../Core/Exception.hpp"
#include <memory>

namespace MCF {

class ZlibEncoder : public StreamFilterBase {
private:
	class xDelegate;

private:
	const bool xm_bRaw;
	const unsigned xm_uLevel;

	std::unique_ptr<xDelegate> xm_pDelegate;

public:
	explicit ZlibEncoder(bool bRaw = false, unsigned uLevel = 6) noexcept;
	~ZlibEncoder();

protected:
	void xDoInit() override;
	void xDoUpdate(const void *pData, std::size_t uSize) override;
	void xDoFinalize() override;
};

class ZlibDecoder : public StreamFilterBase {
private:
	class xDelegate;

private:
	const bool xm_bRaw;

	std::unique_ptr<xDelegate> xm_pDelegate;

public:
	explicit ZlibDecoder(bool bRaw = false) noexcept;
	~ZlibDecoder();

protected:
	void xDoInit() override;
	void xDoUpdate(const void *pData, std::size_t uSize) override;
	void xDoFinalize() override;
};

class ZlibError : public Exception {
private:
	long xm_lZlibError;

public:
	ZlibError(const char *pszFile, unsigned long ulLine, const char *pszMessage, long lZlibError) noexcept;
	~ZlibError() override;

public:
	long GetZlibError() const noexcept {
		return xm_lZlibError;
	}
};

}

#endif
