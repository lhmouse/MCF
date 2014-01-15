// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2012 - 2013. LH_Mouse. All wrongs reserved.

#ifndef __MCF_X_FILE_THREAD_CLASS_HPP__
#define __MCF_X_FILE_THREAD_CLASS_HPP__

#include "StdMCF.hpp"
#include "FileClass.hpp"
#include "AbstractThreadClass.hpp"

namespace MCF {
	class FileClass::xFileThreadClass final : public AbstractThreadClass {
	private:
		static const std::size_t xBLOCK_SIZE = 4 * 1024;
	private:
		FileClass *const xm_pFile;
		const bool xm_bAsyncMode;

		bool xm_bToRead;
		void *xm_pBuffer;
		std::size_t xm_uSize;

		volatile bool xm_bCancelled;
	public:
		xFileThreadClass(FileClass *pFile, bool bAsyncMode);
		~xFileThreadClass();
	private:
		using AbstractThreadClass::Create;
		using AbstractThreadClass::Release;
	protected:
		virtual int xThreadProc();
	public:
		void BeginReading(void *pBuffer, std::size_t uSize);
		void BeginWriting(const void *pBuffer, std::size_t uSize);
		void Cancel();
	};
}

#endif
