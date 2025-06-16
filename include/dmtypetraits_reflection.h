
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

// include/dmtypetraits_reflection.h

#ifndef __DMTYPETRAITS_REFLECTION_H_INCLUDE__
#define __DMTYPETRAITS_REFLECTION_H_INCLUDE__

#include "dmbase_typetraits.h"
#include <utility>
#include <type_traits>

#if __cplusplus < 202002L
#error "dmtypetraits_reflection.h requires C++20 or later for consteval and requires expressions."
#endif

namespace dm_detail {

/**
 * @brief 一个可以隐式转换为任何类型的“万能转换器”。
 *
 * 这是探测成员数量技巧的核心辅助工具。它本身不占任何体积，
 * 仅用于在编译期试探能否用 N 个参数来构造一个对象。
 */
struct UniversalConverter {
    template <typename T>
    operator T() const;
};

/**
 * @brief 递归地、在编译期计算一个聚合类型 T 的成员数量。
 * @tparam T 要计算的聚合类型。
 * @tparam Args 在递归中累积的参数类型，通常是 UniversalConverter。
 * @return T 的成员数量。
 */
template <typename T, typename... Args>
consteval size_t count_members_impl() {
    // 核心技巧：检查是否可以用 sizeof...(Args) + 1 个参数来构造 T。
    // T{ {Args{}}..., {UniversalConverter{}} } 尝试用 N+1 个参数进行聚合初始化。
    if constexpr (!requires { T{ {Args{}}..., {UniversalConverter{}} }; }) {
        // 如果上述 requires 表达式为 false，说明 T 不能由 N+1 个参数构造。
        // 这意味着 T 的成员数量就是 N (即 sizeof...(Args))。
        // 这是递归的终点。
        return sizeof...(Args);
    }
    else {
        // 如果可以用 N+1 个参数构造，说明成员数量至少是 N+1。
        // 我们继续递归，在参数包中再增加一个 UniversalConverter，继续试探。
        return count_members_impl<T, Args..., UniversalConverter>();
    }
}

} // namespace dm_detail

/**
 * @brief (C++20) 获取一个聚合类型 T 的成员数量。
 *
 * @warning 此方法仅对聚合类型（例如，没有用户自定义构造函数、
 * 没有私有/保护的非静态成员、没有基类、没有虚函数的 struct 或 class）有效。
 * @tparam T 要查询的类型。
 */
template <typename T>
inline constexpr size_t dm_member_count_v = dm_detail::count_members_impl<dm_remove_cvref_t<T>>();

/**
 * @brief (C++20) 使用访问者函数遍历一个聚合对象的所有成员。
 *
 * 此函数利用结构化绑定将对象的成员解构，并将它们作为独立参数传递给 visitor。
 *
 * @tparam T 聚合对象的类型。
 * @tparam Visitor 访问者函数的类型（通常是 lambda）。
 * @param object 要访问其成员的聚合对象。
 * @param visitor 一个可调用对象，其参数列表匹配 object 的成员列表。
 * @return visitor 的返回值。
 *
 * @example
 * struct Point { float x, y, z; };
 * Point p{1, 2, 3};
 * dm_visit_members(p, [](float x, float y, float z){
 * std::cout << x << y << z;
 * });
 */
template <typename T, typename Visitor>
constexpr decltype(auto) dm_visit_members(T&& object, Visitor&& visitor) {
    using CleanT = dm_remove_cvref_t<T>;
    constexpr size_t Count = dm_member_count_v<CleanT>;
    
    // 不允许访问空结构体，因为它们无法被结构化绑定解构。
    static_assert(Count > 0 || !std::is_class_v<CleanT>, "Visiting members of an empty struct/class is not supported.");

    // 通过 if constexpr 和结构化绑定，根据成员数量调用不同的 visitor。
    // 这种展开方式虽然冗长，但在编译期完成，没有运行时开销。
    // 对于生产级库，这段代码通常由脚本生成。
    if constexpr (Count == 1) {
        auto&& [m1] = object;
        return visitor(m1);
    } else if constexpr (Count == 2) {
        auto&& [m1, m2] = object;
        return visitor(m1, m2);
    } else if constexpr (Count == 3) {
        auto&& [m1, m2, m3] = object;
        return visitor(m1, m2, m3);
    } else if constexpr (Count == 4) {
        auto&& [m1, m2, m3, m4] = object;
        return visitor(m1, m2, m3, m4);
    } else if constexpr (Count == 5) {
        auto&& [m1, m2, m3, m4, m5] = object;
        return visitor(m1, m2, m3, m4, m5);
    } else if constexpr (Count == 6) {
        auto&& [m1, m2, m3, m4, m5, m6] = object;
        return visitor(m1, m2, m3, m4, m5, m6);
    } // ... 可以根据需要扩展到更多成员
    else {
        // 如果成员数量为0或超出支持范围，给出编译期错误。
        static_assert(Count <= 6, "dm_visit_members currently supports up to 6 members. You can extend it if needed.");
        return visitor();
    }
}

#endif // __DMTYPETRAITS_REFLECTION_H_INCLUDE__