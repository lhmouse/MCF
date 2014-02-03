// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2012 - 2013. LH_Mouse. All wrongs reserved.

#ifndef __MCF_MD5_HPP__
#define __MCF_MD5_HPP__

#include <cstddef>

namespace MCF {

class MD5 {
private:
	bool xm_bInited;
	std::uint32_t xm_auResult[4];

	union {
		unsigned char xm_abyChunk[64];
		struct {
			unsigned char xm_abyFirstPart[56];
			std::uint64_t xm_uBitsTotal;
		};
	};
	std::size_t xm_uBytesInChunk;
	std::uint64_t xm_u64BytesTotal;
public:
	MD5() noexcept;
public:
	void Update(const void *pData, std::size_t uSize) noexcept;
	void Finalize(unsigned char (&abyOutput)[16]) noexcept;
};

}

#endif
