// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2012 - 2013. LH_Mouse. All wrongs reserved.

#include "StdMCF.hpp"
#include "xXAudio2SourceVoiceClass.hpp"
#include <cstdlib>
#include <vector>
using namespace MCF;

static_assert(sizeof(void *) >= sizeof(std::uintptr_t), "Oh fuck");

// 构造函数和析构函数。
WavPlayerClass::xXAudio2ManagerClass::xXAudio2SourceVoiceClass::xXAudio2SourceVoiceClass(
	WavPlayerClass::xXAudio2ManagerClass *pOwner,
	std::size_t uID,
	const WAVEFORMATEX *pWaveFormatEx,
	std::vector<std::vector<BYTE>> &&vecvecbyAttackData,
	std::vector<std::vector<BYTE>> &&vecvecbyLoopData,
	std::vector<std::vector<BYTE>> &&vecvecbyDecayData,
	std::size_t uLoopCount,
	bool bSuspended
) :
	xm_pOwner		(pOwner),
	xm_uID			(uID),
	xm_uLoopCount	(uLoopCount)
{
	xm_VoiceCallback.m_pOwner	= this;
	VERIFY(SUCCEEDED(xm_pOwner->xm_pXAudio2->CreateSourceVoice(&xm_pSourceVoice, pWaveFormatEx, 0, XAUDIO2_MAX_FREQ_RATIO, &xm_VoiceCallback)));
	xm_uSerialNumber			= 0;
	xm_uLoopIndex				= 0;
	xm_eState					= STOPPED;
	xm_bStopHere				= false;

	xm_vecvecbyAttackData		= std::move(vecvecbyAttackData);
	xm_vecvecbyLoopData			= std::move(vecvecbyLoopData);
	xm_vecvecbyDecayData		= std::move(vecvecbyDecayData);

	if(!xm_vecvecbyAttackData.empty()){
		xPickAndSubmitBuffer(xm_vecvecbyAttackData);
		xm_eState = ATTACKING;
	} else if(!xm_vecvecbyLoopData.empty()){
		xPickAndSubmitBuffer(xm_vecvecbyLoopData);
		xm_eState = LOOPING;
	} else if(!xm_vecvecbyDecayData.empty()){
		xPickAndSubmitBuffer(xm_vecvecbyDecayData);
		xm_eState = DECAYING;
	}
	if(!bSuspended && (xm_eState != STOPPED)){
		VERIFY(SUCCEEDED(xm_pSourceVoice->Start()));
	}
}
WavPlayerClass::xXAudio2ManagerClass::xXAudio2SourceVoiceClass::~xXAudio2SourceVoiceClass(){
	xm_pSourceVoice->DestroyVoice();
}

void WavPlayerClass::xXAudio2ManagerClass::xXAudio2SourceVoiceClass::xOnBufferStart(std::uintptr_t uContext){
	xPumpStateAndPreSubmitBuffer();

	xm_uExpectingEndOf = uContext;
	xm_pOwner->xm_pOwner->xPrePlayBlock(xm_uID, xm_uSerialNumber);
}
void WavPlayerClass::xXAudio2ManagerClass::xXAudio2SourceVoiceClass::xOnBufferEnd(std::uintptr_t uContext){
	if(uContext == xm_uExpectingEndOf){
		xm_pOwner->xm_pOwner->xPostPlayBlock(xm_uID, xm_uSerialNumber, xm_bStopHere);

		++xm_uSerialNumber;
		xm_eState = xm_eNextState;
	}
}

void WavPlayerClass::xXAudio2ManagerClass::xXAudio2SourceVoiceClass::xPickAndSubmitBuffer(const std::vector<std::vector<BYTE>> &vecvecbyBuffer) const {
	ASSERT(!vecvecbyBuffer.empty());

	const std::vector<BYTE> &vecbyPickedBuffer = vecvecbyBuffer[((std::size_t)std::rand()) % vecvecbyBuffer.size()];

	XAUDIO2_BUFFER Buffer;
	Buffer.Flags		= 0;
	Buffer.AudioBytes	= vecbyPickedBuffer.size();
	Buffer.pAudioData	= vecbyPickedBuffer.data();
	Buffer.PlayBegin	= 0;
	Buffer.PlayLength	= 0;
	Buffer.LoopBegin	= 0;
	Buffer.LoopLength	= 0;
	Buffer.LoopCount	= 0;
	Buffer.pContext		= (void *)(std::uintptr_t)xm_uSerialNumber;
	VERIFY(SUCCEEDED(xm_pSourceVoice->SubmitSourceBuffer(&Buffer)));
}
void WavPlayerClass::xXAudio2ManagerClass::xXAudio2SourceVoiceClass::xPumpStateAndPreSubmitBuffer(){
	switch(xm_eState){
		case ATTACKING:
			if(!xm_vecvecbyLoopData.empty()){
				xm_eNextState = LOOPING;
				break;
			}
		case LOOPING:
			if(!xm_bStopHere && !xm_vecvecbyLoopData.empty()){
				++xm_uLoopIndex;
				if((xm_uLoopCount == 0) || (xm_uLoopIndex < xm_uLoopCount)){
					xm_eNextState = LOOPING;
				} else {
					xm_eNextState = DECAYING;
				}
				break;
			} else if(!xm_vecvecbyDecayData.empty()){
				xm_eNextState = DECAYING;
				break;
			}
		case DECAYING:
		case STOPPED:
			xm_eNextState = STOPPED;
			break;
		default:
			ASSERT(false);
	}
	switch(xm_eNextState){
		case ATTACKING:
			xPickAndSubmitBuffer(xm_vecvecbyLoopData);
			break;
		case LOOPING:
			xPickAndSubmitBuffer(xm_vecvecbyLoopData);
			break;
		case DECAYING:
			xPickAndSubmitBuffer(xm_vecvecbyDecayData);
			break;
		default:
			break;
	}
}

// 其他非静态成员函数。
bool WavPlayerClass::xXAudio2ManagerClass::xXAudio2SourceVoiceClass::IsStopped() const {
	return xm_eState == STOPPED;
}
void WavPlayerClass::xXAudio2ManagerClass::xXAudio2SourceVoiceClass::Resume(){
	VERIFY(SUCCEEDED(xm_pSourceVoice->Start()));
}
void WavPlayerClass::xXAudio2ManagerClass::xXAudio2SourceVoiceClass::Pause(){
	VERIFY(SUCCEEDED(xm_pSourceVoice->Stop()));
}
void WavPlayerClass::xXAudio2ManagerClass::xXAudio2SourceVoiceClass::Stop(){
	VERIFY(SUCCEEDED(xm_pSourceVoice->FlushSourceBuffers()));

	// 这里不会有 OnBufferStart 回调，我们可以确保这些操作都是线程安全的。
	xm_bStopHere = true;
	xPumpStateAndPreSubmitBuffer();
}
