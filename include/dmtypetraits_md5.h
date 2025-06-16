
// Copyright (c) 2018 brinkqiang (brink.qiang@gmail.com)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef __DMTYPETRAITS_MD5_H_INCLUDE__
#define __DMTYPETRAITS_MD5_H_INCLUDE__

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>

#if __cplusplus < 202002L
#error "dmtypetraits_md5.h requires C++20 or later for consteval."
#endif

// Define helper macros consistent with the dm-style
#if defined(__clang__)
#define DM_MD5_INLINE __attribute__((always_inline)) inline
#define DM_MD5_CONSTEVAL __attribute__((always_inline)) consteval
#elif defined(_MSC_VER)
#define DM_MD5_INLINE __forceinline inline
#define DM_MD5_CONSTEVAL consteval
#else
#define DM_MD5_INLINE __attribute__((always_inline)) inline
#define DM_MD5_CONSTEVAL consteval __attribute__((always_inline))
#endif

namespace dm {
namespace md5_detail {

// A constexpr-friendly string literal container.
template <typename CharType, std::size_t Size>
struct DmStringLiteral : public std::array<CharType, Size + 1> {
    using base = std::array<CharType, Size + 1>;

    constexpr DmStringLiteral() = default;
    constexpr DmStringLiteral(const CharType (&value)[Size + 1]) {
        for (size_t i = 0; i < Size + 1; ++i) {
            (*this)[i] = value[i];
        }
    }

    constexpr std::size_t size() const { return Size; }
    using base::data;
};

// Deduction guide for DmStringLiteral
template <typename CharType, std::size_t Size>
DmStringLiteral(const CharType (&value)[Size])
    -> DmStringLiteral<CharType, Size - 1>;

// Compile-time string concatenation
template <typename CharType, size_t Len1, size_t Len2>
DM_MD5_CONSTEVAL auto operator+(DmStringLiteral<CharType, Len1> str1,
                                DmStringLiteral<CharType, Len2> str2) {
    auto ret = DmStringLiteral<CharType, Len1 + Len2>{};
    for (size_t i = 0; i < Len1; ++i) ret[i] = str1[i];
    for (size_t i = 0; i < Len2; ++i) ret[i + Len1] = str2[i];
    return ret;
}

// Core implementation of the MD5 algorithm.
// The logic is directly ported from the MD5CE struct in the reference code.
struct DmMd5Calculator {
    struct IntermediateData { uint32_t a, b, c, d; };
    using RoundData = std::array<uint32_t, 16>;

    static constexpr std::array<uint32_t, 64> kConstants = {
        {0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee, 0xf57c0faf, 0x4787c62a,
         0xa8304613, 0xfd469501, 0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
         0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821, 0xf61e2562, 0xc040b340,
         0x265e5a51, 0xe9b6c7aa, 0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
         0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed, 0xa9e3e905, 0xfcefa3f8,
         0x676f02d9, 0x8d2a4c8a, 0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
         0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70, 0x289b7ec6, 0xeaa127fa,
         0xd4ef3085, 0x04881d05, 0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
         0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039, 0x655b59c3, 0x8f0ccc92,
         0xffeff47d, 0x85845dd1, 0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
         0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391}};
    static constexpr std::array<uint32_t, 16> kShifts = {
        {7, 12, 17, 22, 5, 9, 14, 20, 4, 11, 16, 23, 6, 10, 15, 21}};
    static constexpr IntermediateData kInitialIntermediateData{
        0x67452301, 0xefcdab89, 0x98badcfe, 0x10325476};

    static constexpr uint32_t get_padded_message_length(const uint32_t n) {
        return (((n + 1 + 8) + 63) / 64) * 64;
    }

    static constexpr uint8_t extract_byte(const uint64_t value, const uint32_t i) {
        return static_cast<uint8_t>((value >> (i * 8)) & 0xff);
    }

    static constexpr uint8_t get_padded_message_byte(const char *data, const uint32_t n,
                                                     const uint32_t m, const uint32_t i) {
        if (i < n) return data[i];
        if (i == n) return 0x80;
        if (i >= m - 8) return extract_byte(uint64_t(n) * 8, i - (m - 8));
        return 0;
    }

    static constexpr uint32_t get_padded_message_word(const char *data, const uint32_t n,
                                                      const uint32_t m, const uint32_t i) {
        return static_cast<uint32_t>(get_padded_message_byte(data, n, m, i)) |
               (static_cast<uint32_t>(get_padded_message_byte(data, n, m, i + 1)) << 8) |
               (static_cast<uint32_t>(get_padded_message_byte(data, n, m, i + 2)) << 16) |
               (static_cast<uint32_t>(get_padded_message_byte(data, n, m, i + 3)) << 24);
    }
    
    static constexpr RoundData get_round_data(const char *data, const uint32_t n,
                                              const uint32_t m, const uint32_t i) {
        return {{get_padded_message_word(data, n, m, i),
                 get_padded_message_word(data, n, m, i + 4),
                 get_padded_message_word(data, n, m, i + 8),
                 get_padded_message_word(data, n, m, i + 12),
                 get_padded_message_word(data, n, m, i + 16),
                 get_padded_message_word(data, n, m, i + 20),
                 get_padded_message_word(data, n, m, i + 24),
                 get_padded_message_word(data, n, m, i + 28),
                 get_padded_message_word(data, n, m, i + 32),
                 get_padded_message_word(data, n, m, i + 36),
                 get_padded_message_word(data, n, m, i + 40),
                 get_padded_message_word(data, n, m, i + 44),
                 get_padded_message_word(data, n, m, i + 48),
                 get_padded_message_word(data, n, m, i + 52),
                 get_padded_message_word(data, n, m, i + 56),
                 get_padded_message_word(data, n, m, i + 60)}};
    }

    static constexpr uint32_t calc_f(const uint32_t i, const uint32_t b,
                                     const uint32_t c, const uint32_t d) {
        if (i < 16) return d ^ (b & (c ^ d));
        if (i < 32) return c ^ (d & (b ^ c));
        if (i < 48) return b ^ c ^ d;
        return c ^ (b | (~d));
    }

    static constexpr uint32_t calc_g(const uint32_t i) {
        if (i < 16) return i;
        if (i < 32) return (5 * i + 1) % 16;
        if (i < 48) return (3 * i + 5) % 16;
        return (7 * i) % 16;
    }

    static constexpr uint32_t get_shift(const uint32_t i) {
        return kShifts[(i / 16) * 4 + (i % 4)];
    }

    static constexpr uint32_t left_rotate(const uint32_t value, const uint32_t bits) {
        return (value << bits) | (value >> (32 - bits));
    }
    
    static constexpr IntermediateData apply_step(const uint32_t i, const RoundData &data,
                                                 const IntermediateData &intermediate) {
        const uint32_t g = calc_g(i);
        const uint32_t f = calc_f(i, intermediate.b, intermediate.c, intermediate.d) +
                           intermediate.a + kConstants[i] + data[g];
        return {intermediate.d, intermediate.b + left_rotate(f, get_shift(i)),
                intermediate.b, intermediate.c};
    }

    static constexpr IntermediateData add(const IntermediateData &id1, const IntermediateData &id2) {
        return {id1.a + id2.a, id1.b + id2.b, id1.c + id2.c, id1.d + id2.d};
    }

    static constexpr IntermediateData process_message(const char *message, const uint32_t n) {
        const uint32_t m = get_padded_message_length(n);
        IntermediateData intermediate0 = kInitialIntermediateData;
        for (uint32_t offset = 0; offset < m; offset += 64) {
            RoundData data = get_round_data(message, n, m, offset);
            IntermediateData intermediate1 = intermediate0;
            for (uint32_t i = 0; i < 64; ++i)
                intermediate1 = apply_step(i, data, intermediate1);
            intermediate0 = add(intermediate0, intermediate1);
        }
        return intermediate0;
    }

    static constexpr uint32_t string_length(const char *string) {
        const char *end = string;
        while (*end != 0) ++end;
        return static_cast<uint32_t>(end - string);
    }

    static constexpr uint32_t swap_endian(uint32_t a) {
        return ((a & 0xff) << 24) | (((a >> 8) & 0xff) << 16) |
               (((a >> 16) & 0xff) << 8) | ((a >> 24) & 0xff);
    }

    static constexpr uint32_t hash32(const char *data, uint32_t n) {
        return swap_endian(process_message(data, n).a);
    }
};

} // namespace md5_detail

/**
 * @brief (C++20) Computes the 32-bit MD5 hash of a string at compile time.
 */
constexpr uint32_t dm_md5_hash32(const char *string, uint32_t length) {
    return md5_detail::DmMd5Calculator::hash32(string, length);
}

/**
 * @brief (C++20) Computes the 32-bit MD5 hash of a null-terminated string at compile time.
 */
constexpr uint32_t dm_md5_hash32(const char *string) {
    return md5_detail::DmMd5Calculator::hash32(string, md5_detail::DmMd5Calculator::string_length(string));
}

// To enable user-defined literals like "hello"_dm_md5_int
namespace literals {

template <md5_detail::DmStringLiteral String>
DM_MD5_CONSTEVAL auto operator""_dm_sl() {
    return String;
}

template <md5_detail::DmStringLiteral String>
DM_MD5_CONSTEVAL auto operator""_dm_md5_int() {
    return dm_md5_hash32(String.data(), String.size());
}

} // namespace literals
} // namespace dm

#endif // __DMTYPETRAITS_MD5_H_INCLUDE__