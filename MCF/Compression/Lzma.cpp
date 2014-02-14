// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "Lzma.hpp"
#include "../Core/UniqueHandle.hpp"
#include "../Core/Exception.hpp"
#include "../Core/Thread.hpp"
#include "../Core/Event.hpp"

#define _7ZIP_ST
#define _LZMA_PROB32
#include "../../External/Lzma/LzmaEnc.h"
#include "../../External/Lzma/LzmaDec.h"

#include <new>
#include <memory>
#include <exception>
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

	class xWorker : NO_COPY {
	private:
		static ::SRes xReadWrapper(void *psisSource, void *pBuffer, size_t *puSize) noexcept {
			const auto pWorker = (xWorker *)((std::uintptr_t)psisSource - OFFSET_OF(xWorker, xm_sisSource));
			return pWorker->xRead(pBuffer, puSize);
		}
		static std::size_t xWriteWrapper(void *psosSink, const void *pBuffer, size_t uSize) noexcept {
			const auto pWorker = (xWorker *)((std::uintptr_t)psosSink - OFFSET_OF(xWorker, xm_sosSink));
			return pWorker->xWrite(pBuffer, uSize);
		}
	private:
		const std::function<std::pair<void *, std::size_t>(std::size_t)> &xm_fnDataCallback;
		::ISeqInStream xm_sisSource;
		::ISeqOutStream xm_sosSink;

		Thread xm_thrdWorker;
		const unsigned char *xm_pbyData;
		std::size_t xm_uSize;
		std::size_t xm_uBytesProcessed;
		Event xm_evnProducer;
		Event xm_evnConsumer;

		std::exception_ptr xm_pException;
	public:
		xWorker(const std::function<std::pair<void *, std::size_t>(std::size_t)> &fnDataCallback)
			: xm_fnDataCallback(fnDataCallback)
			, xm_sisSource{&xReadWrapper}
			, xm_sosSink{&xWriteWrapper}
			, xm_thrdWorker()
			, xm_evnProducer(false)
			, xm_evnConsumer(false)
		{
		}
		~xWorker(){
			Abort();
		}
	private:
		void xThreadProc(::CLzmaEncHandle hEncoder) noexcept {
			const auto ulErrorCode = LzmaErrorToWin32Error(::LzmaEnc_Encode(
				hEncoder,
				&xm_sosSink,
				&xm_sisSource,
				nullptr,
				&g_vAllocSmall,
				&g_vAllocLarge
			));
			if(!xm_pException && (ulErrorCode != ERROR_SUCCESS)){
				xm_pException = MCF_MAKE_EXCEPTION_PTR(ulErrorCode, L"::LzmaEnc_Encode() 失败。");
			}
			xm_evnConsumer.Set();
		}

		::SRes xRead(void *pBuffer, size_t *puSize) noexcept {
			xm_evnProducer.Wait();
			if(xm_uSize == 0){
				*puSize = 0;
			} else {
				const auto uToCopy = std::min(std::min(*puSize, xm_uSize), (std::size_t)0x1000000);
				std::memcpy(pBuffer, xm_pbyData, uToCopy);
				*puSize = uToCopy;

				xm_pbyData += uToCopy;
				xm_uSize -= uToCopy;
				xm_uBytesProcessed += uToCopy;
			}
			if(xm_uSize == 0){
				xm_evnProducer.Reset();
				xm_evnConsumer.Set();
			}
			return SZ_OK;
		}
		std::size_t xWrite(const void *pBuffer, size_t uSize) noexcept {
			try {
				CopyOut(xm_fnDataCallback, pBuffer, uSize);
				return uSize;
			} catch(...){
				xm_pException = std::current_exception();
				return 0;
			}
		}
	public:
		void Abort() noexcept {
			if(xm_thrdWorker.IsAlive()){
				xm_pbyData = nullptr;
				xm_uSize = 0;
				xm_evnConsumer.Reset();
				xm_evnProducer.Set();
				xm_evnConsumer.Wait();
				xm_thrdWorker.JoinDetach();
				xm_pException = std::exception_ptr();
			}
		}
		void Init(::CLzmaEncHandle hEncoder){
			Abort();

			xm_thrdWorker.Start(std::bind(&xWorker::xThreadProc, this, hEncoder));
		}
		void Update(const void *pData, std::size_t uSize){
			ASSERT(xm_thrdWorker.IsAlive());

			xm_uBytesProcessed = 0;
			if(uSize != 0){
				xm_pbyData = (const unsigned char *)pData;
				xm_uSize = uSize;
				xm_evnConsumer.Reset();
				xm_evnProducer.Set();
				xm_evnConsumer.Wait();
				if(xm_pException){
					std::rethrow_exception(xm_pException);
				}
			}
		}
		std::size_t QueryBytesProcessed() const noexcept {
			return xm_uBytesProcessed;
		}
		void Finalize(){
			ASSERT(xm_thrdWorker.IsAlive());

			xm_pbyData = nullptr;
			xm_uSize = 0;
			xm_evnConsumer.Reset();
			xm_evnProducer.Set();
			xm_evnConsumer.Wait();
			if(xm_pException){
				std::rethrow_exception(xm_pException);
			}
		}
	};
private:
	const std::function<std::pair<void *, std::size_t>(std::size_t)> xm_fnDataCallback;
	::CLzmaEncProps xm_vEncProps;
	xWorker xm_vWorker;
	bool xm_bInited;

	UniqueHandle<::CLzmaEncHandle, xLzmaEncHandleCloser> xm_hEncoder;
public:
	xDelegate(std::function<std::pair<void *, std::size_t>(std::size_t)> &&fnDataCallback, int nLevel, std::uint32_t u32DictSize)
		: xm_fnDataCallback(std::move(fnDataCallback))
		, xm_vWorker(xm_fnDataCallback)
		, xm_bInited(false)
	{
		::LzmaEncProps_Init(&xm_vEncProps);
		xm_vEncProps.level = nLevel;
		xm_vEncProps.dictSize = u32DictSize;
		xm_vEncProps.writeEndMark = 1;
	}
public:
	void Abort() noexcept {
		xm_vWorker.Abort();

		xm_bInited = false;
	}
	void Update(const void *pData, std::size_t uSize){
		unsigned long ulErrorCode;

		if(!xm_bInited){
			xm_hEncoder.Reset(::LzmaEnc_Create(&g_vAllocSmall));
			if(!xm_hEncoder){
				MCF_THROW(ERROR_NOT_ENOUGH_MEMORY, L"::LzmaEnc_Create() 失败。");
			}

			ulErrorCode = LzmaErrorToWin32Error(::LzmaEnc_SetProps(xm_hEncoder, &xm_vEncProps));
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

			xm_vWorker.Init(xm_hEncoder);

			xm_bInited = true;
		}

		xm_vWorker.Update(pData, uSize);
	}
	std::size_t QueryBytesProcessed() const noexcept {
		return xm_vWorker.QueryBytesProcessed();
	}
	void Finalize(){
		if(xm_bInited){
			xm_vWorker.Finalize();

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
std::size_t LzmaEncoder::QueryBytesProcessed() const noexcept {
	return xm_pDelegate->QueryBytesProcessed();
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
			::LzmaDec_Free(pDecoder, &g_vAllocSmall);
		}
	};
private:
	const std::function<std::pair<void *, std::size_t>(std::size_t)> xm_fnDataCallback;
	::CLzmaDec xm_vDecoder;
	bool xm_bInited;

	UniqueHandle<::CLzmaDec *, xLzmaDecHandleCloser> xm_pDecoder;
	unsigned char xm_abyTemp[0x10000];
	std::size_t xm_uBytesProcessed;
public:
	xDelegate(std::function<std::pair<void *, std::size_t>(std::size_t)> &&fnDataCallback)
		: xm_fnDataCallback(std::move(fnDataCallback))
		, xm_bInited(false)
	{
		LzmaDec_Construct(&xm_vDecoder); // 这是宏？！
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

			ulErrorCode = ::LzmaDec_Allocate(&xm_vDecoder, abyHeader, sizeof(abyHeader), &g_vAllocSmall);
			if(ulErrorCode != ERROR_SUCCESS){
				MCF_THROW(ulErrorCode, L"::LzmaDec_Allocate() 失败。");
			}
			xm_pDecoder.Reset(&xm_vDecoder);

			::LzmaDec_Init(xm_pDecoder);

			xm_bInited = true;
		}

		xm_uBytesProcessed = 0;
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
			pbyRead += uToDecode;
			xm_uBytesProcessed += uToDecode;

			if(vStatus == LZMA_STATUS_FINISHED_WITH_MARK){
				::LzmaDec_Init(xm_pDecoder);
			}
		}
	}
	std::size_t QueryBytesProcessed() const noexcept {
		return xm_uBytesProcessed;
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
std::size_t LzmaDecoder::QueryBytesProcessed() const noexcept {
	return xm_pDelegate->QueryBytesProcessed();
}
void LzmaDecoder::Finalize(){
	xm_pDelegate->Finalize();
}
