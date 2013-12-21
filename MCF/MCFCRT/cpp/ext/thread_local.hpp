// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013. LH_Mouse. All wrongs reserved.

#ifndef __MCF_CRT_THREAD_LOCAL_HPP__
#define __MCF_CRT_THREAD_LOCAL_HPP__

#include <cstddef>
#include <new>
#include <tuple>
#include <type_traits>
#include "../../env/thread.h"
#include "../../env/daemon.h"

namespace MCF {
	namespace __MCF {
		template<class OBJECT_T, class... CTOR_PARAM_T>
		class TlsWrapper {
		private:
			static volatile std::intptr_t s_nCounter;
		private:
			template<std::size_t N, std::size_t MAX_N, class... UNPACKED_T>
			struct xCtorWrapper {
				static inline void Construct(void *pMem, const std::tuple<CTOR_PARAM_T...> &Params, const UNPACKED_T &...Unpacked){
					xCtorWrapper<
						N + 1, MAX_N, UNPACKED_T..., typename std::tuple_element<N, std::tuple<CTOR_PARAM_T...>>::type
					>::Construct(pMem, Params, Unpacked..., std::get<N>(Params));
				}
			};
			template<std::size_t MAX_N, class... UNPACKED_T>
			struct xCtorWrapper<MAX_N, MAX_N, UNPACKED_T...> {
				static inline void Construct(void *pMem, const std::tuple<CTOR_PARAM_T...> &, const UNPACKED_T &...Unpacked){
					new(pMem) OBJECT_T(Unpacked...);
				}
			};
		private:
			static void xCtorJumper(void *pMem, std::intptr_t nContext){
				xCtorWrapper<0, sizeof...(CTOR_PARAM_T)>::Construct(pMem, ((const TlsWrapper *)nContext)->xm_CtorParams);
			}
			static void xDtorJumper(void *pMem){
				((OBJECT_T *)pMem)->~OBJECT_T();
			}
		private:
			const std::intptr_t xm_nUniqueID;
			const std::tuple<CTOR_PARAM_T...> xm_CtorParams;
		public:
			TlsWrapper(CTOR_PARAM_T ...Params) :
				xm_nUniqueID(__sync_add_and_fetch(&s_nCounter, 1)),
				xm_CtorParams(std::move(Params)...)
			{
			}
			~TlsWrapper(){ ::__MCF_CRTDeleteTls(xm_nUniqueID); }

			TlsWrapper(const TlsWrapper &) = delete;
			void operator=(const TlsWrapper &) = delete;
		public:
			OBJECT_T *Get() const {
				const auto pRet = (OBJECT_T *)::__MCF_CRTRetrieveTls(
					xm_nUniqueID,
					sizeof(OBJECT_T),
					&xCtorJumper,
					(std::intptr_t)this,
					&xDtorJumper
				);
				if(pRet == nullptr){
					::__MCF_Bail(
						L"__MCF_CRTRetrieveTls() 返回了一个空指针。\n"
						"如果这不是由于系统内存不足造成的，请确保不要在静态对象的构造函数或析构函数中访问 TLS。"
					);
				}
				return pRet;
			}
			void Release() const {
				::__MCF_CRTDeleteTls(xm_nUniqueID);
			}
		public:
			OBJECT_T *operator->() const {
				return Get();
			}
			operator OBJECT_T *() const {
				return Get();
			}
		};

		template<class OBJECT_T, class... CTOR_PARAM_T>
		volatile std::intptr_t TlsWrapper<OBJECT_T, CTOR_PARAM_T...>::s_nCounter = 0;

		template<class OBJECT_T, class... CTOR_PARAM_T>
		auto TlsWrapperTypeHelper(const CTOR_PARAM_T &...)
			-> TlsWrapper<OBJECT_T, typename std::decay<const CTOR_PARAM_T>::type...>;
	}
}

#define THREAD_LOCAL(type, id, ...)		decltype(::MCF::__MCF::TlsWrapperTypeHelper<type>(__VA_ARGS__)) id{__VA_ARGS__}

#endif
