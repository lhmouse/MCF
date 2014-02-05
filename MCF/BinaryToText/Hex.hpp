// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef __MCF_HEX_HPP__
#define __MCF_HEX_HPP__

#include <memory>
#include <cstddef>

namespace MCF {

class HexEncoder {
private:
	const std::function<void *(std::size_t)> xm_fnDataCallback;
	const unsigned char xm_byDelta;
public:
	HexEncoder(std::function<void *(std::size_t)> fnDataCallback, bool bUpperCase);
public:
	void Update(const void *pData, std::size_t uSize);
	void Finalize();
};

class HexDecoder {
private:
	const std::function<void *(std::size_t)> xm_fnDataCallback;
	bool xm_bInited;

	unsigned char xm_uchLastDigit;
public:
	HexDecoder(std::function<void *(std::size_t)> fnDataCallback);
public:
	void Update(const void *pData, std::size_t uSize);
	void Finalize();
};

}

#endif
