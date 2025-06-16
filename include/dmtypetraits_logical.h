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

#ifndef __DMTYPETRAITS_LOGICAL_H_INCLUDE__
#define __DMTYPETRAITS_LOGICAL_H_INCLUDE__

#include <type_traits>

//-----------------------------------------------------------------------------
// 逻辑组合萃取 (Logical Combination Traits)
//-----------------------------------------------------------------------------
// 这些工具需要 C++17 或更高版本
//
// 它们用于在编译期对多个类型谓词 (返回 bool 值的萃取) 进行逻辑运算。
//-----------------------------------------------------------------------------

/**
 * @brief 编译期逻辑与。若所有谓词 B... 都为 true，则结果为 true。
 * @example dm_conjunction_v<std::is_integral<int>, std::is_signed<int>> // true
 */
template<class... B>
inline constexpr bool dm_conjunction_v = std::conjunction_v<B...>;

/**
 * @brief 编译期逻辑或。若任一谓词 B... 为 true，则结果为 true。
 * @example dm_disjunction_v<std::is_integral<float>, std::is_floating_point<float>> // true
 */
template<class... B>
inline constexpr bool dm_disjunction_v = std::disjunction_v<B...>;

/**
 * @brief 编译期逻辑非。反转谓词 B 的结果。
 * @example dm_negation_v<std::is_const<int>> // true
 */
template<class B>
inline constexpr bool dm_negation_v = std::negation_v<B>;


#endif // __DMTYPETRAITS_LOGICAL_H_INCLUDE__