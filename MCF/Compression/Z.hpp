// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef __MCF_Z_HPP__
#define __MCF_Z_HPP__

#include "../Core/Utilities.hpp"
#include <memory>
#include <functional>
#include <utility>
#include <cstddef>

namespace MCF {

class ZEncoder : NO_COPY, ABSTRACT {
public:
	static std::unique_ptr<ZEncoder> Create(
		std::function<std::pair<void *, std::size_t> (std::size_t)> fnDataCallback,
		bool bRaw = false,
		int nLevel = 6
	);

public:
	void Abort() noexcept;
	void Update(const void *pData, std::size_t uSize);
	std::size_t QueryBytesProcessed() const noexcept;
	void Finalize();
};

class ZDecoder : NO_COPY, ABSTRACT {
public:
	static std::unique_ptr<ZDecoder> Create(
		std::function<std::pair<void *, std::size_t> (std::size_t)> fnDataCallback,
		bool bRaw = false
	);

public:
	void Abort() noexcept;
	void Update(const void *pData, std::size_t uSize);
	std::size_t QueryBytesProcessed() const noexcept;
	void Finalize();
};

}

#endif
