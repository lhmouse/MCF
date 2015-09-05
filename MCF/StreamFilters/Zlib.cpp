// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "Zlib.hpp"
#include "../Core/Exception.hpp"
#include "../Utilities/MinMax.hpp"

#define ZLIB_CONST
#define Z_PREFIX
#include "../../External/zlib/zlib.h"

namespace MCF {

namespace {
	constexpr std::size_t kStepSize = 0x4000;

	unsigned long ZlibErrorToWin32Error(int nZlibError) noexcept {
		switch(nZlibError){
		case Z_OK:
			return ERROR_SUCCESS;

		case Z_STREAM_END:
			return ERROR_HANDLE_EOF;

		case Z_NEED_DICT:
			return ERROR_INVALID_PARAMETER;

		case Z_ERRNO:
			return ERROR_OPEN_FAILED;

		case Z_STREAM_ERROR:
			return ERROR_INVALID_PARAMETER;

		case Z_DATA_ERROR:
			return ERROR_INVALID_DATA;

		case Z_MEM_ERROR:
			return ERROR_NOT_ENOUGH_MEMORY;

		case Z_BUF_ERROR:
			return ERROR_SUCCESS;

		case Z_VERSION_ERROR:
			return ERROR_NOT_SUPPORTED;

		default:
			return ERROR_INVALID_FUNCTION;
		}
	}
}

class ZlibEncoder::$Delegate {
private:
	ZlibEncoder &$vOwner;

	::z_stream $vStream;

public:
	$Delegate(ZlibEncoder &vOwner, bool bRaw, unsigned uLevel)
		: $vOwner(vOwner)
	{
		$vStream.zalloc = nullptr;
		$vStream.zfree  = nullptr;
		$vStream.opaque = nullptr;

		const auto nError = ::deflateInit2(&$vStream, (int)uLevel, Z_DEFLATED, bRaw ? -15 : 15, 9, Z_DEFAULT_STRATEGY);
		if(nError != Z_OK){
			DEBUG_THROW(ZlibError, nError, "deflateInit2");
		}
	}
	~$Delegate(){
		::deflateEnd(&$vStream);
	}

public:
	void Init(){
		const auto nError = ::deflateReset(&$vStream);
		if(nError != Z_OK){
			DEBUG_THROW(ZlibError, nError, "deflateReset");
		}
	}
	void Update(const void *pData, std::size_t uSize){
		unsigned char abyTemp[kStepSize];
		$vStream.next_out = abyTemp;
		$vStream.avail_out = sizeof(abyTemp);

		auto pbyRead = (const unsigned char *)pData;
		std::size_t uProcessed = 0;
		while(uProcessed < uSize){
			const auto uToProcess = Min(uSize - uProcessed, kStepSize);

			$vStream.next_in = pbyRead;
			$vStream.avail_in = uToProcess;
			do {
				const auto nError = ::deflate(&$vStream, Z_NO_FLUSH);
				if(nError == Z_STREAM_END){
					break;
				}
				if(nError != Z_OK){
					DEBUG_THROW(ZlibError, nError, "deflate");
				}
				if($vStream.avail_out == 0){
					$vOwner.$Output(abyTemp, sizeof(abyTemp));

					$vStream.next_out = abyTemp;
					$vStream.avail_out = sizeof(abyTemp);
				}
			} while($vStream.avail_in != 0);

			pbyRead += uToProcess;
			uProcessed += uToProcess;
		}
		if($vStream.avail_out != 0){
			$vOwner.$Output(abyTemp, sizeof(abyTemp) - $vStream.avail_out);
		}
	}
	void Finalize(){
		unsigned char abyTemp[kStepSize];
		$vStream.next_out = abyTemp;
		$vStream.avail_out = sizeof(abyTemp);

		$vStream.next_in = nullptr;
		$vStream.avail_in = 0;
		for(;;){
			const auto nError = ::deflate(&$vStream, Z_FINISH);
			if(nError == Z_STREAM_END){
				break;
			}
			if(nError != Z_OK){
				DEBUG_THROW(ZlibError, nError, "deflate");
			}
			if($vStream.avail_out == 0){
				$vOwner.$Output(abyTemp, sizeof(abyTemp));

				$vStream.next_out = abyTemp;
				$vStream.avail_out = sizeof(abyTemp);
			}
		}
		if($vStream.avail_out != 0){
			$vOwner.$Output(abyTemp, sizeof(abyTemp) - $vStream.avail_out);
		}
	}
};

class ZlibDecoder::$Delegate {
private:
	ZlibDecoder &$vOwner;

	::z_stream $vStream;

public:
	$Delegate(ZlibDecoder &vOwner, bool bRaw)
		: $vOwner(vOwner)
	{
		$vStream.zalloc   = nullptr;
		$vStream.zfree    = nullptr;
		$vStream.opaque   = nullptr;

		$vStream.next_in  = nullptr;
		$vStream.avail_in = 0;

		const auto nError = ::inflateInit2(&$vStream, bRaw ? -15 : 15);
		if(nError != Z_OK){
			DEBUG_THROW(ZlibError, nError, "inflateInit2");
		}
	}
	~$Delegate(){
		::inflateEnd(&$vStream);
	}

public:
	void Init(){
		const auto nError = ::inflateReset(&$vStream);
		if(nError != Z_OK){
			DEBUG_THROW(ZlibError, nError, "inflateReset");
		}
	}
	void Update(const void *pData, std::size_t uSize){
		unsigned char abyTemp[kStepSize];
		$vStream.next_out = abyTemp;
		$vStream.avail_out = sizeof(abyTemp);

		auto pbyRead = (const unsigned char *)pData;
		std::size_t uProcessed = 0;
		while(uProcessed < uSize){
			const auto uToProcess = Min(uSize - uProcessed, kStepSize);

			$vStream.next_in = pbyRead;
			$vStream.avail_in = uToProcess;
			do {
				const auto nError = ::inflate(&$vStream, Z_NO_FLUSH);
				if(nError == Z_STREAM_END){
					break;
				}
				if(nError != Z_OK){
					DEBUG_THROW(ZlibError, nError, "inflate");
				}
				if($vStream.avail_out == 0){
					$vOwner.$Output(abyTemp, sizeof(abyTemp));

					$vStream.next_out = abyTemp;
					$vStream.avail_out = sizeof(abyTemp);
				}
			} while($vStream.avail_in != 0);

			pbyRead += uToProcess;
			uProcessed += uToProcess;
		}
		if($vStream.avail_out != 0){
			$vOwner.$Output(abyTemp, sizeof(abyTemp) - $vStream.avail_out);
		}
	}
	void Finalize(){
		unsigned char abyTemp[kStepSize];
		$vStream.next_out = abyTemp;
		$vStream.avail_out = sizeof(abyTemp);

		$vStream.next_in = nullptr;
		$vStream.avail_in = 0;
		for(;;){
			const auto nError = ::inflate(&$vStream, Z_FINISH);
			if(nError == Z_STREAM_END){
				break;
			}
			if(nError != Z_OK){
				DEBUG_THROW(ZlibError, nError, "inflate");
			}
			if($vStream.avail_out == 0){
				$vOwner.$Output(abyTemp, sizeof(abyTemp));

				$vStream.next_out = abyTemp;
				$vStream.avail_out = sizeof(abyTemp);
			}
		}
		if($vStream.avail_out != 0){
			$vOwner.$Output(abyTemp, sizeof(abyTemp) - $vStream.avail_out);
		}
	}
};

// ========== ZlibEncoder ==========
// 静态成员函数。
ZlibEncoder::ZlibEncoder(bool bRaw, unsigned uLevel) noexcept
	: $bRaw(bRaw), $uLevel(uLevel)
{
}
ZlibEncoder::~ZlibEncoder(){
}

void ZlibEncoder::$DoInit(){
	if(!$pDelegate){
		$pDelegate.Reset(new $Delegate(*this, $bRaw, $uLevel));
	}
	$pDelegate->Init();
}
void ZlibEncoder::$DoUpdate(const void *pData, std::size_t uSize){
	$pDelegate->Update(pData, uSize);
}
void ZlibEncoder::$DoFinalize(){
	$pDelegate->Finalize();
}

// ========== ZlibDecoder ==========
// 静态成员函数。
ZlibDecoder::ZlibDecoder(bool bRaw) noexcept
	: $bRaw(bRaw)
{
}
ZlibDecoder::~ZlibDecoder(){
}

void ZlibDecoder::$DoInit(){
	if(!$pDelegate){
		$pDelegate.Reset(new $Delegate(*this, $bRaw));
	}
	$pDelegate->Init();
}
void ZlibDecoder::$DoUpdate(const void *pData, std::size_t uSize){
	$pDelegate->Update(pData, uSize);
}
void ZlibDecoder::$DoFinalize(){
	$pDelegate->Finalize();
}

// ========== ZlibError ==========
ZlibError::ZlibError(const char *pszFile, unsigned long ulLine, long lZlibError, const char *pszFunction) noexcept
	: Exception(pszFile, ulLine, ZlibErrorToWin32Error(lZlibError), pszFunction)
	, $lZlibError(lZlibError)
{
}
ZlibError::~ZlibError(){
}

}
