// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

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
			DEBUG_THROW(LzmaError, LZMA_OPTIONS_ERROR, "lzma_lzma_preset"_rcs);
		}
		vRet.dict_size = ulDictSize;
		return vRet;
	}
}

class LzmaEncoder::X_Delegate {
private:
	LzmaEncoder &x_vOwner;
	const ::lzma_options_lzma x_vOptions;

	::lzma_stream x_vStream;
	UniquePtr<::lzma_stream, LzmaStreamCloser> x_pStream;

public:
	X_Delegate(LzmaEncoder &vOwner, unsigned uLevel, unsigned long ulDictSize)
		: x_vOwner(vOwner), x_vOptions(MakeOptions(uLevel, ulDictSize))
		, x_vStream(kInitStream)
	{
	}

public:
	void Init(){
		x_pStream.Reset();

		const auto eError = ::lzma_alone_encoder(&x_vStream, &x_vOptions);
		if(eError != LZMA_OK){
			DEBUG_THROW(LzmaError, eError, "lzma_alone_encoder"_rcs);
		}
		x_pStream.Reset(&x_vStream);
	}
	void Update(const void *pData, std::size_t uSize){
		unsigned char abyTemp[kStepSize];
		x_pStream->next_out = abyTemp;
		x_pStream->avail_out = sizeof(abyTemp);

		auto pbyRead = (const unsigned char *)pData;
		std::size_t uProcessed = 0;
		while(uProcessed < uSize){
			const auto uToProcess = Min(uSize - uProcessed, kStepSize);

			x_pStream->next_in = pbyRead;
			x_pStream->avail_in = uToProcess;
			do {
				const auto eError = ::lzma_code(x_pStream.Get(), LZMA_RUN);
				if(eError == LZMA_STREAM_END){
					break;
				}
				if(eError != LZMA_OK){
					DEBUG_THROW(LzmaError, eError, "lzma_code"_rcs);
				}
				if(x_pStream->avail_out == 0){
					x_vOwner.X_Output(abyTemp, sizeof(abyTemp));

					x_pStream->next_out = abyTemp;
					x_pStream->avail_out = sizeof(abyTemp);
				}
			} while(x_pStream->avail_in != 0);

			pbyRead += uToProcess;
			uProcessed += uToProcess;
		}
		if(x_pStream->avail_out != 0){
			x_vOwner.X_Output(abyTemp, sizeof(abyTemp) - x_pStream->avail_out);
		}
	}
	void Finalize(){
		unsigned char abyTemp[kStepSize];
		x_pStream->next_out  = abyTemp;
		x_pStream->avail_out = sizeof(abyTemp);

		x_pStream->next_in   = nullptr;
		x_pStream->avail_in  = 0;
		for(;;){
			const auto eError = ::lzma_code(x_pStream.Get(), LZMA_FINISH);
			if(eError == LZMA_STREAM_END){
				break;
			}
			if(eError != LZMA_OK){
				DEBUG_THROW(LzmaError, eError, "lzma_code"_rcs);
			}
			if(x_pStream->avail_out == 0){
				x_vOwner.X_Output(abyTemp, sizeof(abyTemp));

				x_pStream->next_out = abyTemp;
				x_pStream->avail_out = sizeof(abyTemp);
			}
		}
		if(x_pStream->avail_out != 0){
			x_vOwner.X_Output(abyTemp, sizeof(abyTemp) - x_pStream->avail_out);
		}
	}
};

class LzmaDecoder::X_Delegate {
private:
	LzmaDecoder &x_vOwner;

	::lzma_stream x_vStream;
	UniquePtr<::lzma_stream, LzmaStreamCloser> x_pStream;

public:
	explicit X_Delegate(LzmaDecoder &vOwner)
		: x_vOwner(vOwner)
		, x_vStream(kInitStream)
	{
	}

public:
	void Init() noexcept {
		x_pStream.Reset();

		const auto eError = ::lzma_alone_decoder(&x_vStream, UINT64_MAX);
		if(eError != LZMA_OK){
			DEBUG_THROW(LzmaError, eError, "lzma_alone_decoder"_rcs);
		}
		x_pStream.Reset(&x_vStream);
	}
	void Update(const void *pData, std::size_t uSize){
		unsigned char abyTemp[kStepSize];
		x_pStream->next_out = abyTemp;
		x_pStream->avail_out = sizeof(abyTemp);

		auto pbyRead = (const unsigned char *)pData;
		std::size_t uProcessed = 0;
		while(uProcessed < uSize){
			const auto uToProcess = Min(uSize - uProcessed, kStepSize);

			x_pStream->next_in = pbyRead;
			x_pStream->avail_in = uToProcess;
			do {
				const auto eError = ::lzma_code(x_pStream.Get(), LZMA_RUN);
				if(eError == LZMA_STREAM_END){
					break;
				}
				if(eError != LZMA_OK){
					DEBUG_THROW(LzmaError, eError, "lzma_code"_rcs);
				}
				if(x_pStream->avail_out == 0){
					x_vOwner.X_Output(abyTemp, sizeof(abyTemp));

					x_pStream->next_out = abyTemp;
					x_pStream->avail_out = sizeof(abyTemp);
				}
			} while(x_pStream->avail_in != 0);

			pbyRead += uToProcess;
			uProcessed += uToProcess;
		}
		if(x_pStream->avail_out != 0){
			x_vOwner.X_Output(abyTemp, sizeof(abyTemp) - x_pStream->avail_out);
		}
	}
	void Finalize(){
		unsigned char abyTemp[kStepSize];
		x_pStream->next_out  = abyTemp;
		x_pStream->avail_out = sizeof(abyTemp);

		x_pStream->next_in   = nullptr;
		x_pStream->avail_in  = 0;
		for(;;){
			const auto eError = ::lzma_code(x_pStream.Get(), LZMA_FINISH);
			if(eError == LZMA_STREAM_END){
				break;
			}
			if(eError != LZMA_OK){
				DEBUG_THROW(LzmaError, eError, "lzma_code"_rcs);
			}
			if(x_pStream->avail_out == 0){
				x_vOwner.X_Output(abyTemp, sizeof(abyTemp));

				x_pStream->next_out = abyTemp;
				x_pStream->avail_out = sizeof(abyTemp);
			}
		}
		if(x_pStream->avail_out != 0){
			x_vOwner.X_Output(abyTemp, sizeof(abyTemp) - x_pStream->avail_out);
		}
	}
};

// ========== LzmaEncoder ==========
// 静态成员函数。
LzmaEncoder::LzmaEncoder(unsigned uLevel, unsigned long ulDictSize) noexcept
	: x_uLevel(uLevel), x_ulDictSize(ulDictSize)
{
}
LzmaEncoder::~LzmaEncoder(){
}

void LzmaEncoder::X_DoInit(){
	if(!x_pDelegate){
		x_pDelegate.Reset(new X_Delegate(*this, x_uLevel, x_ulDictSize));
	}
	x_pDelegate->Init();
}
void LzmaEncoder::X_DoUpdate(const void *pData, std::size_t uSize){
	x_pDelegate->Update(pData, uSize);
}
void LzmaEncoder::X_DoFinalize(){
	x_pDelegate->Finalize();
}


// ========== LzmaDecoder ==========
// 静态成员函数。
LzmaDecoder::LzmaDecoder() noexcept {
}
LzmaDecoder::~LzmaDecoder(){
}

void LzmaDecoder::X_DoInit(){
	if(!x_pDelegate){
		x_pDelegate.Reset(new X_Delegate(*this));
	}
	x_pDelegate->Init();
}
void LzmaDecoder::X_DoUpdate(const void *pData, std::size_t uSize){
	x_pDelegate->Update(pData, uSize);
}
void LzmaDecoder::X_DoFinalize(){
	x_pDelegate->Finalize();
}

// ========== LzmaError ==========
LzmaError::LzmaError(const char *pszFile, unsigned long ulLine, long lLzmaError, RefCountingNtmbs rcsFunction) noexcept
	: Exception(pszFile, ulLine, LzmaErrorToWin32Error(static_cast<::lzma_ret>(lLzmaError)), std::move(rcsFunction))
	, x_lLzmaError(lLzmaError)
{
}
LzmaError::~LzmaError(){
}

}
