// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "Lzma.hpp"
#include "../Core/Exception.hpp"
#include "../Utilities/MinMax.hpp"
#include "../../External/lzmalite/lzma.h"

namespace MCF {

namespace {
	constexpr std::size_t kStepSize     = 0x4000;
	constexpr ::lzma_stream kInitStream = LZMA_STREAM_INIT;

	unsigned long LzmaErrorToWin32Error(::lzma_ret eLzmaError) noexcept {
		switch(eLzmaError){
		case LZMA_OK:
			return ERROR_SUCCESS;

		case LZMA_STREAM_END:
			return ERROR_HANDLE_EOF;

		case LZMA_NO_CHECK:
		case LZMA_UNSUPPORTED_CHECK:
		case LZMA_GET_CHECK:
			return ERROR_INVALID_DATA;

		case LZMA_MEM_ERROR:
		case LZMA_MEMLIMIT_ERROR:
			return ERROR_NOT_ENOUGH_MEMORY;

		case LZMA_FORMAT_ERROR:
			return ERROR_BAD_FORMAT;

		case LZMA_OPTIONS_ERROR:
			return ERROR_INVALID_PARAMETER;

		case LZMA_DATA_ERROR:
			return ERROR_INVALID_DATA;

		case LZMA_BUF_ERROR:
			return ERROR_SUCCESS;

		case LZMA_PROG_ERROR:
			return ERROR_INVALID_DATA;

		default:
			return ERROR_INVALID_FUNCTION;
		}
	}

	struct LzmaStreamCloser {
		constexpr ::lzma_stream *operator()() const noexcept {
			return nullptr;
		}
		void operator()(::lzma_stream *pStream) const noexcept {
			::lzma_end(pStream);
		}
	};

	::lzma_options_lzma MakeOptions(unsigned uLevel, unsigned long ulDictSize){
		::lzma_options_lzma vRet;
		if(::lzma_lzma_preset(&vRet, uLevel)){
			DEBUG_THROW(LzmaError, LZMA_OPTIONS_ERROR, "lzma_lzma_preset");
		}
		vRet.dict_size = ulDictSize;
		return vRet;
	}
}

class LzmaEncoder::$Delegate {
private:
	LzmaEncoder &$vOwner;
	const ::lzma_options_lzma $vOptions;

	::lzma_stream $vStream;
	UniquePtr<::lzma_stream, LzmaStreamCloser> $pStream;

public:
	$Delegate(LzmaEncoder &vOwner, unsigned uLevel, unsigned long ulDictSize)
		: $vOwner(vOwner), $vOptions(MakeOptions(uLevel, ulDictSize))
		, $vStream(kInitStream)
	{
	}

public:
	void Init(){
		$pStream.Reset();

		const auto eError = ::lzma_alone_encoder(&$vStream, &$vOptions);
		if(eError != LZMA_OK){
			DEBUG_THROW(LzmaError, eError, "lzma_alone_encoder");
		}
		$pStream.Reset(&$vStream);
	}
	void Update(const void *pData, std::size_t uSize){
		unsigned char abyTemp[kStepSize];
		$pStream->next_out = abyTemp;
		$pStream->avail_out = sizeof(abyTemp);

		auto pbyRead = (const unsigned char *)pData;
		std::size_t uProcessed = 0;
		while(uProcessed < uSize){
			const auto uToProcess = Min(uSize - uProcessed, kStepSize);

			$pStream->next_in = pbyRead;
			$pStream->avail_in = uToProcess;
			do {
				const auto eError = ::lzma_code($pStream.Get(), LZMA_RUN);
				if(eError == LZMA_STREAM_END){
					break;
				}
				if(eError != LZMA_OK){
					DEBUG_THROW(LzmaError, eError, "lzma_code");
				}
				if($pStream->avail_out == 0){
					$vOwner.$Output(abyTemp, sizeof(abyTemp));

					$pStream->next_out = abyTemp;
					$pStream->avail_out = sizeof(abyTemp);
				}
			} while($pStream->avail_in != 0);

			pbyRead += uToProcess;
			uProcessed += uToProcess;
		}
		if($pStream->avail_out != 0){
			$vOwner.$Output(abyTemp, sizeof(abyTemp) - $pStream->avail_out);
		}
	}
	void Finalize(){
		unsigned char abyTemp[kStepSize];
		$pStream->next_out  = abyTemp;
		$pStream->avail_out = sizeof(abyTemp);

		$pStream->next_in   = nullptr;
		$pStream->avail_in  = 0;
		for(;;){
			const auto eError = ::lzma_code($pStream.Get(), LZMA_FINISH);
			if(eError == LZMA_STREAM_END){
				break;
			}
			if(eError != LZMA_OK){
				DEBUG_THROW(LzmaError, eError, "lzma_code");
			}
			if($pStream->avail_out == 0){
				$vOwner.$Output(abyTemp, sizeof(abyTemp));

				$pStream->next_out = abyTemp;
				$pStream->avail_out = sizeof(abyTemp);
			}
		}
		if($pStream->avail_out != 0){
			$vOwner.$Output(abyTemp, sizeof(abyTemp) - $pStream->avail_out);
		}
	}
};

class LzmaDecoder::$Delegate {
private:
	LzmaDecoder &$vOwner;

	::lzma_stream $vStream;
	UniquePtr<::lzma_stream, LzmaStreamCloser> $pStream;

public:
	explicit $Delegate(LzmaDecoder &vOwner)
		: $vOwner(vOwner)
		, $vStream(kInitStream)
	{
	}

public:
	void Init() noexcept {
		$pStream.Reset();

		const auto eError = ::lzma_alone_decoder(&$vStream, UINT64_MAX);
		if(eError != LZMA_OK){
			DEBUG_THROW(LzmaError, eError, "lzma_alone_decoder");
		}
		$pStream.Reset(&$vStream);
	}
	void Update(const void *pData, std::size_t uSize){
		unsigned char abyTemp[kStepSize];
		$pStream->next_out = abyTemp;
		$pStream->avail_out = sizeof(abyTemp);

		auto pbyRead = (const unsigned char *)pData;
		std::size_t uProcessed = 0;
		while(uProcessed < uSize){
			const auto uToProcess = Min(uSize - uProcessed, kStepSize);

			$pStream->next_in = pbyRead;
			$pStream->avail_in = uToProcess;
			do {
				const auto eError = ::lzma_code($pStream.Get(), LZMA_RUN);
				if(eError == LZMA_STREAM_END){
					break;
				}
				if(eError != LZMA_OK){
					DEBUG_THROW(LzmaError, eError, "lzma_code");
				}
				if($pStream->avail_out == 0){
					$vOwner.$Output(abyTemp, sizeof(abyTemp));

					$pStream->next_out = abyTemp;
					$pStream->avail_out = sizeof(abyTemp);
				}
			} while($pStream->avail_in != 0);

			pbyRead += uToProcess;
			uProcessed += uToProcess;
		}
		if($pStream->avail_out != 0){
			$vOwner.$Output(abyTemp, sizeof(abyTemp) - $pStream->avail_out);
		}
	}
	void Finalize(){
		unsigned char abyTemp[kStepSize];
		$pStream->next_out  = abyTemp;
		$pStream->avail_out = sizeof(abyTemp);

		$pStream->next_in   = nullptr;
		$pStream->avail_in  = 0;
		for(;;){
			const auto eError = ::lzma_code($pStream.Get(), LZMA_FINISH);
			if(eError == LZMA_STREAM_END){
				break;
			}
			if(eError != LZMA_OK){
				DEBUG_THROW(LzmaError, eError, "lzma_code");
			}
			if($pStream->avail_out == 0){
				$vOwner.$Output(abyTemp, sizeof(abyTemp));

				$pStream->next_out = abyTemp;
				$pStream->avail_out = sizeof(abyTemp);
			}
		}
		if($pStream->avail_out != 0){
			$vOwner.$Output(abyTemp, sizeof(abyTemp) - $pStream->avail_out);
		}
	}
};

// ========== LzmaEncoder ==========
// 静态成员函数。
LzmaEncoder::LzmaEncoder(unsigned uLevel, unsigned long ulDictSize) noexcept
	: $uLevel(uLevel), $ulDictSize(ulDictSize)
{
}
LzmaEncoder::~LzmaEncoder(){
}

void LzmaEncoder::$DoInit(){
	if(!$pDelegate){
		$pDelegate.Reset(new $Delegate(*this, $uLevel, $ulDictSize));
	}
	$pDelegate->Init();
}
void LzmaEncoder::$DoUpdate(const void *pData, std::size_t uSize){
	$pDelegate->Update(pData, uSize);
}
void LzmaEncoder::$DoFinalize(){
	$pDelegate->Finalize();
}


// ========== LzmaDecoder ==========
// 静态成员函数。
LzmaDecoder::LzmaDecoder() noexcept {
}
LzmaDecoder::~LzmaDecoder(){
}

void LzmaDecoder::$DoInit(){
	if(!$pDelegate){
		$pDelegate.Reset(new $Delegate(*this));
	}
	$pDelegate->Init();
}
void LzmaDecoder::$DoUpdate(const void *pData, std::size_t uSize){
	$pDelegate->Update(pData, uSize);
}
void LzmaDecoder::$DoFinalize(){
	$pDelegate->Finalize();
}

// ========== LzmaError ==========
LzmaError::LzmaError(const char *pszFile, unsigned long ulLine, long lLzmaError, const char *pszFunction) noexcept
	: Exception(pszFile, ulLine, LzmaErrorToWin32Error(static_cast<::lzma_ret>(lLzmaError)), pszFunction)
	, $lLzmaError(lLzmaError)
{
}
LzmaError::~LzmaError(){
}

}
