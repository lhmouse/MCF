// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "Uuid.hpp"
#include "Time.hpp"
#include "Exception.hpp"
#include "../Utilities/Endian.hpp"
#include "../Random/FastGenerator.hpp"
#include "../Thread/Atomic.hpp"
using namespace MCF;

namespace {

volatile std::uint32_t g_u32AutoId = 0;

}

// 静态成员函数。
Uuid Uuid::Generate(){
	const auto u64Now = GetUtcTime();
	const auto u32Unique = AtomicIncrement(g_u32AutoId, MemoryModel::RELAXED);

	Uuid vRet(nullptr);
	StoreBe(vRet.xm_unData.au32[0], u64Now >> 28);
	StoreBe(vRet.xm_unData.au16[2], u64Now >> 12);
	StoreBe(vRet.xm_unData.au16[3], u64Now & 0x0FFFu); // 版本 = 0
	StoreBe(vRet.xm_unData.au32[2], 0xC0000000u | u32Unique); // 变种 = 3
	StoreBe(vRet.xm_unData.au32[3], FastGenerator::GlobalGet());
	return vRet;
}

// 构造函数和析构函数。
Uuid::Uuid(const char (&pszString)[37]){
	if(!Scan(pszString)){
		DEBUG_THROW(Exception, "Invalid UUID string", ERROR_INVALID_PARAMETER);
	}
}

// 其他非静态成员函数。
void Uuid::Print(char (&pszString)[37], bool bUpperCase) const noexcept {
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
	PRINT(6) *(pchWrite++) = 0;
}
bool Uuid::Scan(const char (&pszString)[37]) noexcept {
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
	SCAN(6) if(*(pchRead++) != 0){ return false; }

	return true;
}
