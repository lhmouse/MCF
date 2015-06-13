// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "Uuid.hpp"
#include "Exception.hpp"
#include "../Core/Time.hpp"
#include "../Utilities/Endian.hpp"
#include "../Thread/Atomic.hpp"

namespace MCF {

namespace {
	const unsigned g_uPid = ::GetCurrentProcessId() & 0xFFFFu;

	volatile std::uint64_t g_u64RandSeed = 0;
	volatile std::uint32_t g_u32AutoInc = 0;

	inline std::uint32_t GetRandomUint32(){
		std::uint64_t u64OldSeed, u64Seed;
		u64OldSeed = AtomicLoad(g_u64RandSeed, MemoryModel::kConsume);
		do {
			u64Seed = u64OldSeed ^ ReadTimestampCounter();
			u64Seed *= 6364136223846793005ull;
			u64Seed += 1442695040888963407ull;
		} while(!AtomicCompareExchange(g_u64RandSeed, u64OldSeed, u64Seed, MemoryModel::kAcqRel));
		return u64Seed >> 32;
	}
}

// 静态成员函数。
Uuid Uuid::Generate(){
	const auto u64Now = GetUtcTime();
	const auto u32AutoInc = AtomicAdd(g_u32AutoInc, 1, MemoryModel::kRelaxed);

	Uuid vRet(nullptr);
	StoreBe(vRet.x_unData.au32[0], u64Now >> 12);
	StoreBe(vRet.x_unData.au16[2], (u64Now << 4) | (g_uPid >> 12));
	StoreBe(vRet.x_unData.au16[3], g_uPid & 0x0FFFu); // 版本 = 0
	StoreBe(vRet.x_unData.au16[4], 0xC000u | (u32AutoInc & 0x3FFFu)); // 变种 = 3
	StoreBe(vRet.x_unData.au16[5], GetRandomUint32());
	StoreBe(vRet.x_unData.au32[3], GetRandomUint32());
	return vRet;
}

// 构造函数和析构函数。
Uuid::Uuid(const char (&pchString)[36]){
	if(!Scan(pchString)){
		DEBUG_THROW(Exception, "Invalid UUID string", ERROR_INVALID_PARAMETER);
	}
}

// 其他非静态成员函数。
void Uuid::Print(char (&pchString)[36], bool bUpperCase) const noexcept {
	auto pbyRead = GetBegin();
	auto pchWrite = pchString;

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
bool Uuid::Scan(const char (&pchString)[36]) noexcept {
	auto pchRead = pchString;
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
