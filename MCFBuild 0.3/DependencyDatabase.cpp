// Copyleft 2013, LH_Mouse. All wrongs reserved.

#include "General.hpp"
#include "DependencyDatabase.hpp"
#include <vector>
#include <algorithm>
#include <iterator>
#include <climits>
#include "UniqueHandle.hpp"
using namespace MCFBuild;

namespace {
	struct FileCloser {
		constexpr HANDLE operator()(){
			return INVALID_HANDLE_VALUE;
		}
		void operator()(HANDLE hObj){
			::CloseHandle(hObj);
		}
	};
}

void DependencyDatabase::SaveToFile(const wvstring &wcsPath) const {
	const auto hFile = MCF::UniqueHandle<HANDLE, FileCloser>(::CreateFileW(wcsPath.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, 0, NULL));
	if(!hFile.IsGood()){
		const DWORD dwError = ::GetLastError();
		throw Exception{dwError, L"创建数据库文件“" + wcsPath + L"”失败。"};
	}

	const auto Write = [&](const void *pBuffer, std::size_t uSize) -> void {
		std::size_t uBytesTotal = 0;

		DWORD dwBytesWritten;
		do {
			if(::WriteFile(hFile, ((BYTE *)pBuffer) + uBytesTotal, (DWORD)(uSize - uBytesTotal), &dwBytesWritten, nullptr) == FALSE){
				const DWORD dwError = ::GetLastError();
				throw Exception{dwError, L"写入数据库文件“" + wcsPath + L"”失败。"};
			}
			uBytesTotal += dwBytesWritten;
		} while(dwBytesWritten != 0);
	};

	const std::uint32_t u32MapSize = (std::uint32_t)xm_mapDependencies.size();
	Write(&u32MapSize, sizeof(u32MapSize));
	for(const auto &DependencyItem : xm_mapDependencies){
		const std::uint32_t u32KeyLength = (std::uint32_t)DependencyItem.first.size();
		Write(&u32KeyLength, sizeof(u32KeyLength));
		Write(DependencyItem.first.data(), u32KeyLength * sizeof(wchar_t));

		const std::int64_t n64Timestamp = (std::int64_t)DependencyItem.second.m_llTimestamp;
		Write(&n64Timestamp, sizeof(n64Timestamp));

		const std::uint32_t u32SetSize = (std::uint32_t)DependencyItem.second.m_setDependencyFiles.size();
		Write(&u32SetSize, sizeof(u32SetSize));
		for(const auto &wcsDependencyFile : DependencyItem.second.m_setDependencyFiles){
			const std::uint32_t u32FileLength = (std::uint32_t)wcsDependencyFile.size();
			Write(&u32FileLength, sizeof(u32FileLength));
			Write(wcsDependencyFile.data(), u32FileLength * sizeof(wchar_t));
		}
	}

	::SetEndOfFile(hFile);
}
void DependencyDatabase::LoadFromFile(const wvstring &wcsPath){
	const auto hFile = MCF::UniqueHandle<HANDLE, FileCloser>(::CreateFileW(wcsPath.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, NULL));
	if(!hFile.IsGood()){
		const DWORD dwError = ::GetLastError();
		if(dwError != ERROR_FILE_NOT_FOUND){
			throw Exception{dwError, L"打开数据库文件“" + wcsPath + L"”失败。"};
		}
		xm_mapDependencies.clear();
		return;
	}

	const auto Read = [&](void *pBuffer, std::size_t uSize) -> void {
		std::size_t uBytesTotal = 0;

		DWORD dwBytesRead;
		do {
			if(::ReadFile(hFile, ((BYTE *)pBuffer) + uBytesTotal, (DWORD)(uSize - uBytesTotal), &dwBytesRead, nullptr) == FALSE){
				const DWORD dwError = ::GetLastError();
				throw Exception{dwError, L"读取数据库文件“" + wcsPath + L"”失败。"};
			}
			uBytesTotal += dwBytesRead;
		} while(dwBytesRead != 0);

		if(uBytesTotal < uSize){
			throw Exception{ERROR_INVALID_DATA, L"数据库文件“" + wcsPath + L"”损坏。"};
		}
	};

	std::map<wvstring, Dependencies> mapDependencies;

	std::uint32_t u32MapSize;
	Read(&u32MapSize, sizeof(u32MapSize));
	for(std::uint32_t i = 0; i < u32MapSize; ++i){
		std::uint32_t u32KeyLength;
		Read(&u32KeyLength, sizeof(u32KeyLength));
		wvstring wcsKey(u32KeyLength, 0);
		Read(&wcsKey[0], u32KeyLength * sizeof(wchar_t));
		auto &Dependencies = mapDependencies[std::move(wcsKey)];

		std::int64_t n64Timestamp;
		Read(&n64Timestamp, sizeof(n64Timestamp));
		Dependencies.m_llTimestamp = n64Timestamp;

		std::uint32_t u32SetSize;
		Read(&u32SetSize, sizeof(u32SetSize));
		for(std::uint32_t j = 0; j < u32SetSize; ++j){
			std::uint32_t u32FileLength;
			Read(&u32FileLength, sizeof(u32FileLength));
			wvstring wcsFile(u32FileLength, 0);
			Read(&wcsFile[0], u32FileLength * sizeof(wchar_t));
			Dependencies.m_setDependencyFiles.insert(std::move(wcsFile));
		}
	}

	xm_mapDependencies.swap(mapDependencies);
}
