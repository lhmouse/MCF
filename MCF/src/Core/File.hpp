// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_FILE_HPP_
#define MCF_CORE_FILE_HPP_

#include "_KernelObjectBase.hpp"
#include "StringView.hpp"
#include <cstddef>
#include <cstdint>

namespace MCF {

class File : public Impl_KernelObjectBase::KernelObjectBase {
public:
	enum : std::uint32_t {
		// 继承自 KernelObjectBase。
		// kDontCreate    = 0x00000002, // 文件不存在则失败。若指定该选项则 kFailIfExists 无效。
		// kFailIfExists  = 0x00000004, // 文件已存在则失败。

		// 权限控制。
		kToRead           = 0x00000100,
		kToWrite          = 0x00000200,

		// 共享访问权限。
		kSharedRead       = 0x00001000, // 共享读权限。对于不带 kToWrite 打开的文件总是开启的。
		kSharedWrite      = 0x00002000, // 共享写权限。
		kSharedDelete     = 0x00004000, // 共享删除权限。

		// 杂项。
		kNoBuffering      = 0x00010000,
		kWriteThrough     = 0x00020000,
		kDeleteOnClose    = 0x00040000,
		kDontTruncate     = 0x00080000, // 默认情况下使用 kToWrite 打开文件会清空现有内容。
	};

private:
	Impl_UniqueNtHandle::UniqueNtHandle x_hFile;

public:
	constexpr File() noexcept {
	}
	File(const WideStringView &wsvPath, std::uint32_t u32Flags);

public:
	Handle GetHandle() const noexcept {
		return x_hFile.Get();
	}

	bool IsOpen() const noexcept {
		return !!x_hFile;
	}
	void Open(const WideStringView &wsvPath, std::uint32_t u32Flags);
	bool OpenNothrow(const WideStringView &wsvPath, std::uint32_t u32Flags);
	void Close() noexcept;

	std::uint64_t GetSize() const;
	void Resize(std::uint64_t u64NewSize);
	void Clear();

	std::size_t Read(void *pBuffer, std::size_t uBytesToRead, std::uint64_t u64Offset) const;
	std::size_t Write(std::uint64_t u64Offset, const void *pBuffer, std::size_t uBytesToWrite);
	void Flush();

	void Swap(File &rhs) noexcept {
		using std::swap;
		swap(x_hFile, rhs.x_hFile);
	}

public:
	explicit operator bool() const noexcept {
		return IsOpen();
	}

	friend void swap(File &lhs, File &rhs) noexcept {
		lhs.Swap(rhs);
	}
};

}

#endif
