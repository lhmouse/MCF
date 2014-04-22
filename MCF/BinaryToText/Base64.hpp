// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef MCF_BASE64_HPP_
#define MCF_BASE64_HPP_

#include <memory>
#include <utility>
#include <cstddef>

namespace MCF {

struct Base64Base {
	static const char ENCODED_CHARS_MIME	[];	// A-Z a-z 0-9 +/ =
	static const char ENCODED_CHARS_URL		[];	//             *- 无填充
	static const char ENCODED_CHARS_REGEXP	[];	//             !-
};

class Base64Encoder : public Base64Base {
private:
	const std::function<std::pair<void *, std::size_t> (std::size_t)> xm_fnDataCallback;
	char xm_achTable[65];

	std::uint32_t xm_u32Word;
	std::size_t xm_uState;

public:
	Base64Encoder(std::function<std::pair<void *, std::size_t> (std::size_t)> fnDataCallback, const char *pchEncodedChars = ENCODED_CHARS_MIME);

public:
	void Abort() noexcept;
	void Update(const void *pData, std::size_t uSize);
	void Finalize();
};

class Base64Decoder : public Base64Base {
private:
	const std::function<std::pair<void *, std::size_t> (std::size_t)> xm_fnDataCallback;
	unsigned char xm_achTable[0x100];

	std::uint32_t xm_u32Word;
	std::size_t xm_uState;

public:
	Base64Decoder(std::function<std::pair<void *, std::size_t> (std::size_t)> fnDataCallback, const char *pchEncodedChars = ENCODED_CHARS_MIME);

public:
	void Abort() noexcept;
	void Update(const void *pData, std::size_t uSize);
	void Finalize();
};

}

#endif
