// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef __MCF_HEX_HPP__
#define __MCF_HEX_HPP__

#include "../Core/Utilities.hpp"
#include <memory>
#include <utility>
#include <cstddef>

namespace MCF {

class HexEncoder : NO_COPY, ABSTRACT {
public:
	static std::unique_ptr<HexEncoder> Create(
		std::function<std::pair<void *, std::size_t> (std::size_t)> fnDataCallback,
		bool bUpperCase
	);

public:
	void Abort() noexcept;
	void Update(const void *pData, std::size_t uSize);
	void Finalize();
};

class HexDecoder : NO_COPY, ABSTRACT {
public:
	static std::unique_ptr<HexDecoder> Create(
		std::function<std::pair<void *, std::size_t> (std::size_t)> fnDataCallback
	);

public:
	void Abort() noexcept;
	void Update(const void *pData, std::size_t uSize);
	void Finalize();
};

}

#endif
