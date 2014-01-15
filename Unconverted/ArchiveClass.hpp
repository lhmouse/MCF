// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2012 - 2013. LH_Mouse. All wrongs reserved.

#ifndef __MCF_ARCHIVE_CLASS_HPP__
#define __MCF_ARCHIVE_CLASS_HPP__

#include "StdMCF.hpp"
#include <string>
#include <vector>

namespace MCF {
	class InArchiveClass;
	class OutArchiveClass;

	class xArchiveBaseClass {
	public:
		class ArchivableBaseClass {
		public:
			void Serialize(OutArchiveClass &dst) const throw() {
				UNREF_PARAM(dst);
			}
			void Unserialize(InArchiveClass &src){
				UNREF_PARAM(src);
			}
		};
	protected:
		template<typename T>
		struct xSTL_STYLE_CHECKER {
			template<typename U>
			static int (*CheckerHelper(
				void *,
				typename U::iterator * = nullptr,
				typename U::const_iterator * = nullptr,
				typename U::value_type * = nullptr,
				decltype(((U *)nullptr)->begin()) * = nullptr,
				decltype(((U *)nullptr)->end()) * = nullptr,
				decltype(((U *)nullptr)->clear()) * = nullptr,
				decltype(((U *)nullptr)->size()) * = nullptr,
				decltype(((U *)nullptr)->emplace_back(U::value_type())) * = nullptr
			))[1];
			template<typename U>
			static int (*CheckerHelper(...))[2];

			static const bool value = (sizeof(*CheckerHelper<T>(nullptr)) == sizeof(int[1]));
		};

		template<
			typename T,
			bool xIS_TRIVIAL = std::is_trivial<T>::value,
			bool xIS_DERIVED = std::is_base_of<ArchivableBaseClass, T>::value,
			bool xIS_STL_STYLE = xSTL_STYLE_CHECKER<T>::value
		> struct xArchiver;
		template<
			typename T,
			bool xIS_TRIVIAL = std::is_trivial<T>::value,
			bool xIS_DERIVED = std::is_base_of<ArchivableBaseClass, T>::value,
			bool xIS_STL_STYLE = xSTL_STYLE_CHECKER<T>::value
		> struct xArrayArchiver;
	private:
		std::vector<BYTE> xm_vecbyData;
	public:
		xArchiveBaseClass(){
			xInit();
		}
		xArchiveBaseClass(const std::vector<BYTE> &src) : xm_vecbyData(src) {
			xInit();
		}
		xArchiveBaseClass(std::vector<BYTE> &&src) : xm_vecbyData(std::move(src)) {
			xInit();
		}
		~xArchiveBaseClass(){
		}
	private:
		void xInit(){
			xm_vecbyData.reserve(0x10);
		}
	public:
		void Push(const void *pData, std::size_t uCountBytes){
			const std::size_t uOldSize = xm_vecbyData.size();
			Resize(uOldSize + uCountBytes);
			std::memcpy(xm_vecbyData.data() + uOldSize, pData, uCountBytes);
		}
		void Clear(){
			xm_vecbyData.clear();
		}
		void Resize(std::size_t uNewSize){
			const std::size_t uOldCap = xm_vecbyData.capacity();
			if(uOldCap < uNewSize){
				xm_vecbyData.reserve((uOldCap * 3 / 2 + 0x0F) & -0x10);
			}
			xm_vecbyData.resize(uNewSize);
		}

		const BYTE *GetDataPtr() const {
			return xm_vecbyData.data();
		}
		BYTE *GetDataPtr(){
			return xm_vecbyData.data();
		}
		std::size_t GetDataSize() const {
			return xm_vecbyData.size();
		}
	};

	class InArchiveClass final : protected xArchiveBaseClass {
	public:
		class BAD_STREAM { };
	private:
		const BYTE *xm_pbyReadPtr;
	public:
		InArchiveClass(){
			xm_pbyReadPtr = nullptr;
		}
		InArchiveClass(const void *pData, std::size_t uCountBytes){
			xm_pbyReadPtr = nullptr;
			Load(pData, uCountBytes);
		}
		InArchiveClass(const InArchiveClass &src) : xArchiveBaseClass(src) {
			xm_pbyReadPtr = src.xm_pbyReadPtr - src.GetDataPtr() + GetDataPtr();
		}
		InArchiveClass(InArchiveClass &&src) : xArchiveBaseClass(std::move(src)) {
			xm_pbyReadPtr = src.xm_pbyReadPtr - src.GetDataPtr() + GetDataPtr();
		}
		InArchiveClass(const OutArchiveClass &src) : xArchiveBaseClass((const xArchiveBaseClass &)src) {
			Rewind();
		}
		InArchiveClass(OutArchiveClass &&src) : xArchiveBaseClass(std::move((xArchiveBaseClass &)src)) {
			Rewind();
		}
		InArchiveClass &operator=(const InArchiveClass &src){
			this->~InArchiveClass();
			new(this) InArchiveClass(src);
			return *this;
		}
		InArchiveClass &operator=(InArchiveClass &&src){
			this->~InArchiveClass();
			new(this) InArchiveClass(std::move(src));
			return *this;
		}
		InArchiveClass &operator=(const OutArchiveClass &src){
			this->~InArchiveClass();
			new(this) InArchiveClass(src);
			return *this;
		}
		InArchiveClass &operator=(OutArchiveClass &&src){
			this->~InArchiveClass();
			new(this) InArchiveClass(std::move(src));
			return *this;
		}
		~InArchiveClass(){
		}
	public:
		using xArchiveBaseClass::GetDataPtr;
		using xArchiveBaseClass::GetDataSize;
	public:
		void Rewind(){
			xm_pbyReadPtr = GetDataPtr();
		}
		void Clear(){
			xArchiveBaseClass::Clear();
			Rewind();
		}
		void Resize(std::size_t uNewSize){
			xArchiveBaseClass::Resize(uNewSize);
			Rewind();
		}
		void Pop(void *pDst, std::size_t uCountBytes){
			if(xm_pbyReadPtr + uCountBytes > GetDataPtr() + GetDataSize()){
				throw BAD_STREAM();
			}
			std::memcpy(pDst, xm_pbyReadPtr, uCountBytes);
			xm_pbyReadPtr += uCountBytes;
		}
		void Load(const void *pData, std::size_t uCountBytes){
			xArchiveBaseClass::Clear();
			xArchiveBaseClass::Push(pData, uCountBytes);
			Rewind();
		}

		template<typename T>
		InArchiveClass &Extract(T &dst){
			xArchiver<T>::Unserialize(dst, *this);
			return *this;
		}
		template<typename T>
		InArchiveClass &ExtractArray(T *pDst, std::size_t uCount){
			xArrayArchiver<T>::UnserializeArray(pDst, uCount, *this);
			return *this;
		}
	public:
		template<typename T>
		InArchiveClass &operator>>(T &dst){
			return Extract<T>(dst);
		}
		template<typename T, std::size_t N>
		InArchiveClass &operator>>(T (&dst)[N]){
			return ExtractArray<T>(dst, N);
		}
	};
	class OutArchiveClass : protected xArchiveBaseClass {
	public:
		OutArchiveClass(){
		}
		OutArchiveClass(const OutArchiveClass &src) : xArchiveBaseClass(src) {
		}
		OutArchiveClass(OutArchiveClass &&src) : xArchiveBaseClass(std::move(src)) {
		}
		OutArchiveClass(const InArchiveClass &src) : xArchiveBaseClass((const xArchiveBaseClass &)src) {
		}
		OutArchiveClass(InArchiveClass &&src) : xArchiveBaseClass(std::move((xArchiveBaseClass &)src)) {
		}
		OutArchiveClass &operator=(const OutArchiveClass &src){
			this->~OutArchiveClass();
			new(this) OutArchiveClass(src);
			return *this;
		}
		OutArchiveClass &operator=(OutArchiveClass &&src){
			this->~OutArchiveClass();
			new(this) OutArchiveClass(std::move(src));
			return *this;
		}
		OutArchiveClass &operator=(const InArchiveClass &src){
			this->~OutArchiveClass();
			new(this) OutArchiveClass(src);
			return *this;
		}
		OutArchiveClass &operator=(InArchiveClass &&src){
			this->~OutArchiveClass();
			new(this) OutArchiveClass(std::move(src));
			return *this;
		}
		~OutArchiveClass(){
		}
	public:
		using xArchiveBaseClass::Push;
		using xArchiveBaseClass::Clear;
		using xArchiveBaseClass::GetDataPtr;
		using xArchiveBaseClass::GetDataSize;
	public:
		template<typename T>
		OutArchiveClass &Insert(const T &src) throw() {
			xArchiver<T>::Serialize(*this, src);
			return *this;
		}
		template<typename T>
		OutArchiveClass &InsertArray(const T *pSrc, std::size_t uCount) throw() {
			xArrayArchiver<T>::SerializeArray(*this, pSrc, uCount);
			return *this;
		}
	public:
		template<typename T>
		OutArchiveClass &operator<<(const T &src) throw() {
			return Insert<T>(src);
		}
		template<typename T, std::size_t N>
		OutArchiveClass &operator<<(const T (&src)[N]) throw() {
			return InsertArray<T>(src, N);
		}
	};

	// 通用的 serializer-unserializer 模板定义。
	// 注意成员函数没有定义，如果需要，请自行定义。
	template<typename T, bool xIS_TRIVIAL, bool xIS_DERIVED, bool xIS_STL_STYLE>
	struct xArchiveBaseClass::xArchiver abstract {
		static void Serialize(OutArchiveClass &dst, const T &src) throw();
		static void Unserialize(T &dst, InArchiveClass &src);
	};
	template<typename T, bool xIS_TRIVIAL, bool xIS_DERIVED, bool xIS_STL_STYLE>
	struct xArchiveBaseClass::xArrayArchiver abstract {
		static void SerializeArray(OutArchiveClass &dst, const T *pSrc, std::size_t uCount) throw() {
			for(std::size_t i = 0; i < uCount; ++i){
				xArchiver<T>::Serialize(dst, pSrc[i]);
			}
		}
		static void UnserializeArray(T *pDst, std::size_t uCount, InArchiveClass &src){
			for(std::size_t i = 0; i < uCount; ++i){
				xArchiver<T>::Unserialize(pDst[i], src);
			}
		}
	};
	// 为 trivial 类型特化的模板。
	// 对于 trivial 类型，简单位拷贝即可。
	template<typename T, bool xIS_DERIVED, bool xIS_STL_STYLE>
	struct xArchiveBaseClass::xArchiver<T, true, xIS_DERIVED, xIS_STL_STYLE> abstract {
		static void Serialize(OutArchiveClass &dst, const T &src) throw() {
			dst.Push(&src, sizeof(T));
		}
		static void Unserialize(T &dst, InArchiveClass &src){
			src.Pop(&dst, sizeof(T));
		}
	};
	template<typename T, bool xIS_DERIVED, bool xIS_STL_STYLE>
	struct xArchiveBaseClass::xArrayArchiver<T, true, xIS_DERIVED, xIS_STL_STYLE> abstract {
		static void SerializeArray(OutArchiveClass &dst, const T *pSrc, std::size_t uCount) throw() {
			if(uCount != 0){
				dst.Push(pSrc, uCount * sizeof(T));
			}
		}
		static void UnserializeArray(T *pDst, std::size_t uCount, InArchiveClass &src){
			if(uCount != 0){
				src.Pop(pDst, uCount * sizeof(T));
			}
		}
	};
	// 对于具有成员函数 Serialize() 和 Unserialzie() 的类，调用之。
	template<typename T>
	struct xArchiveBaseClass::xArchiver<T, false, true, false> abstract {
		static void Serialize(OutArchiveClass &dst, const T &src) throw() {
			src.Serialize(dst);
		}
		static void Unserialize(T &dst, InArchiveClass &src){
			dst.Unserialize(src);
		}
	};
	// 部分 STL 以及标准库类特化模板。
	// 要求：
	// 		必须是可双向迭代的序列容器（例如 map 和 forward_list 不允许。如果要对这些容器进行操作可以先拷贝进 vector 或 list 再进行操作）。
	//		必须拥有 iterator 和 const_iterator，
	//		必须拥有 value_type，
	//		必须支持 begin() 和 end()，
	//		必须支持 clear() 和 size()，
	//		必须支持 emplace_back()。
	template<typename T>
	struct xArchiveBaseClass::xArchiver<T, false, false, true> abstract {
		static void Serialize(OutArchiveClass &dst, const T &src) throw() {
			dst <<(DWORD)src.size();
			for(auto iter = src.cbegin(); iter != src.cend(); ++iter){
				dst <<*iter;
			}
		}
		static void Unserialize(T &dst, InArchiveClass &src){
			dst.clear();

			T::value_type Element;
			DWORD dwCount;
			src >>dwCount;
			for(DWORD i = 0; i < dwCount; ++i){
				src >>Element;
				dst.emplace_back(std::move(Element));
			}
		}
	};
	template<typename T>
	struct xArchiveBaseClass::xArchiver<std::vector<T>, false, false, true> abstract {
		static void Serialize(OutArchiveClass &dst, const std::vector<T> &src) throw() {
			dst <<(DWORD)src.size();
			xArrayArchiver<T>::SerializeArray(dst, src.data(), src.size());
		}
		static void Unserialize(std::vector<T> &dst, InArchiveClass &src){
			DWORD dwCount;
			src >>dwCount;
			dst.resize(dwCount);
			xArrayArchiver<T>::UnserializeArray(dst.data(), dwCount, src);
		}
	};
	template<typename T>
	struct xArchiveBaseClass::xArchiver<std::basic_string<T>, false, false, true> abstract {
		static void Serialize(OutArchiveClass &dst, const std::basic_string<T> &src) throw() {
			dst <<(DWORD)src.size();
			xArrayArchiver<T>::SerializeArray(dst, &*src.begin(), src.size());
		}
		static void Unserialize(std::basic_string<T> &dst, InArchiveClass &src){
			DWORD dwCount;
			src >>dwCount;
			dst.resize(dwCount);
			xArrayArchiver<T>::UnserializeArray(&*dst.begin(), dwCount, src);
		}
	};
	template<typename T1, typename T2>
	struct xArchiveBaseClass::xArchiver<std::pair<T1, T2>, false, false, false> abstract {
		static void Serialize(OutArchiveClass &dst, const std::pair<T1, T2> &src) throw() {
			dst <<src.first <<src.second;
		}
		static void Unserialize(std::pair<T1, T2> &dst, InArchiveClass &src){
			src >>dst.first >>dst.second;
		}
	};

	typedef xArchiveBaseClass::ArchivableBaseClass ArchivableBaseClass;

	// MCF 标准字符串类序列化/反序列化器的实例化。
	extern template struct xArchiveBaseClass::xArchiver<MBString>;
	extern template struct xArchiveBaseClass::xArchiver<WCString>;
	extern template struct xArchiveBaseClass::xArchiver<UTF8String>;
}

#endif
