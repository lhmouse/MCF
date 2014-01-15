// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2012 - 2013. LH_Mouse. All wrongs reserved.

#ifndef __MCF_X_XAUDIO2_SOURCE_VOICE_CLASS_HPP__
#define __MCF_X_XAUDIO2_SOURCE_VOICE_CLASS_HPP__

#include "StdMCF.hpp"
#include "xXAudio2ManagerClass.hpp"
#include <xaudio2.h>
#include <vector>
#include <map>
#include "Memory.hpp"

namespace MCF {
	class WavPlayerClass::xXAudio2ManagerClass::xXAudio2SourceVoiceClass final : NO_COPY_OR_ASSIGN {
	private:
		typedef enum {
			ATTACKING,
			LOOPING,
			DECAYING,
			STOPPED
		} xSTATE;

		struct xVoiceCallback : public IXAudio2VoiceCallback {
			xXAudio2SourceVoiceClass *m_pOwner;

			virtual void __stdcall OnVoiceProcessingPassStart(UINT32 uSamplesRequired) override {
				UNREF_PARAM(uSamplesRequired);
			}
			virtual void __stdcall OnVoiceProcessingPassEnd() override {
			}
			virtual void __stdcall OnBufferStart(void *pBufferContext) override {
				m_pOwner->xOnBufferStart((std::uintptr_t)pBufferContext);
			}
			virtual void __stdcall OnBufferEnd(void *pBufferContext) override {
				m_pOwner->xOnBufferEnd((std::uintptr_t)pBufferContext);
			}
			virtual void __stdcall OnLoopEnd(void *pBufferContext) override {
				UNREF_PARAM(pBufferContext);
			}
			virtual void __stdcall OnVoiceError(void *pBufferContext, HRESULT hrError) override {
				UNREF_PARAM(hrError);

				m_pOwner->xOnBufferEnd((std::uintptr_t)pBufferContext);
			}
			virtual void __stdcall OnStreamEnd() override {
			}
		};
	private:
		xXAudio2ManagerClass *const xm_pOwner;
		const std::size_t xm_uID;
		const std::size_t xm_uLoopCount;

		xVoiceCallback xm_VoiceCallback;
		IXAudio2SourceVoice *xm_pSourceVoice;
		std::vector<std::vector<BYTE>> xm_vecvecbyAttackData;
		std::vector<std::vector<BYTE>> xm_vecvecbyLoopData;
		std::vector<std::vector<BYTE>> xm_vecvecbyDecayData;
		std::size_t xm_uSerialNumber;
		std::size_t xm_uLoopIndex;
		xSTATE xm_eState;
		xSTATE xm_eNextState;
		bool xm_bStopHere;
		std::uintptr_t xm_uExpectingEndOf;
	public:
		xXAudio2SourceVoiceClass(
			xXAudio2ManagerClass *pOwner,
			std::size_t uID,
			const WAVEFORMATEX *pWaveFormatEx,
			std::vector<std::vector<BYTE>> &&vecvecbyAttackData,
			std::vector<std::vector<BYTE>> &&vecvecbyLoopData,
			std::vector<std::vector<BYTE>> &&vecvecbyDecayData,
			std::size_t uLoopFileCount,
			bool bSuspended
		);
		virtual ~xXAudio2SourceVoiceClass();
	private:
		void xOnBufferStart(std::uintptr_t uContext);
		void xOnBufferEnd(std::uintptr_t uContext);

		void xPickAndSubmitBuffer(const std::vector<std::vector<BYTE>> &vecvecbyBuffer) const;
		void xPumpStateAndPreSubmitBuffer();
	public:
		bool IsStopped() const;
		void Stop();
		void Resume();
		void Pause();
	};
}

#endif
