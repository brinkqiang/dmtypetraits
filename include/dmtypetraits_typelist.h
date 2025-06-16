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

#ifndef __DMTYPETRAITS_TYPELIST_H_INCLUDE__
#define __DMTYPETRAITS_TYPELIST_H_INCLUDE__

#include "dmbase_typetraits.h"
#include "dmtypetraits_logical.h"
#include "dmtypetraits_extensions.h"
#include <tuple>

//-----------------------------------------------------------------------------
// 类型列表定义 (TypeList Definition)
//-----------------------------------------------------------------------------
template <typename... Ts>
struct dm_typelist {};

//-----------------------------------------------------------------------------
// 类型列表查询 (TypeList Queries)
//-----------------------------------------------------------------------------

/**
 * @brief 获取类型列表中的元素数量。
 */
template <typename List>
struct dm_typelist_size;

template <typename... Ts>
struct dm_typelist_size<dm_typelist<Ts...>>
    : std::integral_constant<std::size_t, sizeof...(Ts)> {};

template <typename List>
inline constexpr std::size_t dm_typelist_size_v = dm_typelist_size<List>::value;

/**
 * @brief 检查类型列表是否为空。
 */
template <typename List>
inline constexpr bool dm_typelist_is_empty_v = (dm_typelist_size_v<List> == 0);


/**
 * @brief 获取类型列表中位于索引 I 的类型。
 */
template <std::size_t I, typename List>
struct dm_typelist_at;

template <std::size_t I, typename... Ts>
struct dm_typelist_at<I, dm_typelist<Ts...>> {
    using type = std::tuple_element_t<I, std::tuple<Ts...>>;
};

template <std::size_t I, typename List>
using dm_typelist_at_t = typename dm_typelist_at<I, List>::type;


/**
 * @brief 获取类型列表的第一个元素类型。
 */
template <typename List>
using dm_typelist_front_t = dm_typelist_at_t<0, List>;


/**
 * @brief (C++17) 检查类型 T 是否存在于类型列表中。
 */
template <typename T, typename List>
struct dm_typelist_contains;

#if __cplusplus >= 201703L
template <typename T, typename... Ts>
struct dm_typelist_contains<T, dm_typelist<Ts...>>
    : std::bool_constant<(std::is_same_v<T, Ts> || ...)> {};
#endif

template <typename T, typename List>
inline constexpr bool dm_typelist_contains_v = dm_typelist_contains<T, List>::value;

//-----------------------------------------------------------------------------
// 类型列表操作 (TypeList Operations)
//-----------------------------------------------------------------------------

/**
 * @brief 在类型列表的前端添加一个或多个类型。
 */
template <typename List, typename... Ts>
struct dm_typelist_push_front;

template <typename... ListTs, typename... Ts>
struct dm_typelist_push_front<dm_typelist<ListTs...>, Ts...> {
    using type = dm_typelist<Ts..., ListTs...>;
};

template <typename List, typename... Ts>
using dm_typelist_push_front_t = typename dm_typelist_push_front<List, Ts...>::type;


/**
 * @brief 在类型列表的后端添加一个或多个类型。
 */
template <typename List, typename... Ts>
struct dm_typelist_push_back;

template <typename... ListTs, typename... Ts>
struct dm_typelist_push_back<dm_typelist<ListTs...>, Ts...> {
    using type = dm_typelist<ListTs..., Ts...>;
};

template <typename List, typename... Ts>
using dm_typelist_push_back_t = typename dm_typelist_push_back<List, Ts...>::type;


/**
 * @brief 移除类型列表的第一个元素。
 */
template <typename List>
struct dm_typelist_pop_front;

template <typename T, typename... Ts>
struct dm_typelist_pop_front<dm_typelist<T, Ts...>> {
    using type = dm_typelist<Ts...>;
};

template <typename List>
using dm_typelist_pop_front_t = typename dm_typelist_pop_front<List>::type;

//-----------------------------------------------------------------------------
// 类型列表算法 (TypeList Algorithms)
//-----------------------------------------------------------------------------

/**
 * @brief 对类型列表中的每个类型应用一个元函数 (MetaFunc)。
 * @tparam MetaFunc 一个接受单个类型参数并产生 ::type 结果的元函数。
 */
template <typename List, template <typename> class MetaFunc>
struct dm_typelist_transform;

template <template <typename> class MetaFunc, typename... Ts>
struct dm_typelist_transform<dm_typelist<Ts...>, MetaFunc> {
    using type = dm_typelist<typename MetaFunc<Ts>::type...>;
};

template <typename List, template <typename> class MetaFunc>
using dm_typelist_transform_t = typename dm_typelist_transform<List, MetaFunc>::type;


/**
 * @brief 根据谓词 (Predicate) 筛选类型列表中的类型。
 * @tparam Predicate 一个接受单个类型参数并产生 ::value 结果的元函数。
 */
namespace dm_detail {
    template <typename List, template <typename> class Predicate, typename ResultList>
    struct filter_impl;

    // 递归步骤: 如果谓词为真，将 Head 添加到结果列表
    template <
        template <typename> class Predicate,
        typename Head, typename... Tail,
        typename... ResultTs
    >
    struct filter_impl<dm_typelist<Head, Tail...>, Predicate, dm_typelist<ResultTs...>> {
        using type = dm_conditional_t<
            Predicate<Head>::value,
            typename filter_impl<dm_typelist<Tail...>, Predicate, dm_typelist<ResultTs..., Head>>::type,
            typename filter_impl<dm_typelist<Tail...>, Predicate, dm_typelist<ResultTs...>>::type
        >;
    };

    // 递归基例: 输入列表为空，返回结果列表
    template <template <typename> class Predicate, typename... ResultTs>
    struct filter_impl<dm_typelist<>, Predicate, dm_typelist<ResultTs...>> {
        using type = dm_typelist<ResultTs...>;
    };
} // namespace dm_detail

template<typename List, template<typename> class Predicate>
struct dm_typelist_filter {
    using type = typename dm_detail::filter_impl<List, Predicate, dm_typelist<>>::type;
};

template<typename List, template<typename> class Predicate>
using dm_typelist_filter_t = typename dm_typelist_filter<List, Predicate>::type;


#endif // __DMTYPETRAITS_TYPELIST_H_INCLUDE__