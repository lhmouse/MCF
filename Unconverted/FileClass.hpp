// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2012 - 2013. LH_Mouse. All wrongs reserved.

#ifndef __MCF_FILE_CLASS_HPP__
#define __MCF_FILE_CLASS_HPP__

#include "StdMCF.hpp"

namespace MCF {
	class FileClass : NO_COPY_OR_ASSIGN {
	public:
		static const unsigned long long INVALID_SIZE = (unsigned long long)-1;
	private:
		class xFileThreadClass;
	private:
		xFileThreadClass *xm_pFileThread;

		HANDLE xm_hFile;
		bool xm_bModeRead;
		bool xm_bModeWrite;
		unsigned long long xm_ullFilePointerRead;
		unsigned long long xm_ullFilePointerWrite;

		mutable DWORD xm_dwErrorCode;
	public:
		FileClass(bool bAsyncMode = false);
		virtual ~FileClass();
	private:
		void xInit();
	protected:
		virtual void xOnReadBlock(DWORD dwBytesRead, DWORD dwBytesTotal);
		virtual void xOnWriteBlock(DWORD dwBytesWritten, DWORD dwBytesTotal);
	public:
		bool Open(LPCTSTR pszFilePath, bool bWantRead, bool bWantWrite, bool bCreateIfNotExist, bool bTruncateIfExistWhenWrite);
		bool OpenForReading(LPCTSTR pszFilePath);
		bool OpenForWriting(LPCTSTR pszFilePath);
		bool OpenForAppending(LPCTSTR pszFilePath);
		void Close();
		bool IsOpen() const;

		DWORD GetErrorCode() const;

		bool IsIdle() const;
		void Wait() const;
		bool WaitTimeout(DWORD dwMilliSeconds) const;

		unsigned long long GetSize() const;
		bool Resize(unsigned long long ullNewSize);

		unsigned long long TellRead() const;
		unsigned long long TellWrite() const;
		void SeekRead(unsigned long long ullPos, bool bRelative);
		void SeekWrite(unsigned long long ullPos, bool bRelative);

		bool Read(void *pBuffer, std::size_t uSize);
		bool Write(const void *pBuffer, std::size_t uSize);
		void Cancel();
	};
}

#endif
