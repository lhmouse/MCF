// Copyleft 2013, LH_Mouse. All wrongs reserved.

#include "General.hpp"
#include <vector>
using namespace MCFBuild;

namespace {
	struct VERSION_INFO {
		wvstring wcsFileDescription;
		unsigned short aushVersion[4];
		bool bDebug;
		wvstring wcsLegalCopyright;
	};

	std::vector<const wchar_t *> GetArgV(std::vector<wchar_t> &vecCmdLine){
		std::vector<const wchar_t *> vecRet;

		auto iterWrite = vecCmdLine.begin();
		auto iterRead = vecCmdLine.cbegin();
		bool bInQuote = false;
		const wchar_t *pwszArgBegin = nullptr;
		for(;;){
			const wchar_t ch = *iterRead;
			if(ch == 0){
				*iterWrite = 0;
				if(pwszArgBegin != nullptr){
					vecRet.push_back(pwszArgBegin);
				}
				break;
			}
			switch(ch){
			case L'\"':
				if(bInQuote){
					if(*(iterRead + 1) == L'\"'){
						++iterRead;
						*(iterWrite++) = L'\"';
					} else {
						bInQuote = false;
					}
				} else {
					bInQuote = true;
					if(pwszArgBegin == nullptr){
						pwszArgBegin = &*iterWrite;
					}
				}
				break;
			case L' ':
			case L'\t':
				if(!bInQuote){
					if(pwszArgBegin != nullptr){
						*(iterWrite++) = 0;
						vecRet.push_back(pwszArgBegin);
						pwszArgBegin = nullptr;
					}
					break;
				}
			default:
				*iterWrite = ch;
				if(pwszArgBegin == nullptr){
					pwszArgBegin = &*iterWrite;
				}
				++iterWrite;
				break;
			}
			++iterRead;
		}
		vecRet.push_back(nullptr);

		return std::move(vecRet);
	}

	VERSION_INFO GetVersionInfo(HINSTANCE hInstance){
		VERSION_INFO ret;

		const HRSRC hVersion = ::FindResourceW(hInstance, MAKEINTRESOURCEW(VS_VERSION_INFO), MAKEINTRESOURCEW(16));
		if(hVersion != NULL){
			const LPVOID pFileVersion = ::LockResource(::LoadResource(hInstance, hVersion));
			void *pBuffer;
			UINT uSize;

			if(::VerQueryValueW(pFileVersion, L"\\", &pBuffer, &uSize) != FALSE){
				const auto pFixedFileInfo = (const VS_FIXEDFILEINFO *)pBuffer;

				ret.aushVersion[0]	= HIWORD(pFixedFileInfo->dwFileVersionMS);
				ret.aushVersion[1]	= LOWORD(pFixedFileInfo->dwFileVersionMS);
				ret.aushVersion[2]	= HIWORD(pFixedFileInfo->dwFileVersionLS);
				ret.aushVersion[3]	= LOWORD(pFixedFileInfo->dwFileVersionLS);
				ret.bDebug			= (pFixedFileInfo->dwFileFlagsMask & pFixedFileInfo->dwFileFlags & VS_FF_DEBUG) != 0;
			}

			if(::VerQueryValueW(pFileVersion, L"\\VarFileInfo\\Translation", &pBuffer, &uSize) != FALSE){
				wvstring wcsPrefix(64, 0);
				wcsPrefix.resize((std::size_t)std::swprintf(
					&wcsPrefix[0],
					wcsPrefix.size(),
					L"\\StringFileInfo\\%04X%04X\\",
					(unsigned int)((const WORD *)pBuffer)[0],
					(unsigned int)((const WORD *)pBuffer)[1]
				));

				if(::VerQueryValueW(pFileVersion, (wcsPrefix + L"FileDescription").c_str(), &pBuffer, &uSize) != FALSE){
					ret.wcsFileDescription.assign((const wchar_t *)pBuffer, uSize);
					ret.wcsFileDescription.resize(std::wcslen(ret.wcsFileDescription.c_str()));
				}
				if(::VerQueryValueW(pFileVersion, (wcsPrefix + L"LegalCopyright").c_str(), &pBuffer, &uSize) != FALSE){
					ret.wcsLegalCopyright.assign((const wchar_t *)pBuffer, uSize);
					ret.wcsLegalCopyright.resize(std::wcslen(ret.wcsLegalCopyright.c_str()));
				}
			}
		}

		return std::move(ret);
	}
}

namespace MCFBuild {
	extern void DoMain(std::size_t argc, const wchar_t *const *argv);
}

extern "C" int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int){
	const VERSION_INFO VersionInfo = GetVersionInfo(hInstance);

	Output(
		L"%ls [版本 %hu.%hu.%hu.%hu%ls]",
		VersionInfo.wcsFileDescription.c_str(),
		VersionInfo.aushVersion[0],
		VersionInfo.aushVersion[1],
		VersionInfo.aushVersion[2],
		VersionInfo.aushVersion[3],
		VersionInfo.bDebug ? L" Debug" : L""
	);
	Output(VersionInfo.wcsLegalCopyright);
	Output();

	int nMainRet = 0;
	try {
		const wchar_t *const pwszCmdLine = ::GetCommandLineW();
		std::vector<wchar_t> vecCmdLineBuffer(pwszCmdLine, pwszCmdLine + (std::wcslen(pwszCmdLine) + 1));
		const auto vecArgs(GetArgV(vecCmdLineBuffer));

		DoMain(vecArgs.size() - 1, vecArgs.data());
	} catch(Exception &e){
		Error(L"错误：" + e.wcsDescription);

		PVOID pDescription;
		const DWORD dwLen = ::FormatMessageW(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			nullptr,
			e.ulCode,
			0,
			(LPWSTR)&pDescription,
			0,
			nullptr
		);
		if(dwLen != 0){
			if(((LPCWSTR)pDescription)[dwLen - 1] == L'\n'){
				((LPWSTR)pDescription)[dwLen - 1] = 0;
			}
			Error(L"错误代码 %lu：%ls", e.ulCode, (LPCWSTR)pDescription);
			::LocalFree((HLOCAL)pDescription);
		} else {
			Error(L"错误代码 %lu。", e.ulCode);
		}
		nMainRet = (int)e.ulCode;
	}
	return nMainRet;
}
