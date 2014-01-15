// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2012 - 2013. LH_Mouse. All wrongs reserved.

#ifndef __MCF_WAV_PLAYER_CLASS_HPP__
#define __MCF_WAV_PLAYER_CLASS_HPP__

#include "StdMCF.hpp"
#include <vector>

namespace MCF {
	class WavPlayerClass : NO_COPY_OR_ASSIGN {
	private:
		class xXAudio2ManagerClass;
	private:
		static void xParseFile(std::vector<BYTE> &vecbyFmt, std::vector<BYTE> &vecbyData, const void *pFile, std::size_t uSize);
	private:
		xXAudio2ManagerClass *xm_pXAudio2Manager;
	public:
		WavPlayerClass();
		virtual ~WavPlayerClass();
	protected:
		virtual void xPrePlayBlock(std::size_t uID, std::size_t uSerialNumber);
		virtual void xPostPlayBlock(std::size_t uID, std::size_t uSerialNumber, bool bIsEnd);
	public:
		std::size_t Start(
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
