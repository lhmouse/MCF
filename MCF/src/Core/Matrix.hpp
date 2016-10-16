// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

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
		Matrix ret;
#pragma GCC ivdep
		for(std::size_t r = 0; r < kRows; ++r){
#pragma GCC ivdep
			for(std::size_t c = 0; c < kColumns; ++c){
				ret.m_a[r][c] = +(m_a[r][c]);
			}
		}
		return ret;
	}
	Matrix operator-() const {
		Matrix ret;
#pragma GCC ivdep
		for(std::size_t r = 0; r < kRows; ++r){
#pragma GCC ivdep
			for(std::size_t c = 0; c < kColumns; ++c){
				ret.m_a[r][c] = -(m_a[r][c]);
			}
		}
		return ret;
	}

	Matrix &operator+=(const Matrix &rhs){
#pragma GCC ivdep
		for(std::size_t r = 0; r < kRows; ++r){
#pragma GCC ivdep
			for(std::size_t c = 0; c < kColumns; ++c){
				m_a[r][c] += rhs.m_a[r][c];
			}
		}
		return *this;
	}
	Matrix &operator-=(const Matrix &rhs){
#pragma GCC ivdep
		for(std::size_t r = 0; r < kRows; ++r){
#pragma GCC ivdep
			for(std::size_t c = 0; c < kColumns; ++c){
				m_a[r][c] -= rhs.m_a[r][c];
			}
		}
		return *this;
	}

	Matrix &operator*=(const Element &rhs){
#pragma GCC ivdep
		for(std::size_t r = 0; r < kRows; ++r){
#pragma GCC ivdep
			for(std::size_t c = 0; c < kColumns; ++c){
				m_a[r][c] *= rhs;
			}
		}
		return *this;
	}
	Matrix &operator/=(const Element &rhs){
#pragma GCC ivdep
		for(std::size_t r = 0; r < kRows; ++r){
#pragma GCC ivdep
			for(std::size_t c = 0; c < kColumns; ++c){
				m_a[r][c] /= rhs;
			}
		}
		return *this;
	}

	Matrix operator+(const Matrix &rhs) const {
		auto ret = *this;
		ret += rhs;
		return ret;
	}
	Matrix operator-(const Matrix &rhs) const {
		auto ret = *this;
		ret -= rhs;
		return ret;
	}

	Matrix operator*(const Element &rhs) const {
		auto ret = *this;
		ret *= rhs;
		return ret;
	}
	Matrix operator/(const Element &rhs) const {
		auto ret = *this;
		ret /= rhs;
		return ret;
	}

	template<std::size_t kOtherColumnsT>
	Matrix<ElementT, kRowsT, kOtherColumnsT> operator*(const Matrix<ElementT, kColumnsT, kOtherColumnsT> &rhs){
		auto ret = Matrix<ElementT, kRowsT, kOtherColumnsT>();
#pragma GCC ivdep
		for(std::size_t r = 0; r < kRowsT; ++r){
#pragma GCC ivdep
			for(std::size_t c = 0; c < kOtherColumnsT; ++c){
#pragma GCC ivdep
				for(std::size_t i = 0; i < kColumnsT; ++i){
					ret[r][c] += m_a[r][i] * rhs[i][c];
				}
			}
		}
		return ret;
	}
};

}

#endif
