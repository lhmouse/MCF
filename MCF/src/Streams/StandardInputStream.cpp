// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "StandardInputStream.hpp"
#include <MCFCRT/env/standard_streams.h>
#include "../Core/Exception.hpp"

namespace MCF {

StandardInputStream::~StandardInputStream(){
}

int StandardInputStream::Peek(){
	return ::_MCFCRT_PeekStandardInputByte();
}
int StandardInputStream::Get(){
	return ::_MCFCRT_ReadStandardInputByte();
}
bool StandardInputStream::Discard(){
	return ::_MCFCRT_ReadStandardInputByte() >= 0;
}
std::size_t StandardInputStream::Peek(void *pData, std::size_t uSize){
	const auto uRead = ::_MCFCRT_PeekStandardInputBinary(pData, uSize);
	if(uRead == 0){
		const auto dwErrorCode = ::GetLastError();
		if((dwErrorCode != ERROR_SUCCESS) && (dwErrorCode != ERROR_HANDLE_EOF)){
			MCF_THROW(Exception, dwErrorCode, Rcntws::View(L"StandardInputStream: _MCFCRT_PeekStandardInputBinary() 失败。"));
		}
	}
	return uRead;
}
std::size_t StandardInputStream::Get(void *pData, std::size_t uSize){
	const auto uRead = ::_MCFCRT_ReadStandardInputBinary(pData, uSize);
	if(uRead == 0){
		const auto dwErrorCode = ::GetLastError();
		if((dwErrorCode != ERROR_SUCCESS) && (dwErrorCode != ERROR_HANDLE_EOF)){
			MCF_THROW(Exception, dwErrorCode, Rcntws::View(L"StandardInputStream: _MCFCRT_ReadStandardInputBinary() 失败。"));
		}
	}
	return uRead;
}
std::size_t StandardInputStream::Discard(std::size_t uSize){
	const auto uRead = ::_MCFCRT_DiscardStandardInputBinary(uSize);
	if(uRead == 0){
		const auto dwErrorCode = ::GetLastError();
		if((dwErrorCode != ERROR_SUCCESS) && (dwErrorCode != ERROR_HANDLE_EOF)){
			MCF_THROW(Exception, dwErrorCode, Rcntws::View(L"StandardInputStream: _MCFCRT_DiscardStandardInputBinary() 失败。"));
		}
	}
	return uRead;
}

long StandardInputStream::PeekChar32(){
	return ::_MCFCRT_PeekStandardInputChar32();
}
long StandardInputStream::GetChar32(){
	return ::_MCFCRT_ReadStandardInputChar32();
}
bool StandardInputStream::DiscardChar32(){
	return ::_MCFCRT_ReadStandardInputChar32() >= 0;
}
std::size_t StandardInputStream::PeekText(wchar_t *pwcData, std::size_t uSize, bool bSingleLine){
	const auto uRead = ::_MCFCRT_PeekStandardInputText(pwcData, uSize, bSingleLine);
	if(uRead == 0){
		const auto dwErrorCode = ::GetLastError();
		if((dwErrorCode != ERROR_SUCCESS) && (dwErrorCode != ERROR_HANDLE_EOF)){
			MCF_THROW(Exception, dwErrorCode, Rcntws::View(L"StandardInputStream: _MCFCRT_PeekStandardInputText() 失败。"));
		}
	}
	return uRead;
}
std::size_t StandardInputStream::GetText(wchar_t *pwcData, std::size_t uSize, bool bSingleLine){
	const auto uRead = ::_MCFCRT_ReadStandardInputText(pwcData, uSize, bSingleLine);
	if(uRead == 0){
		const auto dwErrorCode = ::GetLastError();
		if((dwErrorCode != ERROR_SUCCESS) && (dwErrorCode != ERROR_HANDLE_EOF)){
			MCF_THROW(Exception, dwErrorCode, Rcntws::View(L"StandardInputStream: _MCFCRT_ReadStandardInputText() 失败。"));
		}
	}
	return uRead;
}
std::size_t StandardInputStream::DiscardText(std::size_t uSize, bool bSingleLine){
	const auto uRead = ::_MCFCRT_DiscardStandardInputText(uSize, bSingleLine);
	if(uRead == 0){
		const auto dwErrorCode = ::GetLastError();
		if((dwErrorCode != ERROR_SUCCESS) && (dwErrorCode != ERROR_HANDLE_EOF)){
			MCF_THROW(Exception, dwErrorCode, Rcntws::View(L"StandardInputStream: _MCFCRT_DiscardStandardInputText() 失败。"));
		}
	}
	return uRead;
}

bool StandardInputStream::IsEchoing() const noexcept {
	return ::_MCFCRT_IsStandardInputEchoing();
}
bool StandardInputStream::SetEchoing(bool bEchoing) noexcept {
	return ::_MCFCRT_SetStandardInputEchoing(bEchoing);
}

}
