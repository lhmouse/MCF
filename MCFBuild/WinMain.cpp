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

	void GetVersion(unsigned short *pushVersion, HINSTANCE hInstance){
		struct EnumVersionResNameProc {
			static BOOL CALLBACK Proc(HMODULE, LPCWSTR, LPWSTR lpwszName, LONG_PTR lParam){
				*(LPWSTR *)lParam = lpwszName;
				return FALSE;
			}
		};

		std::fill_n(pushVersion, 4, 0);

		LPCWSTR pwszResourceName = nullptr;
		::EnumResourceNamesW(hInstance, (LPCWSTR)16, &EnumVersionResNameProc::Proc, (LONG_PTR)&pwszResourceName);
		if(pwszResourceName == nullptr){
			return;
		}

		const HRSRC hVersionResource = ::FindResourceW(hInstance, pwszResourceName, (LPCWSTR)16);
		const LPVOID pFileVersionBuffer = ::LockResource(::LoadResource(hInstance, hVersionResource));
		VS_FIXEDFILEINFO *pFixedFileInfo;
		UINT uFixedFileInfoSize;
		if(::VerQueryValueW(pFileVersionBuffer, L"\\", (LPVOID *)&pFixedFileInfo, &uFixedFileInfoSize) != FALSE){
			pushVersion[0] = HIWORD(pFixedFileInfo->dwFileVersionMS);
			pushVersion[1] = LOWORD(pFixedFileInfo->dwFileVersionMS);
			pushVersion[2] = HIWORD(pFixedFileInfo->dwFileVersionLS);
			pushVersion[3] = LOWORD(pFixedFileInfo->dwFileVersionLS);
		}
	}
}

namespace MCFBuild {
	extern void DoMain(std::size_t argc, const wchar_t *const *argv);
}

extern "C" int WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int){
	unsigned short aushVersion[4];
	GetVersion(aushVersion, hInstance);
	wchar_t awchWelcome[0x100];
	std::swprintf(
		awchWelcome,
		COUNTOF(awchWelcome),
		L"MCF Build [版本 %hu.%hu.%hu.%hu]\nCopyleft 2013, LH_Mouse. All wrongs reserved.",
		aushVersion[0],
		aushVersion[1],
		aushVersion[2],
		aushVersion[3]
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
