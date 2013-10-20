// Copyleft 2013, LH_Mouse. All wrongs reserved.

#include "PreCompiled.hpp"
#include <vector>
using namespace MCFBuild;

namespace {
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

	bool GetVersion(VS_FIXEDFILEINFO *pBuffer, HINSTANCE hInstance){
		const HRSRC hVersion = ::FindResourceW(hInstance, MAKEINTRESOURCEW(VS_VERSION_INFO), MAKEINTRESOURCEW(16));
		if(hVersion == NULL){
			return false;
		}

		const LPVOID pFileVersion = ::LockResource(::LoadResource(hInstance, hVersion));
		VS_FIXEDFILEINFO *pFixedFileInfo;
		UINT uFixedFileInfoSize;
		if(::VerQueryValueW(pFileVersion, L"\\", (LPVOID *)&pFixedFileInfo, &uFixedFileInfoSize) == FALSE){
			return false;
		}
		if(uFixedFileInfoSize != sizeof(VS_FIXEDFILEINFO)){
			return false;
		}
		std::memcpy(pBuffer, pFixedFileInfo, sizeof(VS_FIXEDFILEINFO));
		return true;
	}
}

namespace MCFBuild {
	extern void DoMain(std::size_t argc, const wchar_t *const *argv);
}

extern "C" int WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int){
	VS_FIXEDFILEINFO FixedFileInfo;
	std::memset(&FixedFileInfo, 0, sizeof(FixedFileInfo));
	GetVersion(&FixedFileInfo, hInstance);
	wchar_t awchWelcome[0x100];
	std::swprintf(
		awchWelcome,
		COUNTOF(awchWelcome),
		L"MCF Build [版本 %u.%u.%u.%u%ls]\nCopyleft 2013, LH_Mouse. All wrongs reserved.",
		(unsigned int)HIWORD(FixedFileInfo.dwFileVersionMS),
		(unsigned int)LOWORD(FixedFileInfo.dwFileVersionMS),
		(unsigned int)HIWORD(FixedFileInfo.dwFileVersionLS),
		(unsigned int)LOWORD(FixedFileInfo.dwFileVersionLS),
		((FixedFileInfo.dwFileFlagsMask & FixedFileInfo.dwFileFlags & VS_FF_DEBUG) != 0) ? L" Debug" : L""
	);
	Print(awchWelcome);

	int nMainRet = 0;
	try {
		const wchar_t *const pwszCmdLine = ::GetCommandLineW();
		std::vector<wchar_t> vecCmdLineBuffer(pwszCmdLine, pwszCmdLine + (std::wcslen(pwszCmdLine) + 1));
		const auto vecArgs(GetArgV(vecCmdLineBuffer));

		DoMain(vecArgs.size() - 1, vecArgs.data());
	} catch(Exception &e){
		Error();
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
