// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2012 - 2013. LH_Mouse. All wrongs reserved.

#include "StdMCF.hpp"
#include "WavPlayerClass.hpp"
#include "xXAudio2ManagerClass.hpp"
#include <vector>
#include <list>
using namespace MCF;

// 静态成员函数。
void WavPlayerClass::xParseFile(std::vector<BYTE> &vecbyFmt, std::vector<BYTE> &vecbyData, const void *pFile, std::size_t uSize){
	vecbyFmt.clear();
	vecbyData.clear();

	if(uSize == 0){
		return;
	}

	typedef struct xtagChunk {
		BYTE abyID[4];
		DWORD dwSize;
		BYTE abyData[0];
	} CHUNK;

	auto ChunkTok = [](const BYTE *(*parpbyContext)[2], const BYTE *pbyData, std::size_t uSize) -> const CHUNK * {
		if(pbyData != nullptr){
			const CHUNK *const pTestHeader = (const CHUNK *)pbyData;
			const BYTE *const pTestContents = pTestHeader->abyData;
			const BYTE *const pbyEnd = pbyData + uSize;

			const BYTE *pbyNextChunk = pTestContents + (std::ptrdiff_t)pTestHeader->dwSize;
			if(pbyNextChunk <= pbyEnd){
				(*parpbyContext)[0] = pbyNextChunk;
				(*parpbyContext)[1] = pbyEnd;

				return pTestHeader;
			}
		} else {
			const CHUNK *const pTestHeader = (const CHUNK *)(*parpbyContext)[0];
			const BYTE *const pTestContents = pTestHeader->abyData;
			const BYTE *const pbyEnd = (*parpbyContext)[1];

			const BYTE *pbyNextChunk = pTestContents + (std::ptrdiff_t)pTestHeader->dwSize;
			if(pbyNextChunk <= pbyEnd){
				(*parpbyContext)[0] = pbyNextChunk;
				(*parpbyContext)[1] = pbyEnd;

				return pTestHeader;
			}
		}
		return nullptr;
	};

	const BYTE *(arpbyContext)[2];
	const CHUNK *const pRoot = ChunkTok(&arpbyContext, (const BYTE *)pFile, uSize);
	if(pRoot == nullptr){
		return;
	} else if(std::memcmp(pRoot->abyID, "RIFF", 4) != 0){
		return;
	} else if(pRoot->dwSize < 0x24){		// "WAVE" + "fmt " + size + 0x10 + "data" + size
		return;
	} else if(std::memcmp(pRoot->abyData, "WAVE", 4) != 0){
		return;
	}

	const CHUNK *pCurrent = ChunkTok(&arpbyContext, pRoot->abyData + 4, pRoot->dwSize - 4);
	while(pCurrent != nullptr){
		if(std::memcmp(pCurrent->abyID, "fmt ", 4) == 0){
			vecbyFmt.assign(pCurrent->abyData, pCurrent->abyData + (std::ptrdiff_t)pCurrent->dwSize);
			if(pCurrent->dwSize < sizeof(WAVEFORMATEX)){
				vecbyFmt.resize(sizeof(WAVEFORMATEX));
				((WAVEFORMATEX *)vecbyFmt.data())->cbSize = 0;
			}
		} else if(std::memcmp(pCurrent->abyID, "data", 4) == 0){
			vecbyData.assign(pCurrent->abyData, pCurrent->abyData + (std::ptrdiff_t)pCurrent->dwSize);
		}

		pCurrent = ChunkTok(&arpbyContext, nullptr, 0);
	}
}

// 构造函数和析构函数。
WavPlayerClass::WavPlayerClass(){
	xm_pXAudio2Manager = new xXAudio2ManagerClass(this);
}
WavPlayerClass::~WavPlayerClass(){
	delete xm_pXAudio2Manager;
}

// 其他非静态成员函数。
void WavPlayerClass::xPrePlayBlock(std::size_t uID, std::size_t uSerialNumber){
	UNREF_PARAM(uID);
	UNREF_PARAM(uSerialNumber);
}
void WavPlayerClass::xPostPlayBlock(std::size_t uID, std::size_t uSerialNumber, bool bIsEnd){
	UNREF_PARAM(uID);
	UNREF_PARAM(uSerialNumber);
	UNREF_PARAM(bIsEnd);
}

std::size_t WavPlayerClass::Start(
	const void *const *ppAttackFiles,
	const std::size_t *puAttackSizes,
	std::size_t uAttackFileCount,
	const void *const *ppLoopFiles,
	const std::size_t *puLoopSizes,
	std::size_t uLoopFileCount,
	const void *const *ppDecayFiles,
	const std::size_t *puDecaySizes,
	std::size_t uDecayFileCount,
	std::size_t uLoopCount,
	bool bSuspended
){
	std::vector<BYTE> vecbyFmt;
	std::vector<BYTE> vecbyTempFmt;
	std::vector<std::vector<BYTE>> vecvecbyAttackData(uAttackFileCount);
	std::vector<std::vector<BYTE>> vecvecbyLoopData(uLoopFileCount);
	std::vector<std::vector<BYTE>> vecvecbyDecayData(uDecayFileCount);

	for(std::size_t i = 0; i < uAttackFileCount; ++i){
		xParseFile(vecbyTempFmt, vecvecbyAttackData[i], ppAttackFiles[i], puAttackSizes[i]);
		if(vecbyTempFmt.empty() || vecvecbyAttackData[i].empty()){
			return 0;
		}
		if(vecbyFmt.empty()){
			vecbyFmt.swap(vecbyTempFmt);
		} else if((vecbyFmt.size() != vecbyTempFmt.size()) || (std::memcmp(vecbyFmt.data(), vecbyTempFmt.data(), vecbyFmt.size()) != 0)){
			return 0;
		}
	}
	for(std::size_t i = 0; i < uLoopFileCount; ++i){
		xParseFile(vecbyTempFmt, vecvecbyLoopData[i], ppLoopFiles[i], puLoopSizes[i]);
		if(vecbyTempFmt.empty() || vecvecbyLoopData[i].empty()){
			return 0;
		}
		if(vecbyFmt.empty()){
			vecbyFmt.swap(vecbyTempFmt);
		} else if((vecbyFmt.size() != vecbyTempFmt.size()) || (std::memcmp(vecbyFmt.data(), vecbyTempFmt.data(), vecbyFmt.size()) != 0)){
			return 0;
		}
	}
	for(std::size_t i = 0; i < uDecayFileCount; ++i){
		xParseFile(vecbyTempFmt, vecvecbyDecayData[i], ppDecayFiles[i], puDecaySizes[i]);
		if(vecbyTempFmt.empty() || vecvecbyDecayData[i].empty()){
			return 0;
		}
		if(vecbyFmt.empty()){
			vecbyFmt.swap(vecbyTempFmt);
		} else if((vecbyFmt.size() != vecbyTempFmt.size()) || (std::memcmp(vecbyFmt.data(), vecbyTempFmt.data(), vecbyFmt.size()) != 0)){
			return 0;
		}
	}

	if(vecbyFmt.empty()){
		return 0;
	}

	return xm_pXAudio2Manager->Start(
		(const WAVEFORMATEX *)vecbyFmt.data(),
		std::move(vecvecbyAttackData),
		std::move(vecvecbyLoopData),
		std::move(vecvecbyDecayData),
		uLoopCount,
		bSuspended
	);
}
void WavPlayerClass::Stop(std::size_t uID){
	xm_pXAudio2Manager->Stop(uID);
}
void WavPlayerClass::StopAll(){
	return xm_pXAudio2Manager->StopAll();
}
void WavPlayerClass::Clear(std::size_t uID){
	xm_pXAudio2Manager->Clear(uID);
}
void WavPlayerClass::ClearAll(){
	xm_pXAudio2Manager->ClearAll();
}

void WavPlayerClass::Resume(std::size_t uID){
	xm_pXAudio2Manager->Resume(uID);
}
void WavPlayerClass::ResumeAll(){
	xm_pXAudio2Manager->ResumeAll();
}
void WavPlayerClass::Pause(std::size_t uID){
	xm_pXAudio2Manager->Pause(uID);
}
void WavPlayerClass::PauseAll(){
	xm_pXAudio2Manager->PauseAll();
}

double WavPlayerClass::GetVolume() const {
	return xm_pXAudio2Manager->GetVolume();
}
void WavPlayerClass::SetVolume(double lfNewVolume){
	xm_pXAudio2Manager->SetVolume(lfNewVolume);
}
