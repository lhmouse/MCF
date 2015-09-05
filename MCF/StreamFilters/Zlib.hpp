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
	class $Delegate;

private:
	const bool $bRaw;
	const unsigned $uLevel;

	UniquePtr<$Delegate> $pDelegate;

public:
	explicit ZlibEncoder(bool bRaw = false, unsigned uLevel = 6) noexcept;
	~ZlibEncoder();

protected:
	void $DoInit() override;
	void $DoUpdate(const void *pData, std::size_t uSize) override;
	void $DoFinalize() override;
};

class ZlibDecoder : public StreamFilterBase {
private:
	class $Delegate;

private:
	const bool $bRaw;

	UniquePtr<$Delegate> $pDelegate;

public:
	explicit ZlibDecoder(bool bRaw = false) noexcept;
	~ZlibDecoder();

protected:
	void $DoInit() override;
	void $DoUpdate(const void *pData, std::size_t uSize) override;
	void $DoFinalize() override;
};

class ZlibError : public Exception {
private:
	long $lZlibError;

public:
	ZlibError(const char *pszFile, unsigned long ulLine, long lZlibError, const char *pszFunction) noexcept;
	~ZlibError() override;

public:
	long GetZlibError() const noexcept {
		return $lZlibError;
	}
};

}

#endif
