// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_STREAM_FILTERS_ZLIB_HPP_
#define MCF_STREAM_FILTERS_ZLIB_HPP_

#include "StreamFilterBase.hpp"
#include "../Core/Exception.hpp"
#include "../SmartPointers/UniquePtr.hpp"

namespace MCF {

class ZlibEncoder : public StreamFilterBase {
private:
	class XDelegate;

private:
	const bool x_bRaw;
	const unsigned x_uLevel;

	UniquePtr<XDelegate> x_pDelegate;

public:
	explicit ZlibEncoder(bool bRaw = false, unsigned uLevel = 6) noexcept;
	~ZlibEncoder();

protected:
	void XDoInit() override;
	void XDoUpdate(const void *pData, std::size_t uSize) override;
	void XDoFinalize() override;
};

class ZlibDecoder : public StreamFilterBase {
private:
	class XDelegate;

private:
	const bool x_bRaw;

	UniquePtr<XDelegate> x_pDelegate;

public:
	explicit ZlibDecoder(bool bRaw = false) noexcept;
	~ZlibDecoder();

protected:
	void XDoInit() override;
	void XDoUpdate(const void *pData, std::size_t uSize) override;
	void XDoFinalize() override;
};

class ZlibError : public Exception {
private:
	long x_lZlibError;

public:
	ZlibError(const char *pszFile, unsigned long ulLine, long lZlibError, const char *pszFunction) noexcept;
	~ZlibError() override;

public:
	long GetZlibError() const noexcept {
		return x_lZlibError;
	}
};

}

#endif
