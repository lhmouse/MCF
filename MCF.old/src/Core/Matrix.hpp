// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_MATRIX_HPP_
#define MCF_CORE_MATRIX_HPP_

#include "Array.hpp"
#include <cstddef>

namespace MCF {

template<typename ElementT, std::size_t kRowsT, std::size_t kColumnsT>
class Matrix {
public:
	using Element = ElementT;

	enum : std::size_t {
		kRows    = kRowsT,
		kColumns = kColumnsT,
	};

public:
	Array<Element, kRows, kColumns> m_a;

public:
	const Array<Element, kColumns> &Get(std::size_t uRow) const {
		return m_a.Get(uRow);
	}
	Array<Element, kColumns> &Get(std::size_t uRow){
		return m_a.Get(uRow);
	}
	const Array<Element, kColumns> &UncheckedGet(std::size_t uRow) const noexcept {
		return m_a.UncheckedGet(uRow);
	}
	Array<Element, kColumns> &UncheckedGet(std::size_t uRow) noexcept {
		return m_a.UncheckedGet(uRow);
	}

public:
	const Array<Element, kColumns> &operator[](std::size_t uRow) const noexcept {
		return m_a[uRow];
	}
	Array<Element, kColumns> &operator[](std::size_t uRow) noexcept {
		return m_a[uRow];
	}

	Matrix operator+() const {
		Matrix vResult;
#pragma GCC ivdep
		for(std::size_t uRow = 0; uRow < kRows; ++uRow){
#pragma GCC ivdep
			for(std::size_t uColumn = 0; uColumn < kColumns; ++uColumn){
				vResult.m_a[uRow][uColumn] = +(m_a[uRow][uColumn]);
			}
		}
		return vResult;
	}
	Matrix operator-() const {
		Matrix vResult;
#pragma GCC ivdep
		for(std::size_t uRow = 0; uRow < kRows; ++uRow){
#pragma GCC ivdep
			for(std::size_t uColumn = 0; uColumn < kColumns; ++uColumn){
				vResult.m_a[uRow][uColumn] = -(m_a[uRow][uColumn]);
			}
		}
		return vResult;
	}

	Matrix &operator+=(const Matrix &vOther){
#pragma GCC ivdep
		for(std::size_t uRow = 0; uRow < kRows; ++uRow){
#pragma GCC ivdep
			for(std::size_t uColumn = 0; uColumn < kColumns; ++uColumn){
				m_a[uRow][uColumn] += vOther.m_a[uRow][uColumn];
			}
		}
		return *this;
	}
	Matrix &operator-=(const Matrix &vOther){
#pragma GCC ivdep
		for(std::size_t uRow = 0; uRow < kRows; ++uRow){
#pragma GCC ivdep
			for(std::size_t uColumn = 0; uColumn < kColumns; ++uColumn){
				m_a[uRow][uColumn] -= vOther.m_a[uRow][uColumn];
			}
		}
		return *this;
	}

	Matrix &operator*=(const Element &vOther){
#pragma GCC ivdep
		for(std::size_t uRow = 0; uRow < kRows; ++uRow){
#pragma GCC ivdep
			for(std::size_t uColumn = 0; uColumn < kColumns; ++uColumn){
				m_a[uRow][uColumn] *= vOther;
			}
		}
		return *this;
	}
	Matrix &operator/=(const Element &vOther){
#pragma GCC ivdep
		for(std::size_t uRow = 0; uRow < kRows; ++uRow){
#pragma GCC ivdep
			for(std::size_t uColumn = 0; uColumn < kColumns; ++uColumn){
				m_a[uRow][uColumn] /= vOther;
			}
		}
		return *this;
	}

	Matrix operator+(const Matrix &vOther) const {
		auto vResult = *this;
		vResult += vOther;
		return vResult;
	}
	Matrix operator-(const Matrix &vOther) const {
		auto vResult = *this;
		vResult -= vOther;
		return vResult;
	}

	Matrix operator*(const Element &vOther) const {
		auto vResult = *this;
		vResult *= vOther;
		return vResult;
	}
	Matrix operator/(const Element &vOther) const {
		auto vResult = *this;
		vResult /= vOther;
		return vResult;
	}

	template<std::size_t kOtherColumnsT>
	Matrix<ElementT, kRowsT, kOtherColumnsT> operator*(const Matrix<ElementT, kColumnsT, kOtherColumnsT> &vOther){
		auto vResult = Matrix<ElementT, kRowsT, kOtherColumnsT>();
#pragma GCC ivdep
		for(std::size_t uRow = 0; uRow < kRowsT; ++uRow){
#pragma GCC ivdep
			for(std::size_t uColumn = 0; uColumn < kOtherColumnsT; ++uColumn){
#pragma GCC ivdep
				for(std::size_t uIndex = 0; uIndex < kColumnsT; ++uIndex){
					vResult[uRow][uColumn] += m_a[uRow][uIndex] * vOther[uIndex][uColumn];
				}
			}
		}
		return vResult;
	}
};

}

#endif
