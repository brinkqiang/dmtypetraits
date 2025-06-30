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

#ifndef __DMCAST_H_INCLUDE__
#define __DMCAST_H_INCLUDE__

#include <string>
#include <cstdint>
#include <tuple>
#include <array>
#include <type_traits>
#include <functional>
#include <vector>
#include <list>
#include <deque>
#include <set>
#include <unordered_set>
#include <map>
#include <unordered_map>
#include <queue>
#include <stack>
#include <forward_list>
#include <iomanip>
#include <iostream>

namespace dmcast
{
    // 前向声明
    template <typename To, typename From>
    typename std::enable_if<!std::is_same<To, From>::value, To>::type
    lexical_cast(const From &from);

    template <typename To, typename From>
    typename std::enable_if<std::is_same<To, From>::value, To>::type 
    lexical_cast(const From &from);

    template <typename To, typename From>
    struct Converter
    {
        static To convert(const From &from)
        {
            return To();
        }
    };

    // 直接特化每个类型，避免复杂的模板继承
    template <typename From>
    struct Converter<char, From>
    {
        static char convert(const From &from)
        {
            return static_cast<char>(std::stoi(from));
        }
    };

    template <typename From>
    struct Converter<int8_t, From>
    {
        static int8_t convert(const From &from)
        {
            return static_cast<int8_t>(std::stoi(from));
        }
    };

    template <typename From>
    struct Converter<uint8_t, From>
    {
        static uint8_t convert(const From &from)
        {
            return static_cast<uint8_t>(std::stoul(from));
        }
    };

    template <typename From>
    struct Converter<int16_t, From>
    {
        static int16_t convert(const From &from)
        {
            return static_cast<int16_t>(std::stoi(from));
        }
    };

    template <typename From>
    struct Converter<uint16_t, From>
    {
        static uint16_t convert(const From &from)
        {
            return static_cast<uint16_t>(std::stoul(from));
        }
    };

    template <typename From>
    struct Converter<int32_t, From>
    {
        static int32_t convert(const From &from)
        {
            return std::stoi(from);
        }
    };

    template <typename From>
    struct Converter<uint32_t, From>
    {
        static uint32_t convert(const From &from)
        {
            return std::stoul(from);
        }
    };

    template <typename From>
    struct Converter<int64_t, From>
    {
        static int64_t convert(const From &from)
        {
            return std::stoll(from);
        }
    };

    template <typename From>
    struct Converter<uint64_t, From>
    {
        static uint64_t convert(const From &from)
        {
            return std::stoull(from);
        }
    };

    template <typename From>
    struct Converter<float, From>
    {
        static float convert(const From &from)
        {
            return std::stof(from);
        }
    };

    template <typename From>
    struct Converter<double, From>
    {
        static double convert(const From &from)
        {
            return std::stod(from);
        }
    };

    template <typename From>
    struct Converter<long double, From>
    {
        static long double convert(const From &from)
        {
            return std::stold(from);
        }
    };

    // bool类型转换器
    template <typename From>
    struct Converter<bool, From>
    {
        static typename std::enable_if<std::is_integral<From>::value, bool>::type
        convert(From from)
        {
            return !!from;
        }
    };

    // bool的字符串特化版本
    template <>
    struct Converter<bool, std::string>
    {
        static bool convert(const std::string &from)
        {
            return !!std::stoi(from);
        }
    };

    template <>
    struct Converter<bool, const char *>
    {
        static bool convert(const char *from)
        {
            return !!std::stoi(from);
        }
    };

    template <>
    struct Converter<bool, char *>
    {
        static bool convert(char *from)
        {
            return !!std::stoi(from);
        }
    };

    template <unsigned N>
    struct Converter<bool, const char[N]>
    {
        static bool convert(const char (&from)[N])
        {
            return !!std::stoi(from);
        }
    };

    template <unsigned N>
    struct Converter<bool, char[N]>
    {
        static bool convert(const char (&from)[N])
        {
            return !!std::stoi(from);
        }
    };


    template <typename From>
    struct Converter<std::string, From>
    {
        static std::string convert(const From& from)
        {
            // 使用 if constexpr 根据类型选择不同的转换方式
            if constexpr (std::is_arithmetic_v<From> && !std::is_same_v<From, bool>)
            {
                // 对所有非bool的算术类型(int, float, double等)使用 std::to_string
                return std::to_string(from);
            }
            else if constexpr (std::is_enum_v<From>)
            {
                // 对枚举类型，先转换为其底层整数类型，再转为字符串
                return std::to_string(static_cast<std::underlying_type_t<From>>(from));
            }
            else
            {
                // 对于所有其他不支持的类型，在编译期给出清晰的错误提示
                // 这会阻止编译器尝试用 std::to_string 处理结构体等类型
                static_assert(!sizeof(From), "dmcast::lexical_cast to std::string is not supported for this type. Please provide a Converter<std::string, YourType> specialization.");
                return ""; // 这行代码在 static_assert 失败时不会被执行
            }
        }
    };

    template <>
    struct Converter<std::string, bool>
    {
        static std::string convert(const bool &from)
        {
            return std::to_string((int)from);
        }
    };

    template <>
    struct Converter<std::string, const char *>
    {
        static std::string convert(const char *from)
        {
            return from;
        }
    };

    template <unsigned N>
    struct Converter<std::string, char[N]>
    {
        static std::string convert(const char (&from)[N])
        {
            return from;
        }
    };

    // STL容器转换为string的通用实现
    namespace detail
    {
        // 序列容器转换器
        template <typename Container>
        std::string convert_sequence_container(const Container &from, const std::string &open = "[", const std::string &close = "]")
        {
            std::string result = open;
            bool first = true;
            for (const auto &item : from)
            {
                if (!first) result += ",";
                result += lexical_cast<std::string>(item);
                first = false;
            }
            result += close;
            return result;
        }

        // 关联容器转换器（单值）
        template <typename Container>
        std::string convert_associative_container(const Container &from)
        {
            std::string result = "{";
            bool first = true;
            for (const auto &item : from)
            {
                if (!first) result += ",";
                result += lexical_cast<std::string>(item);
                first = false;
            }
            result += "}";
            return result;
        }

        // 映射容器转换器（键值对）
        template <typename Container>
        std::string convert_map_container(const Container &from)
        {
            std::string result = "{";
            bool first = true;
            for (const auto &pair : from)
            {
                if (!first) result += ",";
                result += lexical_cast<std::string>(pair.first);
                result += ":";
                result += lexical_cast<std::string>(pair.second);
                first = false;
            }
            result += "}";
            return result;
        }
    }

    // 序列容器特化
    #define DECLARE_SEQUENCE_CONTAINER_CONVERTER(CONTAINER) \
        template <typename T> \
        struct Converter<std::string, CONTAINER<T>> \
        { \
            static std::string convert(const CONTAINER<T> &from) \
            { \
                return detail::convert_sequence_container(from); \
            } \
        };

    DECLARE_SEQUENCE_CONTAINER_CONVERTER(std::vector)
    DECLARE_SEQUENCE_CONTAINER_CONVERTER(std::list)
    DECLARE_SEQUENCE_CONTAINER_CONVERTER(std::deque)
    DECLARE_SEQUENCE_CONTAINER_CONVERTER(std::forward_list)

    // 关联容器特化（单值）
    #define DECLARE_SET_CONTAINER_CONVERTER(CONTAINER) \
        template <typename T> \
        struct Converter<std::string, CONTAINER<T>> \
        { \
            static std::string convert(const CONTAINER<T> &from) \
            { \
                return detail::convert_associative_container(from); \
            } \
        };

    DECLARE_SET_CONTAINER_CONVERTER(std::set)
    DECLARE_SET_CONTAINER_CONVERTER(std::multiset)
    DECLARE_SET_CONTAINER_CONVERTER(std::unordered_set)
    DECLARE_SET_CONTAINER_CONVERTER(std::unordered_multiset)

    // 映射容器特化（键值对）
    #define DECLARE_MAP_CONTAINER_CONVERTER(CONTAINER) \
        template <typename K, typename V> \
        struct Converter<std::string, CONTAINER<K, V>> \
        { \
            static std::string convert(const CONTAINER<K, V> &from) \
            { \
                return detail::convert_map_container(from); \
            } \
        };

    DECLARE_MAP_CONTAINER_CONVERTER(std::map)
    DECLARE_MAP_CONTAINER_CONVERTER(std::multimap)
    DECLARE_MAP_CONTAINER_CONVERTER(std::unordered_map)
    DECLARE_MAP_CONTAINER_CONVERTER(std::unordered_multimap)

    #undef DECLARE_SEQUENCE_CONTAINER_CONVERTER
    #undef DECLARE_SET_CONTAINER_CONVERTER
    #undef DECLARE_MAP_CONTAINER_CONVERTER


    // 将 std::vector<std::string> 转换为 std::tuple
    template<typename... Args>
    struct Converter<std::tuple<Args...>, std::vector<std::string>>
    {
        using TupleType = std::tuple<Args...>;

        template<std::size_t... I>
        static TupleType convert_impl(const std::vector<std::string>& from, std::index_sequence<I...>)
        {
            return std::make_tuple(
                dmcast::lexical_cast<std::tuple_element_t<I, TupleType>>(from.at(I))...
            );
        }

        static TupleType convert(const std::vector<std::string>& from)
        {
            constexpr size_t tuple_size = sizeof...(Args);
            if (from.size() < tuple_size)
            {
                throw std::invalid_argument("Vector size is smaller than the number of elements in the tuple.");
            }

            return convert_impl(
                from,
                std::make_index_sequence<tuple_size>{}
            );
        }
    };

    // 将 std::tuple 转换为 std::vector<std::string>
    template <typename... Args>
    struct Converter<std::vector<std::string>, std::tuple<Args...>>
    {
        static std::vector<std::string> convert(const std::tuple<Args...>& from)
        {
            std::vector<std::string> result;
            result.reserve(sizeof...(Args));

            std::apply([&](const auto&... args) {
                // 使用 initializer_list 展开参数包，逐个转换为 string 并添加到 vector
                (void)std::initializer_list<int>{((result.push_back(lexical_cast<std::string>(args))), 0)...};
                }, from);

            return result;
        }
    };

    template <typename To, typename From>
    typename std::enable_if<!std::is_same<To, From>::value, To>::type
    lexical_cast(const From &from)
    {
        return dmcast::Converter<To, From>::convert(from);
    }

    template <typename To, typename From>
    typename std::enable_if<std::is_same<To, From>::value, To>::type 
    lexical_cast(const From &from)
    {
        return from;
    }

    template <typename... T>
    std::string lexical_cast(std::tuple<T...> &t)
    {
        std::string strData;
        std::apply([&](auto &&...args)
                   { ((strData += (strData.empty() ? "" : ","),
                       strData += lexical_cast<std::string>(args)),
                      ...); }, t);
        return strData;
    }

    template <typename Array, std::size_t... N>
    decltype(auto) array2tuple_impl(const Array &a, std::index_sequence<N...>)
    {
        return std::make_tuple(a[N]...);
    }

    template <typename T, std::size_t N>
    decltype(auto) array2tuple(const std::array<T, N> &a)
    {
        return array2tuple_impl(a, std::make_index_sequence<N>{});
    }

    template <typename T, typename... U, unsigned... N>
    std::array<T, sizeof...(U)>
    tuple2array_impl(std::tuple<U...> &t, std::index_sequence<N...>)
    {
        return std::array<T, sizeof...(U)>{{std::get<N>(t)...}};
    }

    template <typename T, typename... U>
    std::array<T, sizeof...(U)>
    tuple2array(std::tuple<U...> t)
    {
        return tuple2array_impl<T>(t, std::make_index_sequence<sizeof...(U)>{});
    }

    template <typename Function, typename Tuple, std::size_t... Index>
    decltype(auto) invoke_impl(Function &&func, Tuple &&t,
                               std::index_sequence<Index...>)
    {
        return func(std::get<Index>(std::forward<Tuple>(t))...);
    }

    template <typename Function, typename Tuple>
    decltype(auto) invoke(Tuple &&t, Function &&func)
    {
        constexpr auto size = std::tuple_size<typename std::decay<Tuple>::type>::value;
        return invoke_impl(std::forward<Function>(func), std::forward<Tuple>(t),
                           std::make_index_sequence<size>{});
    }

    template<typename To, typename From>
    To union_cast(const From& from) {
        static_assert(sizeof(From) == sizeof(To), "Size of From and To must be equal.");
        union {
            From from;
            To to;
        } u{ from };
        return u.to;
    }
}
#endif // __DMCAST_H_INCLUDE__