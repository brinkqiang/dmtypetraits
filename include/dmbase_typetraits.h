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

#ifndef __DMBASE_TYPETRAITS_H_INCLUDE__
#define __DMBASE_TYPETRAITS_H_INCLUDE__

#include <type_traits>
#include <utility>

// 仅在 C++20 及以上版本中引入 <concepts> 头文件
#if __cplusplus >= 202002L
#include <concepts>
#endif

//-----------------------------------------------------------------------------
// 类型属性查询 (Type Property Queries)
//-----------------------------------------------------------------------------

// Primary type categories
template<typename T>
inline constexpr bool dm_is_void_v = std::is_void_v<T>;
template<typename T>
inline constexpr bool dm_is_null_pointer_v = std::is_null_pointer_v<T>;
template<typename T>
inline constexpr bool dm_is_integral_v = std::is_integral_v<T>;
template<typename T>
inline constexpr bool dm_is_floating_point_v = std::is_floating_point_v<T>;
template<typename T>
inline constexpr bool dm_is_array_v = std::is_array_v<T>;
template<typename T>
inline constexpr bool dm_is_enum_v = std::is_enum_v<T>;
template<typename T>
inline constexpr bool dm_is_union_v = std::is_union_v<T>;
template<typename T>
inline constexpr bool dm_is_class_v = std::is_class_v<T>;
template<typename T>
inline constexpr bool dm_is_function_v = std::is_function_v<T>;
template<typename T>
inline constexpr bool dm_is_pointer_v = std::is_pointer_v<T>;
template<typename T>
inline constexpr bool dm_is_lvalue_reference_v = std::is_lvalue_reference_v<T>;
template<typename T>
inline constexpr bool dm_is_rvalue_reference_v = std::is_rvalue_reference_v<T>;
template<typename T>
inline constexpr bool dm_is_member_pointer_v = std::is_member_pointer_v<T>;

// Composite type categories
template<typename T>
inline constexpr bool dm_is_fundamental_v = std::is_fundamental_v<T>;
template<typename T>
inline constexpr bool dm_is_arithmetic_v = std::is_arithmetic_v<T>;
template<typename T>
inline constexpr bool dm_is_scalar_v = std::is_scalar_v<T>;
template<typename T>
inline constexpr bool dm_is_object_v = std::is_object_v<T>;
template<typename T>
inline constexpr bool dm_is_compound_v = std::is_compound_v<T>;
template<typename T>
inline constexpr bool dm_is_reference_v = std::is_reference_v<T>;

// Type properties
template<typename T>
inline constexpr bool dm_is_const_v = std::is_const_v<T>;
template<typename T>
inline constexpr bool dm_is_volatile_v = std::is_volatile_v<T>;
template<typename T>
inline constexpr bool dm_is_trivial_v = std::is_trivial_v<T>;
template<typename T>
inline constexpr bool dm_is_trivially_copyable_v = std::is_trivially_copyable_v<T>;
template<typename T>
inline constexpr bool dm_is_standard_layout_v = std::is_standard_layout_v<T>;
template<typename T>
inline constexpr bool dm_is_empty_v = std::is_empty_v<T>;
template<typename T>
inline constexpr bool dm_is_polymorphic_v = std::is_polymorphic_v<T>;
template<typename T>
inline constexpr bool dm_is_abstract_v = std::is_abstract_v<T>;
template<typename T>
inline constexpr bool dm_is_final_v = std::is_final_v<T>;
template<typename T>
inline constexpr bool dm_is_aggregate_v = std::is_aggregate_v<T>; // C++17
template<typename T>
inline constexpr bool dm_is_signed_v = std::is_signed_v<T>;
template<typename T>
inline constexpr bool dm_is_unsigned_v = std::is_unsigned_v<T>;

// Supported operations
template<typename T, typename... Args>
inline constexpr bool dm_is_constructible_v = std::is_constructible_v<T, Args...>;
template<typename T>
inline constexpr bool dm_is_default_constructible_v = std::is_default_constructible_v<T>;

//-----------------------------------------------------------------------------
// 类型关系查询 (Type Relationship Queries)
//-----------------------------------------------------------------------------

template<typename T, typename U>
inline constexpr bool dm_is_same_v = std::is_same_v<T, U>;
template<typename Base, typename Derived>
inline constexpr bool dm_is_base_of_v = std::is_base_of_v<Base, Derived>;
template<typename From, typename To>
inline constexpr bool dm_is_convertible_v = std::is_convertible_v<From, To>;

//-----------------------------------------------------------------------------
// 类型转换 (Type Modifications)
//-----------------------------------------------------------------------------

template<typename T>
using dm_remove_cv_t = std::remove_cv_t<T>;
template<typename T>
using dm_remove_const_t = std::remove_const_t<T>;
template<typename T>
using dm_remove_volatile_t = std::remove_volatile_t<T>;
template<typename T>
using dm_add_cv_t = std::add_cv_t<T>;
template<typename T>
using dm_add_const_t = std::add_const_t<T>;
template<typename T>
using dm_add_volatile_t = std::add_volatile_t<T>;

template<typename T>
using dm_remove_reference_t = std::remove_reference_t<T>;
template<typename T>
using dm_add_lvalue_reference_t = std::add_lvalue_reference_t<T>;
template<typename T>
using dm_add_rvalue_reference_t = std::add_rvalue_reference_t<T>;

template<typename T>
using dm_decay_t = std::decay_t<T>;
template<typename T>
using dm_underlying_type_t = std::underlying_type_t<T>;

//-----------------------------------------------------------------------------
// 条件类型选择工具
//-----------------------------------------------------------------------------

/**
 * @brief 根据条件选择类型
 */
template<bool B, typename T, typename F>
using dm_conditional_t = std::conditional_t<B, T, F>;

/**
 * @brief 如果类型 T 满足条件 Condition，则返回 T，否则返回 void
 */
template<template<typename> class Condition, typename T>
using dm_enable_if_t = std::enable_if_t<Condition<T>::value, T>;

template<typename T, T... Ints>
using dm_integer_sequence = std::integer_sequence<T, Ints...>;

template<std::size_t... Ints>
using dm_index_sequence = std::index_sequence<Ints...>;

template<std::size_t N>
using dm_make_index_sequence = std::make_index_sequence<N>;

template<typename... T>
using dm_index_sequence_for = std::index_sequence_for<T...>;

//-----------------------------------------------------------------------------
// C++20 特有功能 (C++20-Specific Features)
//-----------------------------------------------------------------------------
#if __cplusplus >= 202002L

// C++20 Type transformations
template<typename T>
using dm_remove_cvref_t = std::remove_cvref_t<T>;
template<typename T>
using dm_type_identity_t = std::type_identity_t<T>;

// C++20 概念 (Concepts)
template<typename T, typename U>
concept dm_same_as = std::same_as<T, U>;

template<typename Derived, typename Base>
concept dm_derived_from = std::derived_from<Derived, Base>;

template<typename From, typename To>
concept dm_convertible_to = std::convertible_to<From, To>;

template<typename T>
concept dm_integral = std::integral<T>;

template<typename T>
concept dm_signed_integral = std::signed_integral<T>;

template<typename T>
concept dm_unsigned_integral = std::unsigned_integral<T>;

template<typename T>
concept dm_floating_point = std::floating_point<T>;

template<typename F, typename... Args>
concept dm_invocable = std::invocable<F, Args...>;

#else // C++20 之前的版本 (例如 C++17)

// 为 C++17 手动实现 dm_remove_cvref_t
template<typename T>
using dm_remove_cvref_t = dm_remove_cv_t<dm_remove_reference_t<T>>;

#endif // __cplusplus >= 202002L

#endif // __DMBASE_TYPETRAITS_H_INCLUDE__