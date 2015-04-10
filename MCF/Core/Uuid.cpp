// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "Uuid.hpp"
#include "Exception.hpp"
#include "../Utilities/Endian.hpp"
#include "../Random/FastGenerator.hpp"
#include "../Thread/Mutex.hpp"

namespace MCF {

namespace {
	class Generator {
	private:
		const unsigned long x_ulProcessIdHigh;

		Mutex x_vMutex;
		unsigned x_uAutoId;
		FastGenerator x_rngRandom;

	public:
		Generator()
			: x_ulProcessIdHigh(::GetCurrentProcessId() << 16)
			, x_uAutoId(0)
		{
		}

	public:
		std::pair<std::uint32_t, std::uint32_t> operator()() noexcept {
			const auto vLock = x_vMutex.GetLock();
			return std::make_pair(x_ulProcessIdHigh | (++x_uAutoId & 0xFFFFu), x_rngRandom());
		}
	} g_vGenerator __attribute__((__init_priority__(101)));
}

// 静态成员函数。
Uuid Uuid::Generate(){
	const auto u64Now = GetUtcTime();
	const auto vUnique = g_vGenerator();

	Uuid vRet(nullptr);
	StoreBe(vRet.x_unData.au32[0], u64Now >> 28);
	StoreBe(vRet.x_unData.au16[2], u64Now >> 12);
	StoreBe(vRet.x_unData.au16[3], u64Now & 0x0FFFu); // 版本 = 0
	StoreBe(vRet.x_unData.au32[2], 0xC0000000u | vUnique.first); // 变种 = 3
	StoreBe(vRet.x_unData.au32[3], vUnique.second);
	return vRet;
}

// 构造函数和析构函数。
Uuid::Uuid(const char (&pszString)[36]){
	if(!Scan(pszString)){
		DEBUG_THROW(Exception, "Invalid UUID string", ERROR_INVALID_PARAMETER);
	}
}

// 其他非静态成员函数。
void Uuid::Print(char (&pszString)[36], bool bUpperCase) const noexcept {
	auto pbyRead = GetBegin();
	auto pchWrite = pszString;

#define PRINT(count_)	\
	for(std::size_t i = 0; i < count_; ++i){	\
		const unsigned uByte = *(pbyRead++);	\
		unsigned uChar = uByte >> 4;	\
		if(uChar <= 9){	\
			uChar += '0';	\
		} else if(bUpperCase){	\
			uChar += 'A' - 0x0A;	\
		} else {	\
			uChar += 'a' - 0x0A;	\
		}	\
		*(pchWrite++) = uChar;	\
		uChar = uByte & 0x0F;	\
		if(uChar <= 9){	\
			uChar += '0';	\
		} else if(bUpperCase){	\
			uChar += 'A' - 0x0A;	\
		} else {	\
			uChar += 'a' - 0x0A;	\
		}	\
		*(pchWrite++) = uChar;	\
	}

	PRINT(4) *(pchWrite++) = '-';
	PRINT(2) *(pchWrite++) = '-';
	PRINT(2) *(pchWrite++) = '-';
	PRINT(2) *(pchWrite++) = '-';
	PRINT(6)
}
bool Uuid::Scan(const char (&pszString)[36]) noexcept {
	auto pchRead = pszString;
	auto pbyWrite = GetBegin();

#define SCAN(count_)	\
	for(std::size_t i = 0; i < count_; ++i){	\
		unsigned uByte;	\
		unsigned uChar = (unsigned char)*(pchRead++);	\
		if(('0' <= uChar) && (uChar <= '9')){	\
			uChar -= '0';	\
		} else if(('A' <= uChar) && (uChar <= 'F')){	\
			uChar -= 'A' - 0x0A;	\
		} else if(('a' <= uChar) && (uChar <= 'f')){	\
			uChar -= 'a' - 0x0A;	\
		} else {	\
			return false;	\
		}	\
		uByte = uChar << 4;	\
		uChar = (unsigned char)*(pchRead++);	\
		if(('0' <= uChar) && (uChar <= '9')){	\
			uChar -= '0';	\
		} else if(('A' <= uChar) && (uChar <= 'F')){	\
			uChar -= 'A' - 0x0A;	\
		} else if(('a' <= uChar) && (uChar <= 'f')){	\
			uChar -= 'a' - 0x0A;	\
		} else {	\
			return false;	\
		}	\
		uByte |= uChar;	\
		*(pbyWrite++) = uByte;	\
	}

	SCAN(4) if(*(pchRead++) != '-'){ return false; }
	SCAN(2) if(*(pchRead++) != '-'){ return false; }
	SCAN(2) if(*(pchRead++) != '-'){ return false; }
	SCAN(2) if(*(pchRead++) != '-'){ return false; }
	SCAN(6)

	return true;
}

}
