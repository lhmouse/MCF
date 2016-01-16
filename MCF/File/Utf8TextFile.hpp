// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_UTF8_TEXT_FILE_HPP_
#define MCF_CORE_UTF8_TEXT_FILE_HPP_

#include "../Core/StreamBuffer.hpp"
#include "../Core/String.hpp"
#include "../File/File.hpp"
#include <utility>

namespace MCF {

class Utf8TextFileReader  {
private:
	File x_vFile;

	std::uint64_t x_u64Offset;
	StreamBuffer x_sbufCache;

public:
	explicit Utf8TextFileReader(File &&vFile = File());
	~Utf8TextFileReader();

public:
	const File &Get() const noexcept {
		return x_vFile;
	}
	File &Get() noexcept {
		return x_vFile;
	}
	void Reset() noexcept;
	void Reset(File &&vFile);

	bool IsAtEndOfFile() const;
	int Read();
	bool Read(Utf8String &u8sData, std::size_t uCount);
	bool ReadLine(Utf8String &u8sData);
	bool ReadTillEof(Utf8String &u8sData);

	void Swap(Utf8TextFileReader &rhs) noexcept {
		using std::swap;
		swap(x_vFile,     rhs.x_vFile);
		swap(x_u64Offset, rhs.x_u64Offset);
		swap(x_sbufCache, rhs.x_sbufCache);
	}

public:
	explicit operator bool() const noexcept {
		return !IsAtEndOfFile();
	}

	friend void swap(Utf8TextFileReader &lhs, Utf8TextFileReader &rhs) noexcept {
		lhs.Swap(rhs);
	}
};

class Utf8TextFileWriter  {
public:
	enum : std::uint32_t {
		kNoBom        = 0x00000000,
		kHasBom       = 0x00000001,

		kEndlLf       = 0x00000000,
		kEndlCrLf     = 0x00000002,

		kLineBuffered = 0x00000000,
		kUnbuffered   = 0x00000004,
		kFullBuffered = 0x00000008,
	};

private:
	File x_vFile;
	std::uint32_t x_u32Flags;

	std::uint64_t x_u64Offset;
	Utf8String x_u8sLine;

public:
	explicit Utf8TextFileWriter(File &&vFile = File(), std::uint32_t u32Flags = 0);
	~Utf8TextFileWriter();

public:
	const File &Get() const noexcept {
		return x_vFile;
	}
	File &Get() noexcept {
		return x_vFile;
	}
	void Reset() noexcept;
	void Reset(File &&vFile, std::uint32_t u32Flags = 0);

	std::uint32_t GetFlags() const noexcept {
		return x_u32Flags;
	}
	std::uint32_t SetFlags(std::uint32_t u32Flags){
		if(x_u32Flags == u32Flags){
			return x_u32Flags;
		}
		Flush();
		return std::exchange(x_u32Flags, u32Flags);
	}

	void Write(char ch);
	void Write(const Utf8StringView &u8svData);
	void WriteLine(const Utf8StringView &u8svData);
	void Flush();

	void Swap(Utf8TextFileWriter &rhs) noexcept {
		using std::swap;
		swap(x_vFile,     rhs.x_vFile);
		swap(x_u32Flags,  rhs.x_u32Flags);
		swap(x_u64Offset, rhs.x_u64Offset);
		swap(x_u8sLine,   rhs.x_u8sLine);
	}

	friend void swap(Utf8TextFileWriter &lhs, Utf8TextFileWriter &rhs) noexcept {
		lhs.Swap(rhs);
	}
};

}

#endif
