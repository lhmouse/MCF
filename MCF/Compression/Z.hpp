// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef __MCF_Z_HPP__
#define __MCF_Z_HPP__

#include "../Core/NoCopy.hpp"
#include <memory>
#include <functional>
#include <utility>
#include <cstddef>

namespace MCF {

class ZEncoder : NO_COPY {
private:
	class xDelegate;

private:
	std::unique_ptr<xDelegate> xm_pDelegate;

public:
	ZEncoder(std::function<std::pair<void *, std::size_t> (std::size_t)> fnDataCallback, int nLevel = 6);
	ZEncoder(ZEncoder &&rhs) noexcept;
	ZEncoder &operator=(ZEncoder &&rhs) noexcept;
	~ZEncoder();

public:
	void Abort() noexcept;
	void Update(const void *pData, std::size_t uSize, const void *pDict = nullptr, std::size_t uDictSize = 0);
	std::size_t QueryBytesProcessed() const noexcept;
	void Finalize();
};

class ZDecoder : NO_COPY {
private:
	class xDelegate;

private:
	std::unique_ptr<xDelegate> xm_pDelegate;

public:
	ZDecoder(std::function<std::pair<void *, std::size_t> (std::size_t)> fnDataCallback);
	ZDecoder(ZDecoder &&rhs) noexcept;
	ZDecoder &operator=(ZDecoder &&rhs) noexcept;
	~ZDecoder();

public:
	void Abort() noexcept;
	void Update(const void *pData, std::size_t uSize, const void *pDict = nullptr, std::size_t uDictSize = 0);
	std::size_t QueryBytesProcessed() const noexcept;
	void Finalize();
};

}

#endif
