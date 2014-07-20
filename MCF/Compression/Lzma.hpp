// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014 LH_Mouse. All wrongs reserved.

#ifndef MCF_LZMA_HPP_
#define MCF_LZMA_HPP_

#include "../Core/Utilities.hpp"
#include <memory>
#include <functional>
#include <utility>
#include <cstddef>

namespace MCF {

class LzmaEncoder : NO_COPY, ABSTRACT {
public:
	static std::unique_ptr<LzmaEncoder> Create(
		std::function<std::pair<void *, std::size_t> (std::size_t)> fnDataCallback,
		int nLevel = 5,
		std::uint32_t u32DictSize = 1u << 24
	);

public:
	void Abort() noexcept;
	void Update(const void *pData, std::size_t uSize);
	std::size_t QueryBytesProcessed() const noexcept;
	void Finalize();
};

class LzmaDecoder : NO_COPY, ABSTRACT {
public:
	static std::unique_ptr<LzmaDecoder> Create(
		std::function<std::pair<void *, std::size_t> (std::size_t)> fnDataCallback
	);

public:
	void Abort() noexcept;
	void Update(const void *pData, std::size_t uSize);
	std::size_t QueryBytesProcessed() const noexcept;
	void Finalize();
};

}

#endif
