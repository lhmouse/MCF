// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "ZlibFilters.hpp"
#include "../Core/Exception.hpp"
#include "../Utilities/MinMax.hpp"

#define ZLIB_CONST
#define Z_PREFIX
#include "../../External/zlib/zlib.h"
using namespace MCF;

namespace {

constexpr std::size_t STEP_SIZE = 0x4000;

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

class ZlibEncoder::xDelegate : NONCOPYABLE {
private:
	ZlibEncoder &xm_vOwner;

	z_stream xm_vStream;
	bool xm_bInited;

public:
	xDelegate(ZlibEncoder &vOwner, bool bRaw, unsigned uLevel)
		: xm_vOwner(vOwner)
	{
		xm_vStream.zalloc	= nullptr;
		xm_vStream.zfree	= nullptr;
		xm_vStream.opaque	= nullptr;

		const auto nError = ::deflateInit2(&xm_vStream, (int)uLevel, Z_DEFLATED, bRaw ? -15 : 15, 9, Z_DEFAULT_STRATEGY);
		if(nError != Z_OK){
			DEBUG_THROW(ZlibError, "deflateInit2", nError);
		}

		xm_bInited = false;
	}
	~xDelegate(){
		::deflateEnd(&xm_vStream);
	}

public:
	void Abort() noexcept {
		xm_bInited = false;
		xm_vOwner.StreamFilterBase::Abort();
	}
	void Update(const void *pData, std::size_t uSize){
		if(!xm_bInited){
			::deflateReset(&xm_vStream);
			xm_bInited = true;
		}

		unsigned char abyTemp[STEP_SIZE];
		xm_vStream.next_out = abyTemp;
		xm_vStream.avail_out = sizeof(abyTemp);

		auto pbyRead = (const unsigned char *)pData;
		std::size_t uProcessed = 0;
		while(uProcessed < uSize){
			const auto uToProcess = Min(uSize - uProcessed, STEP_SIZE);

			xm_vStream.next_in = pbyRead;
			xm_vStream.avail_in = uToProcess;
			do {
				const auto nError = ::deflate(&xm_vStream, Z_NO_FLUSH);
				if(nError == Z_STREAM_END){
					break;
				}
				if(nError != Z_OK){
					DEBUG_THROW(ZlibError, "deflate", nError);
				}
				if(xm_vStream.avail_out == 0){
					xm_vOwner.xOutput(abyTemp, sizeof(abyTemp));

					xm_vStream.next_out = abyTemp;
					xm_vStream.avail_out = sizeof(abyTemp);
				}
			} while(xm_vStream.avail_in != 0);

			xm_vOwner.StreamFilterBase::Update(pbyRead, uToProcess);
			pbyRead += uToProcess;
			uProcessed += uToProcess;
		}
		if(xm_vStream.avail_out != 0){
			xm_vOwner.xOutput(abyTemp, sizeof(abyTemp) - xm_vStream.avail_out);
		}
	}
	void Finalize(){
		if(xm_bInited){
			unsigned char abyTemp[STEP_SIZE];
			xm_vStream.next_out = abyTemp;
			xm_vStream.avail_out = sizeof(abyTemp);

			xm_vStream.next_in = nullptr;
			xm_vStream.avail_in = 0;
			for(;;){
				const auto nError = ::deflate(&xm_vStream, Z_FINISH);
				if(nError == Z_STREAM_END){
					break;
				}
				if(nError != Z_OK){
					DEBUG_THROW(ZlibError, "deflate", nError);
				}
				if(xm_vStream.avail_out == 0){
					xm_vOwner.xOutput(abyTemp, sizeof(abyTemp));

					xm_vStream.next_out = abyTemp;
					xm_vStream.avail_out = sizeof(abyTemp);
				}
			}
			if(xm_vStream.avail_out != 0){
				xm_vOwner.xOutput(abyTemp, sizeof(abyTemp) - xm_vStream.avail_out);
			}

			xm_bInited = false;
		}
		xm_vOwner.StreamFilterBase::Finalize();
	}
};

class ZlibDecoder::xDelegate : NONCOPYABLE {
private:
	ZlibDecoder &xm_vOwner;

	z_stream xm_vStream;
	bool xm_bInited;

public:
	xDelegate(ZlibDecoder &vOwner, bool bRaw)
		: xm_vOwner(vOwner)
	{
		xm_vStream.zalloc	= nullptr;
		xm_vStream.zfree	= nullptr;
		xm_vStream.opaque	= nullptr;

		xm_vStream.next_in	= nullptr;
		xm_vStream.avail_in	= 0;

		const auto nError = ::inflateInit2(&xm_vStream, bRaw ? -15 : 15);
		if(nError != Z_OK){
			DEBUG_THROW(ZlibError, "inflateInit2", nError);
		}

		xm_bInited = false;
	}
	~xDelegate(){
		::inflateEnd(&xm_vStream);
	}

public:
	void Abort() noexcept {
		xm_bInited = false;
		xm_vOwner.StreamFilterBase::Abort();
	}
	void Update(const void *pData, std::size_t uSize){
		if(!xm_bInited){
			::inflateReset(&xm_vStream);
			xm_bInited = true;
		}

		unsigned char abyTemp[STEP_SIZE];
		xm_vStream.next_out = abyTemp;
		xm_vStream.avail_out = sizeof(abyTemp);

		auto pbyRead = (const unsigned char *)pData;
		std::size_t uProcessed = 0;
		while(uProcessed < uSize){
			const auto uToProcess = Min(uSize - uProcessed, STEP_SIZE);

			xm_vStream.next_in = pbyRead;
			xm_vStream.avail_in = uToProcess;
			do {
				const auto nError = ::inflate(&xm_vStream, Z_NO_FLUSH);
				if(nError == Z_STREAM_END){
					break;
				}
				if(nError != Z_OK){
					DEBUG_THROW(ZlibError, "inflate", nError);
				}
				if(xm_vStream.avail_out == 0){
					xm_vOwner.xOutput(abyTemp, sizeof(abyTemp));

					xm_vStream.next_out = abyTemp;
					xm_vStream.avail_out = sizeof(abyTemp);
				}
			} while(xm_vStream.avail_in != 0);

			xm_vOwner.StreamFilterBase::Update(pbyRead, uToProcess);
			pbyRead += uToProcess;
			uProcessed += uToProcess;
		}
		if(xm_vStream.avail_out != 0){
			xm_vOwner.xOutput(abyTemp, sizeof(abyTemp) - xm_vStream.avail_out);
		}
	}
	void Finalize(){
		if(xm_bInited){
			unsigned char abyTemp[STEP_SIZE];
			xm_vStream.next_out = abyTemp;
			xm_vStream.avail_out = sizeof(abyTemp);

			xm_vStream.next_in = nullptr;
			xm_vStream.avail_in = 0;
			for(;;){
				const auto nError = ::inflate(&xm_vStream, Z_FINISH);
				if(nError == Z_STREAM_END){
					break;
				}
				if(nError != Z_OK){
					DEBUG_THROW(ZlibError, "inflate", nError);
				}
				if(xm_vStream.avail_out == 0){
					xm_vOwner.xOutput(abyTemp, sizeof(abyTemp));

					xm_vStream.next_out = abyTemp;
					xm_vStream.avail_out = sizeof(abyTemp);
				}
			}
			if(xm_vStream.avail_out != 0){
				xm_vOwner.xOutput(abyTemp, sizeof(abyTemp) - xm_vStream.avail_out);
			}

			xm_bInited = false;
		}
		xm_vOwner.StreamFilterBase::Finalize();
	}
};

// ========== ZlibEncoder ==========
// 静态成员函数。
ZlibEncoder::ZlibEncoder(bool bRaw, unsigned uLevel)
	: xm_pDelegate(std::make_unique<xDelegate>(*this, bRaw, uLevel))
{
}
ZlibEncoder::~ZlibEncoder(){
}

void ZlibEncoder::Abort() noexcept {
	xm_pDelegate->Abort();
}
void ZlibEncoder::Update(const void *pData, std::size_t uSize){
	xm_pDelegate->Update(pData, uSize);
}
void ZlibEncoder::Finalize(){
	xm_pDelegate->Finalize();
}

// ========== ZlibDecoder ==========
// 静态成员函数。
ZlibDecoder::ZlibDecoder(bool bRaw)
	: xm_pDelegate(std::make_unique<xDelegate>(*this, bRaw))
{
}
ZlibDecoder::~ZlibDecoder(){
}

void ZlibDecoder::Abort() noexcept {
	xm_pDelegate->Abort();
}
void ZlibDecoder::Update(const void *pData, std::size_t uSize){
	xm_pDelegate->Update(pData, uSize);
}
void ZlibDecoder::Finalize(){
	xm_pDelegate->Finalize();
}

// ========== ZlibError ==========
ZlibError::ZlibError(const char *pszFile, unsigned long ulLine, const char *pszMessage, long lZlibError) noexcept
	: Exception(pszFile, ulLine, pszMessage, ZlibErrorToWin32Error(lZlibError))
	, xm_lZlibError(lZlibError)
{
}
ZlibError::~ZlibError(){
}
