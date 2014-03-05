// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef __MCF_TEXT_FILE_STREAM_HPP__
#define __MCF_TEXT_FILE_STREAM_HPP__

#include "../Core/File.hpp"
#include "../Core/String.hpp"
#include <cstddef>
#include <cstdint>

namespace MCF {

class TextFileStream {
public:
	typedef enum {
		ANSI,
		UTF8_NOBOM,
		UTF8,
		UTF16LE,
		UTF16BE
	} ENCODING;
private:
	File xm_vFile;
	std::uint64_t xm_u64CurrentPos;
public:
	TextFileStream() noexcept;
	TextFileStream(const wchar_t *pwszPath, bool bToRead, bool bToWrite, bool bAutoCreate);
	~TextFileStream();
public:
	bool IsOpen() const noexcept;
	bool Open(const wchar_t *pwszPath, bool bToRead, bool bToWrite, bool bAutoCreate);
	void Close() noexcept;

	UTF16String GetContents();
	void PutContents(const wchar_t *pwszData, std::size_t uSize = (std::size_t)-1);
	void PutContents(const UTF16String &u16sData);

	wchar_t Get();
	void Put(wchar_t wch);

	UTF16String GetLine();
	void PutLine(const wchar_t *pwszData, std::size_t uSize = (std::size_t)-1);
	void PutLine(const UTF16String &u16sData);

	void Flush();
public:
	explicit operator bool() const noexcept {
		return IsOpen();
	}
};

}

#endif
