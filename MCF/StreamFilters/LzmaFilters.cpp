// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "LzmaFilters.hpp"
#include "../Core/Exception.hpp"
#include "../Utilities/MinMax.hpp"
#include "../../External/lzmalite/lzma.h"
using namespace MCF;

namespace {

constexpr std::size_t STEP_SIZE		= 0x4000;
constexpr lzma_stream INIT_STREAM	= LZMA_STREAM_INIT;

unsigned long LzmaErrorToWin32Error(lzma_ret eLzmaError) noexcept {
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
	void operator()(lzma_stream *pStream) const noexcept {
		::lzma_end(pStream);
	}
};

lzma_options_lzma MakeOptions(unsigned uLevel, unsigned long ulDictSize){
	lzma_options_lzma vRet;
	if(::lzma_lzma_preset(&vRet, uLevel)){
		DEBUG_THROW(LzmaError, "lzma_lzma_preset", LZMA_OPTIONS_ERROR);
	}
	vRet.dict_size = ulDictSize;
	return std::move(vRet);
}

}

class LzmaEncoder::xDelegate : Noncopyable {
private:
	LzmaEncoder &xm_vOwner;
	const lzma_options_lzma xm_vOptions;

	lzma_stream xm_vStream;
	std::unique_ptr<lzma_stream, LzmaStreamCloser> xm_pStream;

public:
	xDelegate(LzmaEncoder &vOwner, unsigned uLevel, unsigned long ulDictSize)
		: xm_vOwner(vOwner) , xm_vOptions(MakeOptions(uLevel, ulDictSize))
		, xm_vStream(INIT_STREAM)
	{
	}

public:
	void Abort() noexcept {
		xm_pStream.reset();

		xm_vOwner.StreamFilterBase::Abort();
	}
	void Update(const void *pData, std::size_t uSize){
		if(!xm_pStream){
			const auto eError = ::lzma_alone_encoder(&xm_vStream, &xm_vOptions);
			if(eError != LZMA_OK){
				DEBUG_THROW(LzmaError, "lzma_alone_encoder", eError);
			}

			xm_pStream.reset(&xm_vStream);
		}

		unsigned char abyTemp[STEP_SIZE];
		xm_pStream->next_out = abyTemp;
		xm_pStream->avail_out = sizeof(abyTemp);

		auto pbyRead = (const unsigned char *)pData;
		std::size_t uProcessed = 0;
		while(uProcessed < uSize){
			const auto uToProcess = Min(uSize - uProcessed, STEP_SIZE);

			xm_pStream->next_in = pbyRead;
			xm_pStream->avail_in = uToProcess;
			do {
				const auto eError = ::lzma_code(xm_pStream.get(), LZMA_RUN);
				if(eError == LZMA_STREAM_END){
					break;
				}
				if(eError != LZMA_OK){
					DEBUG_THROW(LzmaError, "lzma_code", eError);
				}
				if(xm_pStream->avail_out == 0){
					xm_vOwner.xOutput(abyTemp, sizeof(abyTemp));

					xm_pStream->next_out = abyTemp;
					xm_pStream->avail_out = sizeof(abyTemp);
				}
			} while(xm_pStream->avail_in != 0);

			xm_vOwner.StreamFilterBase::Update(pbyRead, uToProcess);
			pbyRead += uToProcess;
			uProcessed += uToProcess;
		}
		if(xm_pStream->avail_out != 0){
			xm_vOwner.xOutput(abyTemp, sizeof(abyTemp) - xm_pStream->avail_out);
		}
	}
	void Finalize(){
		if(xm_pStream){
			unsigned char abyTemp[STEP_SIZE];
			xm_pStream->next_out = abyTemp;
			xm_pStream->avail_out = sizeof(abyTemp);

			xm_pStream->next_in = nullptr;
			xm_pStream->avail_in = 0;
			for(;;){
				const auto eError = ::lzma_code(xm_pStream.get(), LZMA_FINISH);
				if(eError == LZMA_STREAM_END){
					break;
				}
				if(eError != LZMA_OK){
					DEBUG_THROW(LzmaError, "lzma_code", eError);
				}
				if(xm_pStream->avail_out == 0){
					xm_vOwner.xOutput(abyTemp, sizeof(abyTemp));

					xm_pStream->next_out = abyTemp;
					xm_pStream->avail_out = sizeof(abyTemp);
				}
			}
			if(xm_pStream->avail_out != 0){
				xm_vOwner.xOutput(abyTemp, sizeof(abyTemp) - xm_pStream->avail_out);
			}

			xm_pStream.reset();
		}
		xm_vOwner.StreamFilterBase::Finalize();
	}
};

class LzmaDecoder::xDelegate : Noncopyable {
private:
	LzmaDecoder &xm_vOwner;

	lzma_stream xm_vStream;
	std::unique_ptr<lzma_stream, LzmaStreamCloser> xm_pStream;

public:
	explicit xDelegate(LzmaDecoder &vOwner)
		: xm_vOwner(vOwner)
		, xm_vStream(INIT_STREAM)
	{
	}

public:
	void Abort() noexcept {
		xm_pStream.reset();

		xm_vOwner.StreamFilterBase::Abort();
	}
	void Update(const void *pData, std::size_t uSize){
		if(!xm_pStream){
			const auto eError = ::lzma_alone_decoder(&xm_vStream, UINT64_MAX);
			if(eError != LZMA_OK){
				DEBUG_THROW(LzmaError, "lzma_alone_decoder", eError);
			}
			xm_pStream.reset(&xm_vStream);
		}

		unsigned char abyTemp[STEP_SIZE];
		xm_pStream->next_out = abyTemp;
		xm_pStream->avail_out = sizeof(abyTemp);

		auto pbyRead = (const unsigned char *)pData;
		std::size_t uProcessed = 0;
		while(uProcessed < uSize){
			const auto uToProcess = Min(uSize - uProcessed, STEP_SIZE);

			xm_pStream->next_in = pbyRead;
			xm_pStream->avail_in = uToProcess;
			do {
				const auto eError = ::lzma_code(xm_pStream.get(), LZMA_RUN);
				if(eError == LZMA_STREAM_END){
					break;
				}
				if(eError != LZMA_OK){
					DEBUG_THROW(LzmaError, "lzma_code", eError);
				}
				if(xm_pStream->avail_out == 0){
					xm_vOwner.xOutput(abyTemp, sizeof(abyTemp));

					xm_pStream->next_out = abyTemp;
					xm_pStream->avail_out = sizeof(abyTemp);
				}
			} while(xm_pStream->avail_in != 0);

			xm_vOwner.StreamFilterBase::Update(pbyRead, uToProcess);
			pbyRead += uToProcess;
			uProcessed += uToProcess;
		}
		if(xm_pStream->avail_out != 0){
			xm_vOwner.xOutput(abyTemp, sizeof(abyTemp) - xm_pStream->avail_out);
		}
	}
	void Finalize(){
		if(xm_pStream){
			unsigned char abyTemp[STEP_SIZE];
			xm_pStream->next_out = abyTemp;
			xm_pStream->avail_out = sizeof(abyTemp);

			xm_pStream->next_in = nullptr;
			xm_pStream->avail_in = 0;
			for(;;){
				const auto eError = ::lzma_code(xm_pStream.get(), LZMA_FINISH);
				if(eError == LZMA_STREAM_END){
					break;
				}
				if(eError != LZMA_OK){
					DEBUG_THROW(LzmaError, "lzma_code", eError);
				}
				if(xm_pStream->avail_out == 0){
					xm_vOwner.xOutput(abyTemp, sizeof(abyTemp));

					xm_pStream->next_out = abyTemp;
					xm_pStream->avail_out = sizeof(abyTemp);
				}
			}
			if(xm_pStream->avail_out != 0){
				xm_vOwner.xOutput(abyTemp, sizeof(abyTemp) - xm_pStream->avail_out);
			}

			xm_pStream.reset();
		}
		xm_vOwner.StreamFilterBase::Finalize();
	}
};

// ========== LzmaEncoder ==========
// 静态成员函数。
LzmaEncoder::LzmaEncoder(unsigned uLevel, unsigned long ulDictSize)
	: xm_pDelegate(std::make_unique<xDelegate>(*this, uLevel, ulDictSize))
{
}
LzmaEncoder::~LzmaEncoder(){
}

void LzmaEncoder::Abort() noexcept {
	xm_pDelegate->Abort();
}
void LzmaEncoder::Update(const void *pData, std::size_t uSize){
	xm_pDelegate->Update(pData, uSize);
}
void LzmaEncoder::Finalize(){
	xm_pDelegate->Finalize();
}

// ========== LzmaDecoder ==========
// 静态成员函数。
LzmaDecoder::LzmaDecoder()
	: xm_pDelegate(std::make_unique<xDelegate>(*this))
{
}
LzmaDecoder::~LzmaDecoder(){
}

void LzmaDecoder::Abort() noexcept {
	xm_pDelegate->Abort();
}
void LzmaDecoder::Update(const void *pData, std::size_t uSize){
	xm_pDelegate->Update(pData, uSize);
}
void LzmaDecoder::Finalize(){
	xm_pDelegate->Finalize();
}

// ========== LzmaError ==========
LzmaError::LzmaError(const char *pszFile, unsigned long ulLine, const char *pszMessage, long lLzmaError) noexcept
	: Exception(pszFile, ulLine, pszMessage, LzmaErrorToWin32Error(static_cast<::lzma_ret>(lLzmaError)))
	, xm_lLzmaError(lLzmaError)
{
}
LzmaError::~LzmaError(){
}
