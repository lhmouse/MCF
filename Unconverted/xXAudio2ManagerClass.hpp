// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2012 - 2013. LH_Mouse. All wrongs reserved.

#ifndef __MCF_X_XAUDIO2_MANAGER_CLASS_HPP__
#define __MCF_X_XAUDIO2_MANAGER_CLASS_HPP__

#include "StdMCF.hpp"
#include "WavPlayerClass.hpp"
#include <xaudio2.h>
#include <vector>
#include <list>
#include <map>
#include "Memory.hpp"

namespace MCF {
	class WavPlayerClass::xXAudio2ManagerClass final : NO_COPY_OR_ASSIGN {
	private:
		class xXAudio2SourceVoiceClass;
	private:
		WavPlayerClass *const xm_pOwner;

		IXAudio2 *xm_pXAudio2;
		IXAudio2MasteringVoice *xm_pMasteringVoice;
		std::map<std::size_t, std::unique_ptr<xXAudio2SourceVoiceClass>> xm_mapSourceVoices;
		std::size_t xm_uLastID;
	public:
		xXAudio2ManagerClass(WavPlayerClass *pOwner);
		virtual ~xXAudio2ManagerClass();
	private:
		void xFlush();
	public:
		std::size_t Start(
			const WAVEFORMATEX *pWaveFormatEx,
			std::vector<std::vector<BYTE>> &&vecvecbyAttackData,
			std::vector<std::vector<BYTE>> &&vecvecbyLoopData,
			std::vector<std::vector<BYTE>> &&vecvecbyDecayData,
			std::size_t uLoopCount,
			bool bSuspended
		);
		void Stop(std::size_t uID);
		void StopAll();
		void Clear(std::size_t uID);
		void ClearAll();

		void Resume(std::size_t uID);
		void ResumeAll();
		void Pause(std::size_t uID);
		void PauseAll();

		// 0.0 ~ 1.0
		double GetVolume() const;
		void SetVolume(double lfNewVolume);
	};
}

#endif
