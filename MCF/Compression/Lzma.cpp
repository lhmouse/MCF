// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "Lzma.hpp"
#include "../Core/UniqueHandle.hpp"
#include "../Core/Exception.hpp"

#define _7ZIP_ST
#define _LZMA_PROB32
#include "../../External/Lzma/NewLzmaEnc.h"
#include "../../External/Lzma/LzmaDec.h"

#include <new>
#include <memory>
#include <exception>
using namespace MCF;

namespace {
	inline void CopyOut(const std::function<std::pair<void *, std::size_t> (std::size_t)> &fnDataCallback, const void *pSrc, std::size_t uBytesToCopy){
		std::size_t uBytesCopied = 0;
		while(uBytesCopied < uBytesToCopy){
			const std::size_t uBytesRemaining = uBytesToCopy - uBytesCopied;
			const auto vResult = fnDataCallback(uBytesRemaining);
			const std::size_t uBytesToCopyThisTime = std::min(vResult.second, uBytesRemaining);
			__builtin_memcpy(vResult.first, (const unsigned char *)pSrc + uBytesCopied, uBytesToCopyThisTime);
			uBytesCopied += uBytesToCopyThisTime;
		}
	}

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
		case SZ_ERROR_OUTPUT_EOF:
			return ERROR_MORE_DATA;
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

	::ISzAlloc g_vAllocSmall = {&SmallAlloc, &SmallFree};
	::ISzAlloc g_vAllocLarge = {&LargeAlloc, &LargeFree};
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
			::LzmaEnc_Destroy(hLzma, &g_vAllocSmall, &g_vAllocLarge);
		}
	};

	struct xNewLzmaEncodeContextDeleter {
		constexpr void *operator()() const noexcept {
			return nullptr;
		}
		void operator()(void *pContext) const noexcept {
			::LzmaEnc_NewEncodeDestroyContext(pContext);
		}
	};

private:
	static std::size_t xWriteCallback(void *pDataSink, const void *pData, std::size_t uSize) noexcept {
		const auto pDelegate = (xDelegate *)((unsigned char *)pDataSink - OFFSET_OF(xDelegate, xm_sosOutputter));
		return pDelegate->xWrite(pData, uSize);
	}

private:
	const std::function<std::pair<void *, std::size_t> (std::size_t)> xm_fnDataCallback;
	::ISeqOutStream xm_sosOutputter;
	::CLzmaEncProps xm_vEncProps;

	UniqueHandle<xLzmaEncHandleCloser> xm_hEncoder;
	UniqueHandle<xNewLzmaEncodeContextDeleter> xm_pContext;
	std::size_t xm_uBytesProcessed;

	std::exception_ptr xm_pException;

public:
	xDelegate(std::function<std::pair<void *, std::size_t> (std::size_t)> &&fnDataCallback, int nLevel, std::uint32_t u32DictSize)
		: xm_fnDataCallback(std::move(fnDataCallback))
	{
		xm_sosOutputter.Write = &xWriteCallback;

		::LzmaEncProps_Init(&xm_vEncProps);
		xm_vEncProps.level = nLevel;
		xm_vEncProps.dictSize = u32DictSize;
		xm_vEncProps.writeEndMark = 1;
	}

private:
	std::size_t xWrite(const void *pData, std::size_t uSize) noexcept {
		std::size_t uBytesWritten = 0;
		try {
			CopyOut(xm_fnDataCallback, pData, uSize);
			uBytesWritten = uSize;
		} catch(...){
			xm_pException = std::current_exception();
		}
		return uBytesWritten;
	}

public:
	void Abort() noexcept {
		xm_pContext.Reset();
	}
	void Update(const void *pData, std::size_t uSize){
		unsigned long ulErrorCode;

		if(!xm_pContext){
			xm_hEncoder.Reset(::LzmaEnc_Create(&g_vAllocSmall));
			if(!xm_hEncoder){
				MCF_THROW(ERROR_NOT_ENOUGH_MEMORY, L"::LzmaEnc_Create() 失败。");
			}

			ulErrorCode = LzmaErrorToWin32Error(::LzmaEnc_SetProps(xm_hEncoder.Get(), &xm_vEncProps));
			if(ulErrorCode != ERROR_SUCCESS){
				MCF_THROW(ulErrorCode, L"::LzmaEnc_SetProps() 失败。");
			}

			unsigned char abyHeader[LZMA_PROPS_SIZE];
			std::size_t uHeaderSize = sizeof(abyHeader);
			ulErrorCode = LzmaErrorToWin32Error(::LzmaEnc_WriteProperties(xm_hEncoder.Get(), abyHeader, &uHeaderSize));
			if(ulErrorCode != ERROR_SUCCESS){
				MCF_THROW(ulErrorCode, L"::LzmaEnc_WriteProperties() 失败。");
			}
			CopyOut(xm_fnDataCallback, abyHeader, uHeaderSize);

			xm_pContext.Reset(::LzmaEnc_NewEncodeCreateContext(xm_hEncoder.Get(), &xm_sosOutputter, nullptr, &g_vAllocSmall, &g_vAllocLarge));
			if(!xm_pContext){
				MCF_THROW(ERROR_NOT_ENOUGH_MEMORY, L"::LzmaEnc_NewEncodeCreateContext() 失败。");
			}

			xm_pException = std::exception_ptr();
		}

		auto pbyRead = (const unsigned char *)pData;
		const auto pbyEnd = pbyRead + uSize;

		xm_uBytesProcessed = 0;
		while(pbyRead != pbyEnd){
			const auto uBytesToProcess = std::min<std::size_t>(pbyEnd - pbyRead, 0x10000);

			ulErrorCode = LzmaErrorToWin32Error(::LzmaEnc_NewEncode(xm_pContext.Get(), pbyRead, uBytesToProcess));
			if(xm_pException){
				std::rethrow_exception(xm_pException);
			}
			if(ulErrorCode != ERROR_SUCCESS){
				MCF_THROW(ulErrorCode, L"::LzmaEnc_NewEncode() 失败。");
			}

			pbyRead += uBytesToProcess;
			xm_uBytesProcessed += uBytesToProcess;
		}
	}
	std::size_t QueryBytesProcessed() const noexcept {
		return xm_uBytesProcessed;
	}
	void Finalize(){
		if(xm_pContext){
			const auto ulErrorCode = LzmaErrorToWin32Error(::LzmaEnc_NewEncode(xm_pContext.Get(), nullptr, 0));
			if(xm_pException){
				std::rethrow_exception(xm_pException);
			}
			if(ulErrorCode != ERROR_SUCCESS){
				MCF_THROW(ulErrorCode, L"::LzmaEnc_NewEncode() 失败。");
			}

			xm_pContext.Reset();
		}
	}
};

// 构造函数和析构函数。
LzmaEncoder::LzmaEncoder(std::function<std::pair<void *, std::size_t> (std::size_t)> fnDataCallback, int nLevel, std::uint32_t u32DictSize)
	: xm_pDelegate(new xDelegate(std::move(fnDataCallback), nLevel, u32DictSize))
{
}
LzmaEncoder::LzmaEncoder(LzmaEncoder &&rhs) noexcept
	: xm_pDelegate(std::move(rhs.xm_pDelegate))
{
}
LzmaEncoder &LzmaEncoder::operator=(LzmaEncoder &&rhs) noexcept {
	if(&rhs != this){
		xm_pDelegate = std::move(rhs.xm_pDelegate);
	}
	return *this;
}
LzmaEncoder::~LzmaEncoder(){
}

// 其他非静态成员函数。
void LzmaEncoder::Abort() noexcept {
	ASSERT(xm_pDelegate);

	xm_pDelegate->Abort();
}
void LzmaEncoder::Update(const void *pData, std::size_t uSize){
	ASSERT(xm_pDelegate);

	xm_pDelegate->Update(pData, uSize);
}
std::size_t LzmaEncoder::QueryBytesProcessed() const noexcept {
	ASSERT(xm_pDelegate);

	return xm_pDelegate->QueryBytesProcessed();
}
void LzmaEncoder::Finalize(){
	ASSERT(xm_pDelegate);

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
			::LzmaDec_Free(pDecoder, &g_vAllocSmall);
		}
	};

private:
	const std::function<std::pair<void *, std::size_t> (std::size_t)> xm_fnDataCallback;
	::CLzmaDec xm_vDecoder;

	unsigned char xm_abyHeader[LZMA_PROPS_SIZE];
	std::size_t xm_uHeaderSize;
	UniqueHandle<xLzmaDecHandleCloser> xm_pDecoder;
	unsigned char xm_abyTemp[0x10000];
	std::size_t xm_uBytesProcessed;

public:
	xDelegate(std::function<std::pair<void *, std::size_t> (std::size_t)> &&fnDataCallback)
		: xm_fnDataCallback(std::move(fnDataCallback))
		, xm_uHeaderSize(0)
	{
		LzmaDec_Construct(&xm_vDecoder); // 这是宏？！
	}

public:
	void Abort() noexcept {
		xm_uHeaderSize = 0;
	}
	void Update(const void *pData, std::size_t uSize){
		auto pbyRead = (const unsigned char *)pData;
		const auto pbyEnd = pbyRead + uSize;

		unsigned long ulErrorCode;

		xm_uBytesProcessed = 0;
		if(xm_uHeaderSize < sizeof(xm_abyHeader)){
			const auto uBytesToCopy = std::min<std::size_t>(pbyEnd - pbyRead, sizeof(xm_abyHeader) - xm_uHeaderSize);
			__builtin_memcpy(xm_abyHeader + xm_uHeaderSize, pbyRead, uBytesToCopy);
			pbyRead += uBytesToCopy;
			xm_uBytesProcessed += uBytesToCopy;

			xm_uHeaderSize += uBytesToCopy;
			if(xm_uHeaderSize == sizeof(xm_abyHeader)){
				xm_pDecoder.Reset();

				ulErrorCode = ::LzmaDec_Allocate(&xm_vDecoder, xm_abyHeader, sizeof(xm_abyHeader), &g_vAllocSmall);
				if(ulErrorCode != ERROR_SUCCESS){
					MCF_THROW(ulErrorCode, L"::LzmaDec_Allocate() 失败。");
				}
				::LzmaDec_Init(&xm_vDecoder);
				xm_pDecoder.Reset(&xm_vDecoder);
			}
		}
		if(xm_uHeaderSize == sizeof(xm_abyHeader)){
			while(pbyRead != pbyEnd){
				::ELzmaStatus vStatus;
				std::size_t uDecoded = sizeof(xm_abyTemp);
				std::size_t uToDecode = (std::size_t)(pbyEnd - pbyRead);

				ulErrorCode = LzmaErrorToWin32Error(::LzmaDec_DecodeToBuf(xm_pDecoder.Get(), xm_abyTemp, &uDecoded, pbyRead, &uToDecode, LZMA_FINISH_ANY, &vStatus));
				if(ulErrorCode != ERROR_SUCCESS){
					MCF_THROW(ulErrorCode, L"::LzmaDec_DecodeToBuf() 失败。");
				}

				CopyOut(xm_fnDataCallback, xm_abyTemp, uDecoded);
				pbyRead += uToDecode;
				xm_uBytesProcessed += uToDecode;

				if(vStatus == LZMA_STATUS_FINISHED_WITH_MARK){
					::LzmaDec_Init(xm_pDecoder.Get());
				}
			}
		}
	}
	std::size_t QueryBytesProcessed() const noexcept {
		return xm_uBytesProcessed;
	}
	void Finalize(){
		xm_uHeaderSize = 0;
	}
};

// 构造函数和析构函数。
LzmaDecoder::LzmaDecoder(std::function<std::pair<void *, std::size_t> (std::size_t)> fnDataCallback)
	: xm_pDelegate(new xDelegate(std::move(fnDataCallback)))
{
}
LzmaDecoder::LzmaDecoder(LzmaDecoder &&rhs) noexcept
	: xm_pDelegate(std::move(rhs.xm_pDelegate))
{
}
LzmaDecoder &LzmaDecoder::operator=(LzmaDecoder &&rhs) noexcept {
	if(&rhs != this){
		xm_pDelegate = std::move(rhs.xm_pDelegate);
	}
	return *this;
}
LzmaDecoder::~LzmaDecoder(){
}

// 其他非静态成员函数。
void LzmaDecoder::Abort() noexcept {
	ASSERT(xm_pDelegate);

	xm_pDelegate->Abort();
}
void LzmaDecoder::Update(const void *pData, std::size_t uSize){
	ASSERT(xm_pDelegate);

	xm_pDelegate->Update(pData, uSize);
}
std::size_t LzmaDecoder::QueryBytesProcessed() const noexcept {
	ASSERT(xm_pDelegate);

	return xm_pDelegate->QueryBytesProcessed();
}
void LzmaDecoder::Finalize(){
	ASSERT(xm_pDelegate);

	xm_pDelegate->Finalize();
}
