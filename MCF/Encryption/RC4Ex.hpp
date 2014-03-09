// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef __MCF_RC4EX_HPP__
#define __MCF_RC4EX_HPP__

#include <memory>
#include <functional>
#include <utility>
#include <cstddef>

namespace MCF {

class RC4ExEncoder {
private:
	const std::function<std::pair<void *, std::size_t> (std::size_t)> xm_fnDataCallback;
	unsigned char xm_abyInitBox[256];
	bool xm_bInited;

	unsigned char xm_abyBox[256];
	unsigned char xm_byI;
	unsigned char xm_byJ;

public:
	RC4ExEncoder(std::function<std::pair<void *, std::size_t> (std::size_t)> fnDataCallback, const void *pKey, std::size_t uKeyLen, std::uint64_t u64Nonce);

public:
	void Abort() noexcept;
	void Update(const void *pData, std::size_t uSize);
	void Finalize();
};

class RC4ExDecoder {
private:
	const std::function<std::pair<void *, std::size_t> (std::size_t)> xm_fnDataCallback;
	unsigned char xm_abyInitBox[256];
	bool xm_bInited;

	unsigned char xm_abyBox[256];
	unsigned char xm_byI;
	unsigned char xm_byJ;

public:
	RC4ExDecoder(std::function<std::pair<void *, std::size_t> (std::size_t)> fnDataCallback, const void *pKey, std::size_t uKeyLen, std::uint64_t u64Nonce);

public:
	void Abort() noexcept;
	void Update(const void *pData, std::size_t uSize);
	void Finalize();
};

}

#endif
