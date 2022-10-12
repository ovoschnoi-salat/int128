#include "int128.h"

#include <iostream>
#include <cstdlib>

Int128::Int128() noexcept: high{0}, low{0} {
}

Int128::Int128(int64_t a) noexcept: high(0), low(a) {
	if (a < 0)
		high = UINT64_MAX;
}

Int128::Int128(std::string_view stringView) noexcept: high(0), low(0) {
	unsigned char str[50];
	size_t size = stringView.size();
	if (size > 40)
		return;
	memcpy(&str, stringView.data(), size);
	size_t start = 0;
	bool isNegative = str[0] == '-';
	if (isNegative) {
		start = 1;
	}
	if (start < size && str[start] == '0') {
		return;
	}
	for (size_t i = start; i < size; ++i) {
		if (str[i] < '0' || str[i] > '9') {
			return;
		}
		str[i] -= '0';
	}
	uint64_t carry;
	size_t bit = 0;
	while (start < size && bit < 128) {
		carry = 0;
		for (size_t i = start; i < size; i++) {
			unsigned char a = str[i] + carry * 10;
			carry = a % 2;
			str[i] = a / 2;
		}
		if (bit < 64)
			low = low | (carry << bit);
		else
			high = high | (carry << (bit % 64));
		while (str[start] == 0 && start < size)
			start++;
		bit++;
	}
	if (isNegative)
		negate();
}

Int128::operator int64_t() const noexcept {
	return low;
}

Int128::operator double() const noexcept {
	Int128 tmp(*this);
	if (isNegative())
		tmp.negate();
	double d = static_cast<double>(tmp.low + tmp.high * 18446744073709551616.0);
	return isNegative() ? -d : d;
}

Int128 operator+(const Int128 &lhs, const Int128 &rhs) noexcept {
	return Int128(lhs) += rhs;
}

Int128 operator-(const Int128 &lhs, const Int128 &rhs) noexcept {
	return Int128(lhs) -= rhs;
}

Int128 operator*(const Int128 &lhs, const Int128 &rhs) noexcept {
	return Int128(lhs) *= rhs;
}

Int128 operator/(const Int128 &lhs, const Int128 &rhs) noexcept {
	return Int128(lhs) /= rhs;
}

Int128 &Int128::operator+=(const Int128 &rhs) noexcept {
	if (low > UINT64_MAX - rhs.low) {
		high += 1;
	}
	high += rhs.high;
	low += rhs.low;
	return *this;
}

Int128 &Int128::operator-=(const Int128 &rhs) noexcept {
	return *this += -rhs;
}

Int128 &Int128::operator*=(Int128 rhs) noexcept {
	bool negative = isNegative() ^ rhs.isNegative();
	if (isNegative())
		negate();
	if (rhs.isNegative())
		rhs.negate();
	uint64_t lhs0 = low & 0xffffffff;
	uint64_t lhs1 = low >> 32;
	uint64_t rhs0 = rhs.low & 0xffffffff;
	uint64_t rhs1 = rhs.low >> 32;
	high = high * rhs.low + low * rhs.high + lhs1 * rhs1;
	low = lhs0 * rhs0;
	uint64_t tmp = lhs1 * rhs0;
	*this += Int128(tmp >> 32, tmp << 32);
	tmp = lhs0 * rhs1;
	*this += Int128(tmp >> 32, tmp << 32);
	if (negative)
		negate();
	return *this;
}

Int128 &Int128::operator/=(Int128 rhs) noexcept {
	bool negative = isNegative() ^ rhs.isNegative();
	if (isNegative())
		negate();
	if (rhs.isNegative())
		rhs.negate();
	Int128 one(0, 1);
	if (rhs != one) {
		Int128 min(0, 0);
		Int128 max;
		int maxBit = 63;
		while (maxBit >= 0 && (rhs.high & (1ull << maxBit)) == 0 )
			maxBit--;
		if (maxBit < 0) {
			maxBit = 63;
			while (maxBit >= 0 && (rhs.low & (1ull << maxBit)) == 0)
				maxBit--;
			if (maxBit < 0)
				return *this;
			max = Int128(1ull << (63 - maxBit), 0);
		} else
			max = Int128(0, 1ull << (63 - maxBit));
		max = cmpAbs(*this, max) < 0 ? *this : max;
		max += one;
		while (max - min != one) {
			Int128 mid = (max + min) >> 1;
			if (cmpAbs(mid * rhs, *this) > 0)
				max = mid;
			else
				min = mid;
		}
		*this = min;
	}
	if (negative)
		negate();
	return *this;
}

Int128::Int128(uint64_t a, uint64_t b) noexcept: high(a), low(b) {
}

Int128 operator-(const Int128 &a) noexcept {
	return Int128(a).negate();
}

std::string Int128::str() const {
	Int128 tmp(*this);
	if (isNegative())
		tmp.negate();
	char str[50];
	memset(str, 0, 50);
	int size = 1;
	bool carry;
	for (int i = 127; i >= 0; --i) {
		carry = false;
		for (int j = 0; j < size; ++j) {
			str[48 - j] = str[48 - j] * 2 + (carry ? 1 : 0);
			carry = str[48 - j] >= 10;
			if (carry)
				str[48 - j] -= 10;
		}
		if (carry) {
			str[48 - size++] = 1;
		}

		if (i >= 64)
			carry = (tmp.high & (1ull << (i % 64))) > 0;
		else
			carry = (tmp.low & (1ull << i)) > 0;

		for (int j = 0; j < size && carry; ++j) {
			str[48 - j] += 1;
			carry = str[48 - j] >= 10;
			if (carry)
				str[48 - j] = 0;
		}
		if (carry) {
			str[48 - size++] = 1;
		}
	}
	for (int i = 0; i < size; ++i) {
		str[48 - i] += '0';
	}
	if (isNegative())
		str[48 - size++] = '-';
	return std::string(&str[49 - size]);
}

std::ostream &operator<<(std::ostream &out, const Int128 &a) {
	return out << a.str();
}

bool Int128::isNegative() const noexcept {
	return (high & 0x1ull << 63) != 0;
}

Int128 &Int128::negate() noexcept {
	high = ~high;
	low = ~low;
	return *this += 1;
}

void Int128::print() const {
	std::cout << *this << std::endl;
}

Int128 Int128::operator<<(int shift) const noexcept {
	return shift >= 64 ? Int128(low << (shift - 64), 0) :
			Int128((high << shift) | (low >> (64 - shift)), low << shift);
}

Int128 Int128::operator>>(int shift) const noexcept {
	return shift >= 64 ? Int128(0, high >> (shift - 64)) :
			Int128(high >> shift, (low >> shift) | (high << (64 - shift)));
}

int cmp(const Int128 &lhs, const Int128 &rhs) {
	if (static_cast<int64_t>(lhs.high) < static_cast<int64_t>(rhs.high))
		return -1;
	else if (static_cast<int64_t>(lhs.high) > static_cast<int64_t>(rhs.high))
		return 1;
	else {
		if (lhs.low < rhs.low)
			return -1;
		else if (lhs.low > rhs.low)
			return 1;
		else
			return 0;
	}
}

int cmpAbs(const Int128 &lhs, const Int128 &rhs) {
	if (lhs.high < rhs.high)
		return -1;
	else if (lhs.high > rhs.high)
		return 1;
	else {
		if (lhs.low < rhs.low)
			return -1;
		else if (lhs.low > rhs.low)
			return 1;
		else
			return 0;
	}
}

bool operator==(const Int128 &lhs, const Int128 &rhs) noexcept {
	return cmp(lhs, rhs) == 0;
}

bool operator!=(const Int128 &lhs, const Int128 &rhs) noexcept {
	return cmp(lhs, rhs) != 0;
}

bool operator<(const Int128 &lhs, const Int128 &rhs) noexcept {
	return cmp(lhs, rhs) < 0;
}

bool operator>(const Int128 &lhs, const Int128 &rhs) noexcept {
	return cmp(lhs, rhs) > 0;
}

bool operator<=(const Int128 &lhs, const Int128 &rhs) noexcept {
	return cmp(lhs, rhs) <= 0;
}

bool operator>=(const Int128 &lhs, const Int128 &rhs) noexcept {
	return cmp(lhs, rhs) >= 0;
}