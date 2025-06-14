
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

#ifndef __DMTYPETRAITS_H_INCLUDE__
#define __DMTYPETRAITS_H_INCLUDE__

#include "dmos.h" // dmos.h已经处理平台头文件, 以及相关宏定义
#include "dmfix_win_utf8.h" // 处理 win平台utf8问题
#include "dmmoduleptr.h"

#ifndef __DMTYPETRAITS_H_INCLUDE__
#define __DMTYPETRAITS_H_INCLUDE__

#include <type_traits>
#include <utility>

//================================================================================
//                     dmtypetraits - 编译期特性判断库
//================================================================================
// 说明：
// 本库提供一系列宏，用于在编译期检查类/结构体的成员属性。
// 所有宏均以 DM_MEMBER_ 开头，返回一个编译期常量 bool 值。
// 可用于 static_assert 或 if constexpr。
//================================================================================


/**
 * @brief 检查类 T 是否拥有名为 m 的成员。
 */
#define DM_MEMBER_HAS(T, m) \
    []() constexpr { \
        if constexpr (requires(T& t) { t.m; }) { /* 检查实例成员 */ \
            return true; \
        } else if constexpr (requires { T::m; }) { /* 检查静态成员 */ \
            return true; \
        } else { \
            return false; \
        } \
    }()

/**
 * @brief 检查类 T 的成员 m 是否为位域。
 */
#define DM_MEMBER_IS_BITFIELD(T, m) \
    []() constexpr { \
        if constexpr (requires(T& t) { &(t.m); }) { \
            return false; \
        } else { \
            return DM_MEMBER_HAS(T, m); \
        } \
    }()

/**
 * @brief 检查类 T 的成员 m 是否为 static 成员。
 */
#define DM_MEMBER_IS_STATIC(T, m) \
    []() constexpr { \
        if constexpr (requires { T::m; }) { \
            return true; \
        } else { \
            return false; \
        } \
    }()

/**
 * @brief 检查类 T 的成员 m 是否为 mutable 成员。
 */
#define DM_MEMBER_IS_MUTABLE(T, m) \
    []() constexpr { \
        if constexpr (DM_MEMBER_HAS(T, m) && !DM_MEMBER_IS_STATIC(T, m)) { \
            using MemberType = decltype(std::declval<T>().m); \
            if constexpr (!std::is_const_v<MemberType> && \
                          requires(const T& t) { t.m = std::declval<MemberType>(); }) { \
                return true; \
            } \
        } \
        return false; \
    }()

/**
 * @brief 检查类 T 的成员 m 是否为 const (不可修改)。
 */
#define DM_MEMBER_IS_CONST(T, m) \
    []() constexpr { \
        if constexpr (DM_MEMBER_HAS(T, m)) { \
            using MemberType = decltype(std::declval<T&>().m); \
            if constexpr (!requires(T& t) { t.m = std::declval<MemberType>(); }) { \
                return true; \
            } \
        } \
        return false; \
    }()


/**
 * @brief 获取成员 m 的类型。
 */
#define DM_MEMBER_TYPE(T, m) decltype(std::declval<T&>().m)

/**
 * @brief 检查类 T 的成员 m 是否为指针类型。
 */
#define DM_MEMBER_IS_POINTER(T, m) \
    (DM_MEMBER_HAS(T, m) && std::is_pointer_v<DM_MEMBER_TYPE(T, m)>)

/**
 * @brief 检查类 T 的成员 m 是否为引用类型。
 */
#define DM_MEMBER_IS_REFERENCE(T, m) \
    (DM_MEMBER_HAS(T, m) && std::is_reference_v<DM_MEMBER_TYPE(T, m)>)

/**
 * @brief 检查类 T 的成员 m 是否为数组类型。
 */
#define DM_MEMBER_IS_ARRAY(T, m) \
    (DM_MEMBER_HAS(T, m) && std::is_array_v<DM_MEMBER_TYPE(T, m)>)

/**
 * @brief 检查类 T 的成员 m 是否可调用。
 * @param ...args 可选的调用参数类型列表
 */
#define DM_MEMBER_IS_CALLABLE(T, m, ...) \
    []() constexpr { \
        if constexpr (DM_MEMBER_HAS(T, m)) { \
            if constexpr (requires(T& t) { t.m(std::declval<__VA_ARGS__>()...); }) { \
                return true; \
            } \
        } \
        return false; \
    }()

#endif // __DMTYPETRAITS_H_INCLUDE__