// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "StandardInputStream.hpp"
#include "../../MCFCRT/env/standard_streams.h"
#include "../Core/Exception.hpp"

namespace MCF {

StandardInputStream::~StandardInputStream(){
}

int StandardInputStream::Peek(){
	unsigned char byData;
	if(Peek(&byData, 1) < 1){
		return -1;
	}
	return byData;
}
int StandardInputStream::Get(){
	unsigned char byData;
	if(Get(&byData, 1) < 1){
		return -1;
	}
	return byData;
}
bool StandardInputStream::Discard(){
	if(Discard(1) < 1){
		return false;
	}
	return true;
}
std::size_t StandardInputStream::Peek(void *pData, std::size_t uSize){
	const auto uRead = ::_MCFCRT_PeekStandardInputAsBinary(pData, uSize);
	if(uRead == 0){
		const auto dwErrorCode = ::GetLastError();
		if((dwErrorCode != ERROR_SUCCESS) && (dwErrorCode != ERROR_HANDLE_EOF)){
			MCF_THROW(Exception, dwErrorCode, Rcntws::View(L"StandardInputStream: _MCFCRT_PeekStandardInputAsBinary() 失败。"));
		}
	}
	return uRead;
}
std::size_t StandardInputStream::Get(void *pData, std::size_t uSize){
	const auto uRead = ::_MCFCRT_ReadStandardInputAsBinary(pData, uSize);
	if(uRead == 0){
		const auto dwErrorCode = ::GetLastError();
		if((dwErrorCode != ERROR_SUCCESS) && (dwErrorCode != ERROR_HANDLE_EOF)){
			MCF_THROW(Exception, dwErrorCode, Rcntws::View(L"StandardInputStream: _MCFCRT_ReadStandardInputAsBinary() 失败。"));
		}
	}
	return uRead;
}
std::size_t StandardInputStream::Discard(std::size_t uSize){
	const auto uRead = ::_MCFCRT_DiscardStandardInputAsBinary(uSize);
	if(uRead == 0){
		const auto dwErrorCode = ::GetLastError();
		if((dwErrorCode != ERROR_SUCCESS) && (dwErrorCode != ERROR_HANDLE_EOF)){
			MCF_THROW(Exception, dwErrorCode, Rcntws::View(L"StandardInputStream: _MCFCRT_DiscardStandardInputAsBinary() 失败。"));
		}
	}
	return uRead;
}

long StandardInputStream::PeekText(){
	wchar_t wcData;
	if(PeekText(&wcData, 1, false) < 1){
		return -1;
	}
	return wcData;
}
long StandardInputStream::GetText(){
	wchar_t wcData;
	if(GetText(&wcData, 1, false) < 1){
		return -1;
	}
	return wcData;
}
bool StandardInputStream::DiscardText(){
	if(DiscardText(1, false) < 1){
		return false;
	}
	return true;
}
std::size_t StandardInputStream::PeekText(wchar_t *pwcData, std::size_t uSize, bool bSingleLine){
	const auto uRead = ::_MCFCRT_PeekStandardInputAsText(pwcData, uSize, bSingleLine);
	if(uRead == 0){
		const auto dwErrorCode = ::GetLastError();
		if((dwErrorCode != ERROR_SUCCESS) && (dwErrorCode != ERROR_HANDLE_EOF)){
			MCF_THROW(Exception, dwErrorCode, Rcntws::View(L"StandardInputStream: _MCFCRT_PeekStandardInputAsText() 失败。"));
		}
	}
	return uRead;
}
std::size_t StandardInputStream::GetText(wchar_t *pwcData, std::size_t uSize, bool bSingleLine){
	const auto uRead = ::_MCFCRT_ReadStandardInputAsText(pwcData, uSize, bSingleLine);
	if(uRead == 0){
		const auto dwErrorCode = ::GetLastError();
		if((dwErrorCode != ERROR_SUCCESS) && (dwErrorCode != ERROR_HANDLE_EOF)){
			MCF_THROW(Exception, dwErrorCode, Rcntws::View(L"StandardInputStream: _MCFCRT_ReadStandardInputAsText() 失败。"));
		}
	}
	return uRead;
}
std::size_t StandardInputStream::DiscardText(std::size_t uSize, bool bSingleLine){
	const auto uRead = ::_MCFCRT_DiscardStandardInputAsText(uSize, bSingleLine);
	if(uRead == 0){
		const auto dwErrorCode = ::GetLastError();
		if((dwErrorCode != ERROR_SUCCESS) && (dwErrorCode != ERROR_HANDLE_EOF)){
			MCF_THROW(Exception, dwErrorCode, Rcntws::View(L"StandardInputStream: _MCFCRT_DiscardStandardInputAsText() 失败。"));
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
