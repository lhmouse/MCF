// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "Lzma.hpp"
#include "../Core/UniqueHandle.hpp"
#include "../Core/Exception.hpp"

#define _7ZIP_ST
#define _LZMA_PROB32
#include "../../External/Lzma/LzmaEnc.h"
#include "../../External/Lzma/LzmaDec.h"

#include <iterator>
#include <new>
#include <memory>
using namespace MCF;

namespace {
	inline void CopyOut(const std::function<std::pair<void *, std::size_t>(std::size_t)> &fnDataCallback, const void *pSrc, std::size_t uBytesToCopy){
		std::size_t uBytesCopied = 0;
		while(uBytesCopied < uBytesToCopy){
			const std::size_t uBytesRemaining = uBytesToCopy - uBytesCopied;
			const auto vResult = fnDataCallback(uBytesRemaining);
			const std::size_t uBytesToCopyThisTime = std::min(vResult.second, uBytesRemaining);
			__builtin_memcpy(vResult.first, (const unsigned char *)pSrc + uBytesCopied, uBytesToCopyThisTime);
			uBytesCopied += uBytesToCopyThisTime;
		}
	};

	inline unsigned long LzmaErrorToWin32Error(int nError) noexcept {
		switch(nError){
		case SZ_OK:
			return ERROR_SUCCESS;
		case SZ_ERROR_DATA:
			return ERROR_INVALID_DATA;
		case SZ_ERROR_MEM:
			return ERROR_NOT_ENOUGH_MEMORY;
		case SZ_ERROR_CRC:
			return ERROR_INVALID_DATA;
		case SZ_ERROR_UNSUPPORTED:
			return ERROR_NOT_SUPPORTED;
		case SZ_ERROR_PARAM:
			return ERROR_INVALID_PARAMETER;
		case SZ_ERROR_INPUT_EOF:
			return ERROR_HANDLE_EOF;
		// case SZ_ERROR_OUTPUT_EOF:
		// case SZ_ERROR_READ:
		// case SZ_ERROR_WRITE:
		// case SZ_ERROR_PROGRESS:
		// case SZ_ERROR_FAIL:
		// case SZ_ERROR_THREAD:
		default:
			return ERROR_INVALID_FUNCTION;
		}
	}

	void *SmallAlloc(void *, size_t uSize) noexcept {
		return ::operator new(uSize);
	}
	void SmallFree(void *, void *pBlock) noexcept {
		::operator delete(pBlock);
	}

	void *LargeAlloc(void *, size_t uSize) noexcept {
		return ::VirtualAlloc(nullptr, uSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	}
	void LargeFree(void *, void *pBlock) noexcept {
		if(pBlock != nullptr){
			::VirtualFree(pBlock, 0, MEM_RELEASE);
		}
	}

	::ISzAlloc g_AllocSmall = {&SmallAlloc, &SmallFree};
	::ISzAlloc g_AllocLarge = {&LargeAlloc, &LargeFree};
}

// ========== LzmaEncoder ==========
// 嵌套类定义。
class LzmaEncoder::xDelegate {
private:
	struct xLzmaEncHandleCloser {
		constexpr ::CLzmaEncHandle operator()() const noexcept {
			return nullptr;
		}
		void operator()(::CLzmaEncHandle hLzma) const noexcept {
			::LzmaEnc_Destroy(hLzma, &g_AllocSmall, &g_AllocLarge);
		}
	};

	class xDataSource : public ::ISeqInStream {
	private:
		static ::SRes xReadWrapper(void *pThis, void *pBuffer, size_t *puSize) noexcept {
			return ((xDataSource *)pThis)->xRead(pBuffer, puSize);
		}
	private:
		const unsigned char *xm_pbyRead;
		std::size_t xm_uRemaining;
	public:
		xDataSource(const void *pData, std::size_t uSize) noexcept
			: ISeqInStream{&xReadWrapper}
			, xm_pbyRead((const unsigned char *)pData)
			, xm_uRemaining(uSize)
		{
		}
	private:
		::SRes xRead(void *pBuffer, size_t *puSize) noexcept {
			const auto uBytesToCopy = std::min(*puSize, xm_uRemaining);
			std::memcpy(pBuffer, xm_pbyRead, uBytesToCopy);
			xm_pbyRead += uBytesToCopy;
			xm_uRemaining -= uBytesToCopy;
			*puSize = uBytesToCopy;
			return SZ_OK;
		}
	};

	class xDataSink : public ::ISeqOutStream {
	private:
		static std::size_t xWriteWrapper(void *pThis, const void *pBuffer, size_t uSize) noexcept {
			return ((xDataSink *)pThis)->xWrite(pBuffer, uSize);
		}
	private:
		const std::function<std::pair<void *, std::size_t>(std::size_t)> &xm_fnDataCallback;
	public:
		xDataSink(const std::function<std::pair<void *, std::size_t>(std::size_t)> &fnDataCallback) noexcept
			: ISeqOutStream{&xWriteWrapper}
			, xm_fnDataCallback(fnDataCallback)
		{
		}
	private:
		std::size_t xWrite(const void *pBuffer, size_t uSize) noexcept {
			try {
				CopyOut(xm_fnDataCallback, pBuffer, uSize);
				return uSize;
			} catch(...){
				return 0;
			}
		}
	};
private:
	const std::function<std::pair<void *, std::size_t>(std::size_t)> xm_fnDataCallback;
	::CLzmaEncProps xm_EncProps;
	bool xm_bInited;

	UniqueHandle<::CLzmaEncHandle, xLzmaEncHandleCloser> xm_hEncoder;
public:
	xDelegate(std::function<std::pair<void *, std::size_t>(std::size_t)> &&fnDataCallback, int nLevel, std::uint32_t u32DictSize)
		: xm_fnDataCallback(std::move(fnDataCallback))
		, xm_bInited(false)
	{
		::LzmaEncProps_Init(&xm_EncProps);
		xm_EncProps.level = nLevel;
		xm_EncProps.dictSize = u32DictSize;
		xm_EncProps.writeEndMark = 1;
	}
public:
	void Abort() noexcept {
		xm_bInited = false;
	}
	void Update(const void *pData, std::size_t uSize){
		unsigned long ulErrorCode;

		if(!xm_bInited){
			xm_hEncoder.Reset(::LzmaEnc_Create(&g_AllocSmall));
			if(!xm_hEncoder){
				MCF_THROW(ERROR_NOT_ENOUGH_MEMORY, L"::LzmaEnc_Create() 失败。");
			}

			ulErrorCode = LzmaErrorToWin32Error(::LzmaEnc_SetProps(xm_hEncoder, &xm_EncProps));
			if(ulErrorCode != ERROR_SUCCESS){
				MCF_THROW(ulErrorCode, L"::LzmaEnc_SetProps() 失败。");
			}

			unsigned char abyHeader[LZMA_PROPS_SIZE];
			std::size_t uHeaderSize = sizeof(abyHeader);
			ulErrorCode = LzmaErrorToWin32Error(::LzmaEnc_WriteProperties(xm_hEncoder, abyHeader, &uHeaderSize));
			if(ulErrorCode != ERROR_SUCCESS){
				MCF_THROW(ulErrorCode, L"::LzmaEnc_WriteProperties() 失败。");
			}
			CopyOut(xm_fnDataCallback, abyHeader, uHeaderSize);

			xm_bInited = true;
		}

		if(uSize != 0){
			xDataSink vSink(xm_fnDataCallback);
			xDataSource vSource(pData, uSize);
			ulErrorCode = LzmaErrorToWin32Error(::LzmaEnc_Encode(xm_hEncoder, &vSink, &vSource, nullptr, &g_AllocSmall, &g_AllocLarge));
			if(ulErrorCode != ERROR_SUCCESS){
				MCF_THROW(ulErrorCode, L"::LzmaEnc_Encode() 失败。");
			}
		}
	}
	void Finalize(){
		if(xm_bInited){
			xDataSink vSink(xm_fnDataCallback);
			xDataSource vSource(nullptr, 0);
			const auto ulErrorCode = LzmaErrorToWin32Error(::LzmaEnc_Encode(xm_hEncoder, &vSink, &vSource, nullptr, &g_AllocSmall, &g_AllocLarge));
			if(ulErrorCode != ERROR_SUCCESS){
				MCF_THROW(ulErrorCode, L"::LzmaEnc_Encode() 失败。");
			}

			xm_bInited = false;
		}
	}
};

// 构造函数和析构函数。
LzmaEncoder::LzmaEncoder(std::function<std::pair<void *, std::size_t>(std::size_t)> fnDataCallback, int nLevel, std::uint32_t u32DictSize)
	: xm_pDelegate(new xDelegate(std::move(fnDataCallback), nLevel, u32DictSize))
{
}
LzmaEncoder::~LzmaEncoder(){
}

// 其他非静态成员函数。
void LzmaEncoder::Update(const void *pData, std::size_t uSize){
	xm_pDelegate->Update(pData, uSize);
}
void LzmaEncoder::Finalize(){
	xm_pDelegate->Finalize();
}

// ========== LzmaDecoder ==========
// 嵌套类定义。
class LzmaDecoder::xDelegate {
private:
	struct xLzmaDecHandleCloser {
		constexpr ::CLzmaDec *operator()() const noexcept {
			return nullptr;
		}
		void operator()(::CLzmaDec *pDecoder) const noexcept {
			::LzmaDec_Free(pDecoder, &g_AllocSmall);
		}
	};
private:
	const std::function<std::pair<void *, std::size_t>(std::size_t)> xm_fnDataCallback;
	::CLzmaDec xm_Decoder;
	bool xm_bInited;

	UniqueHandle<::CLzmaDec *, xLzmaDecHandleCloser> xm_pDecoder;
	unsigned char xm_abyTemp[0x10000];
public:
	xDelegate(std::function<std::pair<void *, std::size_t>(std::size_t)> &&fnDataCallback)
		: xm_fnDataCallback(std::move(fnDataCallback))
		, xm_bInited(false)
	{
		LzmaDec_Construct(&xm_Decoder); // 这是宏？！
	}
public:
	void Abort() noexcept {
		xm_bInited = false;
	}
	void Update(const void *pData, std::size_t uSize){
		auto pbyRead = (const unsigned char *)pData;
		const auto pbyEnd = pbyRead + uSize;

		unsigned long ulErrorCode;

		if(!xm_bInited){
			unsigned char abyHeader[LZMA_PROPS_SIZE];
			if((std::size_t)(pbyEnd - pbyRead) < sizeof(abyHeader)){
				MCF_THROW(ERROR_INVALID_DATA, L"::LZMA 头丢失。");
			}
			__builtin_memcpy(abyHeader, pbyRead, sizeof(abyHeader));
			pbyRead += sizeof(abyHeader);

			ulErrorCode = ::LzmaDec_Allocate(&xm_Decoder, abyHeader, sizeof(abyHeader), &g_AllocSmall);
			if(ulErrorCode != ERROR_SUCCESS){
				MCF_THROW(ulErrorCode, L"::LzmaDec_Allocate() 失败。");
			}
			xm_pDecoder.Reset(&xm_Decoder);

			::LzmaDec_Init(xm_pDecoder);

			xm_bInited = true;
		}

		while(pbyRead != pbyEnd){
			::ELzmaStatus vStatus;
			std::size_t uDecoded = sizeof(xm_abyTemp);
			std::size_t uToDecode = (std::size_t)(pbyEnd - pbyRead);
			ulErrorCode = LzmaErrorToWin32Error(::LzmaDec_DecodeToBuf(
				xm_pDecoder,
				xm_abyTemp,
				&uDecoded,
				pbyRead,
				&uToDecode,
				LZMA_FINISH_ANY,
				&vStatus
			));
			if(ulErrorCode != ERROR_SUCCESS){
				MCF_THROW(ulErrorCode, L"::LzmaDec_DecodeToBuf() 失败。");
			}
			CopyOut(xm_fnDataCallback, xm_abyTemp, uDecoded);

			if(vStatus == LZMA_STATUS_FINISHED_WITH_MARK){
				break;
			}
			pbyRead += uToDecode;
		}
	}
	void Finalize(){
		if(xm_bInited){
			xm_bInited = false;
		}
	}
};

// 构造函数和析构函数。
LzmaDecoder::LzmaDecoder(std::function<std::pair<void *, std::size_t>(std::size_t)> fnDataCallback)
	:  xm_pDelegate(new xDelegate(std::move(fnDataCallback)))
{
}
LzmaDecoder::~LzmaDecoder(){
}

// 其他非静态成员函数。
void LzmaDecoder::Update(const void *pData, std::size_t uSize){
	xm_pDelegate->Update(pData, uSize);
}
void LzmaDecoder::Finalize(){
	xm_pDelegate->Finalize();
}
