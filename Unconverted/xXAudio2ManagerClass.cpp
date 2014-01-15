// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2012 - 2013. LH_Mouse. All wrongs reserved.

#include "StdMCF.hpp"
#include "xXAudio2ManagerClass.hpp"
#include "xXAudio2SourceVoiceClass.hpp"
#include <vector>
using namespace MCF;

// 构造函数和析构函数。
WavPlayerClass::xXAudio2ManagerClass::xXAudio2ManagerClass(WavPlayerClass *pOwner) :
	xm_pOwner(pOwner)
{
	::CoInitialize(nullptr);

	VERIFY(SUCCEEDED(::XAudio2Create(&xm_pXAudio2)));
	VERIFY(SUCCEEDED(xm_pXAudio2->CreateMasteringVoice(&xm_pMasteringVoice)));
}
WavPlayerClass::xXAudio2ManagerClass::~xXAudio2ManagerClass(){
	ClearAll();

	xm_pMasteringVoice->DestroyVoice();
	xm_pXAudio2->Release();

	::CoUninitialize();
}

// 其他非静态成员函数。
void WavPlayerClass::xXAudio2ManagerClass::xFlush(){
	auto iter = xm_mapSourceVoices.cbegin();
	while(iter != xm_mapSourceVoices.end()){
		if(iter->second->IsStopped()){
			iter = xm_mapSourceVoices.erase(iter);
		} else {
			++iter;
		}
	}
}

std::size_t WavPlayerClass::xXAudio2ManagerClass::Start(
	const WAVEFORMATEX *pWaveFormatEx,
	std::vector<std::vector<BYTE>> &&vecvecbyAttackData,
	std::vector<std::vector<BYTE>> &&vecvecbyLoopData,
	std::vector<std::vector<BYTE>> &&vecvecbyDecayData,
	std::size_t uLoopCount,
	bool bSuspended
){
	xFlush();

	++xm_uLastID;
	xm_mapSourceVoices.emplace(
		std::make_pair(
			xm_uLastID,
			new xXAudio2SourceVoiceClass(
				this,
				xm_uLastID,
				pWaveFormatEx,
				std::move(vecvecbyAttackData),
				std::move(vecvecbyLoopData),
				std::move(vecvecbyDecayData),
				uLoopCount,
				bSuspended
			)
		)
	);
	return xm_uLastID;
}
void WavPlayerClass::xXAudio2ManagerClass::Stop(std::size_t uID){
	const auto iter = xm_mapSourceVoices.find(uID);
	if(iter != xm_mapSourceVoices.end()){
		iter->second->Stop();
	}
}
void WavPlayerClass::xXAudio2ManagerClass::StopAll(){
	for(auto iter = xm_mapSourceVoices.begin(); iter != xm_mapSourceVoices.end(); ++iter){
		iter->second->Stop();
	}
}
void WavPlayerClass::xXAudio2ManagerClass::Clear(std::size_t uID){
	xm_mapSourceVoices.erase(uID);
}
void WavPlayerClass::xXAudio2ManagerClass::ClearAll(){
	xm_mapSourceVoices.clear();
}

void WavPlayerClass::xXAudio2ManagerClass::Resume(std::size_t uID){
	const auto iter = xm_mapSourceVoices.find(uID);
	if(iter != xm_mapSourceVoices.end()){
		iter->second->Resume();
	}
}
void WavPlayerClass::xXAudio2ManagerClass::ResumeAll(){
	for(auto iter = xm_mapSourceVoices.begin(); iter != xm_mapSourceVoices.end(); ++iter){
		iter->second->Resume();
	}
}
void WavPlayerClass::xXAudio2ManagerClass::Pause(std::size_t uID){
	const auto iter = xm_mapSourceVoices.find(uID);
	if(iter != xm_mapSourceVoices.end()){
		iter->second->Pause();
	}
}
void WavPlayerClass::xXAudio2ManagerClass::PauseAll(){
	for(auto iter = xm_mapSourceVoices.begin(); iter != xm_mapSourceVoices.end(); ++iter){
		iter->second->Pause();
	}
}

double WavPlayerClass::xXAudio2ManagerClass::GetVolume() const {
	float fVolume;
	xm_pMasteringVoice->GetVolume(&fVolume);
	return fVolume;
}
void WavPlayerClass::xXAudio2ManagerClass::SetVolume(double lfNewVolume){
	if(lfNewVolume > 1.0){
		xm_pMasteringVoice->SetVolume(1.0f);
	} else if(lfNewVolume < 0.0){
		xm_pMasteringVoice->SetVolume(0.0f);
	} else {
		xm_pMasteringVoice->SetVolume((float)lfNewVolume);
	}
}
