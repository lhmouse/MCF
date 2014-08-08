// MCF Build
// Copyleft 2014, LH_Mouse. All wrongs reserved.

#include "MCFBuild.hpp"
#include "FileSystem.hpp"
#include "../MCF/Core/Exception.hpp"
#include "../MCF/Core/File.hpp"
#include "../MCF/Hash/Sha256.hpp"
#include <climits>
#include <cstdint>
#include <cstddef>
using namespace MCFBuild;

constexpr std::size_t FILE_BUFFER_SIZE	= 64 * 0x400; // 64 KiB

namespace MCFBuild {

MCF::WideString GetFullPath(const MCF::WideString &wcsSrc){
	MCF::WideString wcsRet;
	if(!wcsSrc.IsEmpty()){
		const DWORD dwSize = ::GetFullPathNameW(wcsSrc.GetCStr(), 0, nullptr, nullptr);
		wcsRet.Resize(dwSize);
		wcsRet.Resize(::GetFullPathNameW(wcsSrc.GetCStr(), dwSize, wcsRet.GetStr(), nullptr));
	}
	return std::move(wcsRet);
}

bool GetFileContents(MCF::StreamBuffer &sbufData, const MCF::WideString &wcsPath, bool bThrowOnFailure){
	const auto wcsFullPath = GetFullPath(wcsPath);
	const auto pFile = MCF::File::OpenNoThrow(wcsFullPath, MCF::File::TO_READ);
	if(!pFile){
		if(bThrowOnFailure){
			FORMAT_THROW(::GetLastError(), L"OPEN_FILE_FAILED\0"_wso + wcsFullPath);
		}
		return false;
	}

	const auto u64FileSize = pFile->GetSize();
	std::uint64_t u64Offset = 0;
	while(u64Offset < u64FileSize){
		unsigned char abyTemp[FILE_BUFFER_SIZE];
		const auto uBytesRead = pFile->Read(abyTemp, sizeof(abyTemp), u64Offset);
		u64Offset += uBytesRead;
		sbufData.Insert(abyTemp, uBytesRead);
	}
	return true;
}
void PutFileContents(const MCF::WideString &wcsPath, const MCF::StreamBuffer &sbufData){
	const auto wcsFullPath = GetFullPath(wcsPath);
	const auto pFile = MCF::File::OpenNoThrow(wcsFullPath, MCF::File::TO_WRITE | MCF::File::NO_CREATE);
	if(pFile){
		FORMAT_THROW(::GetLastError(), L"OPEN_FILE_FAILED\0"_wso + wcsFullPath);
	}
	pFile->Clear();

	std::uint64_t u64Offset = 0;
	sbufData.Traverse(
		[&](auto pbyData, auto uSize){
			pFile->Write(u64Offset, pbyData, uSize);
			u64Offset += uSize;
		}
	);
}
bool GetFileSha256(Sha256 &vSha256, const MCF::WideString &wcsPath, bool bThrowOnFailure){
	const auto wcsFullPath = GetFullPath(wcsPath);
	const auto pFile = MCF::File::OpenNoThrow(wcsFullPath, MCF::File::TO_READ);
	if(!pFile){
		if(bThrowOnFailure){
			FORMAT_THROW(::GetLastError(), L"OPEN_FILE_FAILED\0"_wso + wcsFullPath);
		}
		return false;
	}
	const auto u64FileSize = pFile->GetSize();
	MCF::Sha256 shaHasher;
	shaHasher.Update(nullptr, 0);
	if(u64FileSize > 0){
		unsigned char abyTemp1[FILE_BUFFER_SIZE], abyTemp2[FILE_BUFFER_SIZE];
		auto *pbyCurBuffer = abyTemp1, *pbyBackBuffer = abyTemp2;
		std::size_t uBytesCur = pFile->Read(pbyCurBuffer, FILE_BUFFER_SIZE, 0);

		std::uint64_t u64Offset = uBytesCur;
		while(u64Offset < u64FileSize){
			const auto uBytesBack = pFile->Read(
				pbyBackBuffer, FILE_BUFFER_SIZE, u64Offset,
				[&]{ shaHasher.Update(pbyCurBuffer, uBytesCur); }, []{ }
			);
			u64Offset += uBytesBack;

			std::swap(pbyCurBuffer, pbyBackBuffer);
			uBytesCur = uBytesBack;
		}

		shaHasher.Update(pbyCurBuffer, uBytesCur);
	}
	shaHasher.Finalize(*(unsigned char (*)[32])vSha256.data());
	return true;
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
				FORMAT_THROW(ERROR_PATH_NOT_FOUND, L"OPEN_DIRECTORY_FAILED\0"_wso + wcsFullPath);
			}
		} else {
			const DWORD dwError = ::GetLastError();
			if(dwError != ERROR_FILE_NOT_FOUND){
				FORMAT_THROW(dwError, L"OPEN_DIRECTORY_FAILED\0"_wso + wcsFullPath);
			}
			wcsFullPath[uSlashPos] = 0;
			const bool bResult = ::CreateDirectoryW(wcsFullPath.GetCStr(), nullptr);
			wcsFullPath[uSlashPos] = L'\\';
			if(!bResult){
				const DWORD dwError = ::GetLastError();
				if(dwError != ERROR_ALREADY_EXISTS){
					FORMAT_THROW(dwError, L"OPEN_DIRECTORY_FAILED\0"_wso + wcsFullPath);
				}
			}
		}
		uSlashPos = wcsFullPath.Find(L'\\', (std::ptrdiff_t)(uSlashPos + 1));
	} while(uSlashPos != MCF::WideString::NPOS);
}
void RemoveFile(const MCF::WideString &wcsPath){
	const auto wcsFullPath = GetFullPath(wcsPath);
	if(!::DeleteFileW(wcsFullPath.GetCStr())){
		const DWORD dwError = ::GetLastError();
		if((dwError != ERROR_FILE_NOT_FOUND) && (dwError != ERROR_PATH_NOT_FOUND)){
			FORMAT_THROW(dwError, L"DELETE_FILE_FAILED\0"_wso + wcsFullPath);
		}
	}
}

}
