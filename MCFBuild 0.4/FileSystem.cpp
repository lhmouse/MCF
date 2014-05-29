// MCF Build
// Copyleft 2014, LH_Mouse. All wrongs reserved.

#include "MCFBuild.hpp"
#include "FileSystem.hpp"
#include "../MCF/Core/UniqueHandle.hpp"
#include "../MCF/Core/Exception.hpp"
#include <climits>
#include <cstddef>
using namespace MCFBuild;

namespace {

struct FileHandleCloser {
	constexpr HANDLE operator()() const noexcept {
		return INVALID_HANDLE_VALUE;
	}
	void operator()(HANDLE hFile) const noexcept {
		::CloseHandle(hFile);
	}
};

typedef MCF::UniqueHandle<FileHandleCloser> UniqueFileHandle;

}

namespace MCFBuild {

unsigned long long GetFileLastWriteTime(const MCF::WideString &wcsPath) noexcept {
	const UniqueFileHandle hFile(::CreateFileW(
		wcsPath.GetCStr(),
		FILE_READ_ATTRIBUTES,
		FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
		nullptr,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	));
	if(!hFile){
		return 0;
	}

	union {
		FILETIME ft;
		ULARGE_INTEGER uli;
	} u;
	if(!::GetFileTime(hFile.Get(), nullptr, nullptr, &u.ft)){
		return 0;
	}
	return u.uli.QuadPart;
}

MCF::WideString GetFullPath(const MCF::WideString &wcsSrc){
	MCF::WideString wcsRet;
	if(!wcsSrc.IsEmpty()){
		const DWORD dwSize = ::GetFullPathNameW(wcsSrc.GetCStr(), 0, nullptr, nullptr);
		wcsRet.Resize(dwSize);
		wcsRet.Resize(::GetFullPathNameW(wcsSrc.GetCStr(), dwSize, wcsRet.GetStr(), nullptr));
	}
	return std::move(wcsRet);
}

void CreateDirectory(const MCF::WideString &wcsPath){
	MCF::WideString wcsFullPath = GetFullPath(wcsPath);
	if(!wcsFullPath.IsEmpty() && (wcsFullPath.GetEnd()[-1] != L'\\')){
		wcsFullPath.Push(L'\\');
	}

	std::size_t uSlashPos = wcsFullPath.Find(L'\\', 0u);
	ASSERT(uSlashPos != MCF::WideString::NPOS);
	do {
		wcsFullPath[uSlashPos] = 0;
		const DWORD dwAttributes = GetFileAttributesW(wcsFullPath.GetCStr());
		wcsFullPath[uSlashPos] = L'\\';
		if(dwAttributes != INVALID_FILE_ATTRIBUTES){
			if((dwAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0){
				MCF_THROW(ERROR_PATH_NOT_FOUND, L"ACCESS_DIRECTORY_FAILED|"_WS + wcsFullPath);
			}
		} else {
			const DWORD dwError = ::GetLastError();
			if(dwError != ERROR_FILE_NOT_FOUND){
				MCF_THROW(dwError, L"ACCESS_DIRECTORY_FAILED|"_WS + wcsFullPath);
			}
			wcsFullPath[uSlashPos] = 0;
			const bool bResult = ::CreateDirectoryW(wcsFullPath.GetCStr(), nullptr);
			wcsFullPath[uSlashPos] = L'\\';
			if(!bResult){
				const DWORD dwError = ::GetLastError();
				if(dwError != ERROR_ALREADY_EXISTS){
					MCF_THROW(dwError, L"ACCESS_DIRECTORY_FAILED|"_WS + wcsFullPath);
				}
			}
		}
		uSlashPos = wcsFullPath.Find(L'\\', (std::ptrdiff_t)(uSlashPos + 1));
	} while(uSlashPos != MCF::WideString::NPOS);
}

}
