// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "Base64.hpp"

namespace MCF {

// ========== Base64Encoder ==========
// 构造函数和析构函数。
Base64Encoder::Base64Encoder(const char *pchTable) noexcept {
	std::memcpy($abyTable, pchTable, sizeof($abyTable));
}

// 其他非静态成员函数。
void Base64Encoder::$DoInit(){
	$uState = 0;
}
void Base64Encoder::$DoUpdate(const void *pData, std::size_t uSize){
	auto pbyRead = static_cast<const unsigned char *>(pData);
	const auto pbyEnd = pbyRead + uSize;

	if(uSize > 6){
		switch($uState){
		case 0:
			break;

		case 1:
			$ulWord = ($ulWord << 8) | *pbyRead;
			++pbyRead;
			$Output($abyTable[($ulWord >> 4) & 0x3F]);
		case 2:
			$ulWord = ($ulWord << 8) | *pbyRead;
			++pbyRead;
			$Output($abyTable[($ulWord >> 6) & 0x3F]);
			$Output($abyTable[ $ulWord       & 0x3F]);
			$uState = 0;
			break;

		default:
			ASSERT(false);
		}

		register auto i = (std::size_t)(pbyEnd - pbyRead) / 3;
		while(i != 0){
			unsigned long ulWord = pbyRead[0];
			ulWord = (ulWord << 8) | pbyRead[1];
			ulWord = (ulWord << 8) | pbyRead[2];
			pbyRead += 3;

			$Output($abyTable[(ulWord >> 18) & 0x3F]);
			$Output($abyTable[(ulWord >> 12) & 0x3F]);
			$Output($abyTable[(ulWord >>  6) & 0x3F]);
			$Output($abyTable[ ulWord        & 0x3F]);

			--i;
		}
	}
	while(pbyRead != pbyEnd){
		$ulWord = ($ulWord << 8) | *pbyRead;
		++pbyRead;

		switch($uState){
		case 0:
			$Output($abyTable[($ulWord >> 2) & 0x3F]);
			$uState = 1;
			break;

		case 1:
			$Output($abyTable[($ulWord >> 4) & 0x3F]);
			$uState = 2;
			break;

		case 2:
			$Output($abyTable[($ulWord >> 6) & 0x3F]);
			$Output($abyTable[ $ulWord       & 0x3F]);
			$uState = 0;
			break;

		default:
			ASSERT(false);
		}
	}
}
void Base64Encoder::$DoFinalize(){
	switch($uState){
	case 0:
		break;

	case 1:
		$Output($abyTable[($ulWord << 4) & 0x30]);
		if($abyTable[64] != 0){
			$Output($abyTable[64]);
			$Output($abyTable[64]);
		}
		$uState = 0;
		break;

	case 2:
		$Output($abyTable[($ulWord << 2) & 0x3C]);
		if($abyTable[64] != 0){
			$Output($abyTable[64]);
		}
		$uState = 0;
		break;

	default:
		ASSERT(false);
	}
}

// ========== Base64Decoder ==========
// 构造函数和析构函数。
Base64Decoder::Base64Decoder(const char *pchTable) noexcept {
	std::memset($aschTable, -1, sizeof($aschTable));
	for(std::size_t i = 0; i < 64; ++i){
		$aschTable[(unsigned char)pchTable[i]] = i;
	}
}

// 其他非静态成员函数。
void Base64Decoder::$DoInit(){
	$uState = 0;
}
void Base64Decoder::$DoUpdate(const void *pData, std::size_t uSize){
	auto pbyRead = static_cast<const unsigned char *>(pData);
	const auto pbyEnd = pbyRead + uSize;

	while(pbyRead != pbyEnd){
		const int nDigit  = $aschTable[*pbyRead];
		++pbyRead;
		if(nDigit == -1){
			continue;
		}

		$ulWord = ($ulWord << 6) | (unsigned char)nDigit;
		switch($uState){
		case 0:
			$uState = 1;
			break;

		case 1:
			$Output((unsigned char)($ulWord >> 4));
			$uState = 2;
			break;

		case 2:
			$Output((unsigned char)($ulWord >> 2));
			$uState = 3;
			break;

		case 3:
			$Output((unsigned char)$ulWord);
			$uState = 0;
			break;

		default:
			ASSERT(false);
		}
	}
}
void Base64Decoder::$DoFinalize(){
}

}
