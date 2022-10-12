#pragma once

#include <cstdint>
#include <string_view>
#include <ostream>

struct Int128 {
public:
	Int128() noexcept;

	Int128(int64_t) noexcept;

	Int128(std::string_view) noexcept;

	explicit operator int64_t() const noexcept;

	explicit operator double() const noexcept;

	std::string str() const;

	friend Int128 operator-(const Int128 &a) noexcept;

	friend Int128 operator+(const Int128 &lhs, const Int128 &rhs) noexcept;

	friend Int128 operator-(const Int128 &lhs, const Int128 &rhs) noexcept;

	friend Int128 operator*(const Int128 &lhs, const Int128 &rhs) noexcept;

	friend Int128 operator/(const Int128 &lhs, const Int128 &rhs) noexcept;

	Int128 &operator+=(const Int128 &rhs) noexcept;

	Int128 &operator-=(const Int128 &rhs) noexcept;

	Int128 &operator*=(Int128 rhs) noexcept;

	Int128 &operator/=(Int128 rhs) noexcept;

	Int128 operator<<(int) const noexcept;

	Int128 operator>>(int) const noexcept;

	friend bool operator<(const Int128 &lhs, const Int128 &rhs) noexcept;
	friend bool operator>(const Int128 &lhs, const Int128 &rhs) noexcept;
	friend bool operator<=(const Int128 &lhs, const Int128 &rhs) noexcept;
	friend bool operator>=(const Int128 &lhs, const Int128 &rhs) noexcept;
	friend bool operator==(const Int128 &lhs, const Int128 &rhs) noexcept;
	friend bool operator!=(const Int128 &lhs, const Int128 &rhs) noexcept;

	friend std::ostream &operator<<(std::ostream &, const Int128 &);

	void print() const;

private:

	friend int cmp(const Int128 &lhs, const Int128 &rhs);

	friend int cmpAbs(const Int128 &lhs, const Int128 &rhs);

	uint64_t high, low;

	explicit Int128(uint64_t, uint64_t) noexcept;

	Int128 &negate() noexcept;

	bool isNegative() const noexcept;
};