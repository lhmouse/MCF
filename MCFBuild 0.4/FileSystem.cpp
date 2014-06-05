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

bool GetFileContents(MCF::Vector<unsigned char> &vecData, const MCF::WideString &wcsPath){
	const auto wcsFullPath = GetFullPath(wcsPath);
	const auto pFile = MCF::File::OpenNoThrow(wcsFullPath, true, false, false);
	if(!pFile){
		return false;
	}
	const auto u64FileSize = pFile->GetSize();
	if(u64FileSize > (std::size_t)-1){
		FORMAT_THROW(ERROR_NOT_ENOUGH_MEMORY, L"FILE_TOO_LARGE|"_wso + wcsFullPath);
	}
	vecData.Resize(u64FileSize);
	pFile->Read(vecData.GetData(), u64FileSize, 0);
	return true;
}
void PutFileContents(const MCF::WideString &wcsPath, const void *pData, std::size_t uSize){
	const auto wcsFullPath = GetFullPath(wcsPath);
	const auto pFile = MCF::File::OpenNoThrow(wcsFullPath, false, true, true);
	if(pFile){
		FORMAT_THROW(::GetLastError(), L"OPEN_FILE_FAILED|"_wso + wcsFullPath);
	}
	pFile->Clear();
	pFile->Write(0, pData, uSize);
}
bool GetFileSha256(Sha256 &shaChecksum, const MCF::WideString &wcsPath){
	const auto wcsFullPath = GetFullPath(wcsPath);
	const auto pFile = MCF::File::OpenNoThrow(wcsFullPath, true, false, false);
	if(!pFile){
		return false;
	}
	const auto u64FileSize = pFile->GetSize();
	MCF::Sha256 shaHasher;
	shaHasher.Update(nullptr, 0);
	if(u64FileSize > 0){
		unsigned char abyTemp1[32 * 1024], abyTemp2[sizeof(abyTemp1)];
		auto *pbyCurBuffer = abyTemp1, *pbyBackBuffer = abyTemp2;
		std::size_t uBytesCur = pFile->Read(pbyCurBuffer, sizeof(abyTemp1), 0);

		std::uint64_t u64Offset = uBytesCur;
		while(u64Offset < u64FileSize){
			const auto uBytesBack = pFile->Read(
				pbyBackBuffer, sizeof(abyTemp1), u64Offset,
				[&]{
					shaHasher.Update(pbyCurBuffer, uBytesCur);
				}
			);
			u64Offset += uBytesBack;

			std::swap(pbyCurBuffer, pbyBackBuffer);
			uBytesCur = uBytesBack;
		}

		shaHasher.Update(pbyCurBuffer, uBytesCur);
	}
	shaHasher.Finalize(*(unsigned char (*)[32])shaChecksum.data());
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
				FORMAT_THROW(ERROR_PATH_NOT_FOUND, L"OPEN_DIRECTORY_FAILED|"_wso + wcsFullPath);
			}
		} else {
			const DWORD dwError = ::GetLastError();
			if(dwError != ERROR_FILE_NOT_FOUND){
				FORMAT_THROW(dwError, L"OPEN_DIRECTORY_FAILED|"_wso + wcsFullPath);
			}
			wcsFullPath[uSlashPos] = 0;
			const bool bResult = ::CreateDirectoryW(wcsFullPath.GetCStr(), nullptr);
			wcsFullPath[uSlashPos] = L'\\';
			if(!bResult){
				const DWORD dwError = ::GetLastError();
				if(dwError != ERROR_ALREADY_EXISTS){
					FORMAT_THROW(dwError, L"OPEN_DIRECTORY_FAILED|"_wso + wcsFullPath);
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
			FORMAT_THROW(dwError, L"DELETE_FILE_FAILED|"_wso + wcsFullPath);
		}
	}
}

}
