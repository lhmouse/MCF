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

bool GetFileSha256(Sha256 &shaRet, const MCF::WideString &wcsPath){
	const auto pFile = MCF::File::OpenNoThrow(wcsPath, true, false, false);
	if(!pFile){
		return false;
	}

	MCF::Sha256 shaHasher;
	shaHasher.Update(nullptr, 0);
	const auto u64FileSize = pFile->GetSize();
	if(u64FileSize > 0){
		unsigned char abyTemp1[32 * 1024], abyTemp2[sizeof(abyTemp1)];
		auto *pbyCurBuffer = abyTemp1, *pbyBackBuffer = abyTemp2;
		std::size_t uBytesInBuffer = pFile->Read(pbyCurBuffer, sizeof(abyTemp1), 0);
		std::uint64_t u64Offset = uBytesInBuffer;
		while(u64Offset < u64FileSize){
			std::swap(pbyCurBuffer, pbyBackBuffer);
			uBytesInBuffer = pFile->Read(
				pbyCurBuffer, sizeof(abyTemp1), u64Offset,
				[&]{ shaHasher.Update(pbyBackBuffer, uBytesInBuffer); }
			);
			u64Offset += uBytesInBuffer;
		}
		shaHasher.Update(pbyCurBuffer, uBytesInBuffer);
	}
	shaHasher.Finalize(shaRet.abyChecksum);
	return true;
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
