#ifndef __DMTYPETRAITS_PACK_IMPL_H_INCLUDE__
#define __DMTYPETRAITS_PACK_IMPL_H_INCLUDE__

#include "dmtypetraits_reflection.h"
#include "dmtypetraits_md5.h"

#include <bit>
#include <climits>
#include <cstring>
#include <iostream>
#include <iterator>
#include <limits>
#include <map>
#include <optional>
#include <system_error>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#if defined(__GNUC__) || defined(__clang__)
#if __BYTE_ORDER__ != __ORDER_LITTLE_ENDIAN__
#error "only support little endian now"
#endif
#elif defined(_MSC_VER)
#else
#warning "Compiler not checked for endianness, assuming little endian."
#endif


namespace dm::pack {

#ifdef STRUCT_PACK_USE_INT8_SIZE
    using size_type = uint8_t;
    constexpr uint8_t MAX_SIZE = UINT8_MAX;
#elif STRUCT_PACK_USE_INT16_SIZE
    using size_type = uint16_t;
    constexpr uint16_t MAX_SIZE = UINT16_MAX;
#else
    using size_type = uint32_t;
    constexpr uint32_t MAX_SIZE = UINT32_MAX;
#endif

    template <typename T>
    struct compatible;
    template <typename T>
    struct deserialize_result;

    namespace detail {

        template <typename T>
        inline constexpr bool dm_pack_byte_v = dm_is_same_v<char, T> || dm_is_same_v<unsigned char, T> || dm_is_same_v<std::byte, T>;

        template <typename T>
        inline constexpr bool dm_pack_buffer_v = dm_pack_trivially_copyable_container_v<T> && dm_pack_byte_v<typename T::value_type>;

        namespace {
            struct GetTypesFunctor {
                template <typename... Args>
                DMPACK_CONSTEXPR_INLINE_LAMBDA auto operator()(Args &&...) const {
                    return std::tuple<dm_remove_cvref_t<Args>...>{};
                }
            };
        }

        template <class U>
        constexpr auto get_types(U&& t) {
            using T = dm_remove_cvref_t<U>;
            if constexpr (dm_is_fundamental_v<T> || dm_is_enum_v<T> || dm_pack_is_std_basic_string_v<T> ||
                (dm_is_container_v<T> && !dm_pack_is_std_basic_string_v<T>) ||
                dm_is_optional_v<T> || dm_is_variant_v<T> || dm_pack_expected_v<T> ||
                dm_is_std_array_v<T> || dm_is_c_array_v<T> || dm_is_monostate_v<T>) {
                return std::tuple<T>{};
            }
            else if constexpr (dm_is_tuple_v<T>) {
                return T{};
            }
            else if constexpr (dm_is_pair_v<T>) {
                return std::tuple<typename T::first_type, typename T::second_type>{};
            }
            else if constexpr (dm_is_aggregate_v<T>) {
                return visit_members(std::forward<U>(t), GetTypesFunctor{});
            }
            else {
                static_assert(!sizeof(T), "the type is not supported!");
            }
        }

        enum class type_id {
            compatible_t = 0,
            int32_t = 1, uint32_t, int64_t, uint64_t, int8_t, uint8_t, int16_t, uint16_t,
            int128_t, uint128_t, bool_t, char_8_t, char_16_t, char_32_t, w_char_t,
            float16_t, float32_t, float64_t, float128_t,
            string_t = 128, array_t, map_container_t, set_container_t, container_t,
            optional_t, variant_t, expected_t,
            monostate_t = 253, aggregate_class_t = 254, type_end_flag = 255,
        };

        template <typename T>
        constexpr type_id get_integral_type() {
            if constexpr (dm_is_same_v<int32_t, T>) { return type_id::int32_t; }
            else if constexpr (dm_is_same_v<uint32_t, T>) { return type_id::uint32_t; }
            else if constexpr (dm_is_same_v<int64_t, T> || (sizeof(long long) == 8 && dm_is_same_v<T, long long>)) { return type_id::int64_t; }
            else if constexpr (dm_is_same_v<uint64_t, T> || (sizeof(unsigned long long) == 8 && dm_is_same_v<T, unsigned long long>)) { return type_id::uint64_t; }
            else if constexpr (dm_is_same_v<int8_t, T> || dm_is_same_v<signed char, T>) { return type_id::int8_t; }
            else if constexpr (dm_is_same_v<uint8_t, T> || dm_is_same_v<unsigned char, T>) { return type_id::uint8_t; }
            else if constexpr (dm_is_same_v<int16_t, T>) { return type_id::int16_t; }
            else if constexpr (dm_is_same_v<uint16_t, T>) { return type_id::uint16_t; }
            else if constexpr (dm_is_same_v<char, T>
#if __cplusplus >= 202002L
                || dm_is_same_v<char8_t, T>
#endif
                ) {
                return type_id::char_8_t;
            }
#ifdef STRUCT_PACK_ENABLE_UNPORTABLE_TYPE
            else if constexpr (dm_is_same_v<wchar_t, T>) { return type_id::w_char_t; }
#endif
            else if constexpr (dm_is_same_v<char16_t, T>) { static_assert(sizeof(char16_t) == 2, "sizeof(char16_t)!=2"); return type_id::char_16_t; }
            else if constexpr (dm_is_same_v<char32_t, T> && sizeof(char32_t) == 4) { static_assert(sizeof(char32_t) == 4, "sizeof(char32_t)!=4"); return type_id::char_32_t; }
            else if constexpr (dm_is_same_v<bool, T> && sizeof(bool)) { static_assert(sizeof(bool) == 1, "sizeof(bool)!=1"); return type_id::bool_t; }
            else {
                static_assert(!dm_is_same_v<wchar_t, T>, "Tips: Add macro STRUCT_PACK_ENABLE_UNPORTABLE_TYPE to support wchar_t");
                static_assert(!dm_is_same_v<long, T> && !dm_is_same_v<unsigned long, T>, "The long types have different width. Please use fixed width integer types like int64_t.");
                static_assert(!sizeof(T), "not supported integral type");
            }
        }

        template <typename T>
        constexpr type_id get_floating_point_type() {
            if constexpr (dm_is_same_v<float, T>) {
                if constexpr (!std::numeric_limits<float>::is_iec559 || sizeof(float) != 4) { static_assert(!sizeof(T), "The float is not standard 32bits float point!"); }
                return type_id::float32_t;
            }
            else if constexpr (dm_is_same_v<double, T>) {
                if constexpr (!std::numeric_limits<double>::is_iec559 || sizeof(double) != 8) { static_assert(!sizeof(T), "The double is not standard 64bits float point!"); }
                return type_id::float64_t;
            }
            else if constexpr (dm_is_same_v<long double, T>) {
                if constexpr (sizeof(long double) != 16 || !std::numeric_limits<long double>::is_iec559) { static_assert(!sizeof(T), "The long double is not standard 128bits float point!"); }
                return type_id::float128_t;
            }
            else { static_assert(!sizeof(T), "not supported float type"); }
        }


        template <typename T>
        constexpr type_id get_type_id() {
            static_assert(CHAR_BIT == 8);
            using U = dm_remove_cvref_t<T>;
            if constexpr (dm_is_enum_v<U>) { return get_integral_type<dm_underlying_type_t<U>>(); }
            else if constexpr (dm_is_integral_v<U>) { return get_integral_type<U>(); }
            else if constexpr (dm_is_floating_point_v<U>) { return get_floating_point_type<U>(); }
            else if constexpr (dm_is_monostate_v<U> || dm_is_void_v<U>) { return type_id::monostate_t; }
            else if constexpr (dm_pack_string_v<U>) { return type_id::string_t; }
            else if constexpr (dm_is_any_array_v<U>) { return type_id::array_t; }
            else if constexpr (dm_is_map_container_v<U>) { return type_id::map_container_t; }
            else if constexpr (dm_is_set_container_v<U>) { return type_id::set_container_t; }
            else if constexpr (dm_is_container_v<U>) { return type_id::container_t; }
            else if constexpr (dm_is_optional_v<U>) { return type_id::optional_t; }
            else if constexpr (dm_is_variant_v<U>) {
                static_assert(std::variant_size_v<U> > 1 || (std::variant_size_v<U> == 1 && !dm_is_monostate_v<std::variant_alternative_t<0, U>>), "The variant should contain's at least one type!");
                static_assert(std::variant_size_v<U> < 256, "The variant is too complex!");
                return type_id::variant_t;
            }
            else if constexpr (dm_pack_expected_v<U>) { return type_id::expected_t; }
            else if constexpr (dm_is_tuple_like_v<U>) { return type_id::aggregate_class_t; }
            else if constexpr (dm_is_class_v<U>) {
                static_assert(dm_is_aggregate_v<U>);
                return type_id::aggregate_class_t;
            }
            else { static_assert(!sizeof(T), "not supported type"); }
        }

        template <size_t size>
        constexpr decltype(auto) get_size_literal() {
            static_assert(sizeof(size_t) <= 8);
            return string_literal<char, 8>{{(char)(size >> 56), (char)((size >> 48) % 256), (char)((size >> 40) % 256), (char)((size >> 32) % 256), (char)((size >> 24) % 256), (char)((size >> 16) % 256), (char)((size >> 8) % 256), (char)(size % 256)}};
        }

        template <typename Args, typename ParentArg, std::size_t... I>
        constexpr decltype(auto) get_type_literal_impl(std::index_sequence<I...>);

        template <typename Args, std::size_t... I>
        constexpr decltype(auto) get_variant_literal_impl(std::index_sequence<I...>);

        template <typename Arg, typename ParentArg>
        constexpr decltype(auto) get_type_literal() {
            constexpr auto id = get_type_id<Arg>();
            constexpr auto ret = string_literal<char, 1>{ {static_cast<char>(id)} };
            if constexpr (id == type_id::monostate_t) {
                if constexpr (dm_pack_expected_v<ParentArg>) {
                    static_assert(dm_is_void_v<typename ParentArg::value_type> && !dm_is_void_v<typename ParentArg::error_type>, "void is only allowed as expected's value_type");
                }
                else {
                    static_assert(dm_is_void_v<ParentArg> || dm_is_variant_v<ParentArg>, "monostate/void only allowed in variant or as expected's value_type");
                }
            }
            if constexpr (id == type_id::aggregate_class_t) {
                using Args = decltype(get_types(Arg{}));
                constexpr auto body = get_type_literal_impl<Args, Arg>(dm_make_index_sequence<std::tuple_size_v<Args>>());
                constexpr auto end = string_literal<char, 1>{ {static_cast<char>(type_id::type_end_flag)} };
                return ret + body + end;
            }
            else if constexpr (id == type_id::variant_t) {
                constexpr auto sz = std::variant_size_v<Arg>;
                static_assert(sz > 0, "empty param of std::variant is not allowed!");
                static_assert(sz < 256, "too many alternative type in variant!");
                constexpr auto body = get_variant_literal_impl<Arg>(dm_make_index_sequence<std::variant_size_v<Arg>>());
                constexpr auto end = string_literal<char, 1>{ {static_cast<char>(type_id::type_end_flag)} };
                return ret + body + end;
            }
            else if constexpr (id == type_id::array_t) {
                constexpr auto sz = dm_get_array_size_v<Arg>;
                return ret + get_type_literal<dm_element_type_t<Arg>, Arg>() + get_size_literal<sz>();
            }
            else if constexpr (id == type_id::container_t || id == type_id::optional_t || id == type_id::string_t) {
                return ret + get_type_literal<dm_element_type_t<Arg>, Arg>();
            }
            else if constexpr (id == type_id::set_container_t) {
                return ret + get_type_literal<typename Arg::key_type, Arg>();
            }
            else if constexpr (id == type_id::map_container_t) {
                return ret + get_type_literal<typename Arg::key_type, Arg>() + get_type_literal<typename Arg::mapped_type, Arg>();
            }
            else if constexpr (id == type_id::expected_t) {
                return ret + get_type_literal<typename Arg::value_type, Arg>() + get_type_literal<typename Arg::error_type, Arg>();
            }
            else if constexpr (id != type_id::compatible_t) { return ret; }
            else { return string_literal<char, 0>{}; }
        }

        template <typename Args, typename ParentArg, std::size_t... I>
        constexpr decltype(auto) get_type_literal_impl(std::index_sequence<I...>) {
            return ((get_type_literal<dm_remove_cvref_t<std::tuple_element_t<I, Args>>, ParentArg>()) + ...);
        }

        template <typename Args, std::size_t... I>
        constexpr decltype(auto) get_variant_literal_impl(std::index_sequence<I...>) {
            return ((get_type_literal<dm_remove_cvref_t<std::variant_alternative_t<I, Args>>, Args>()) + ...);
        }

        template <typename... Args>
        constexpr decltype(auto) get_types_literal() {
            return (get_type_literal<Args, void>() + ...);
        }

        template <size_t depth, typename... Args>
        constexpr int check_if_compatible_element_exist();

        template <std::size_t depth, typename Args, std::size_t... I>
        constexpr decltype(auto) check_if_compatible_element_exist_help_coverage(std::index_sequence<I...>) {
            return check_if_compatible_element_exist<depth + 1, dm_remove_cvref_t<std::tuple_element_t<I, Args>>...>();
        }

        template <size_t depth, typename Arg>
        constexpr int check_if_compatible_element_exist_helper(bool& flag) {
            constexpr auto id = get_type_id<Arg>();
            if constexpr (id == type_id::compatible_t) {
                flag = false;
                if (depth) return -1;
                return 1;
            }
            else {
                if (!flag) return -1;
                if constexpr (id == type_id::aggregate_class_t) {
                    using subArgs = decltype(get_types(Arg{}));
                    return check_if_compatible_element_exist_help_coverage<depth, subArgs>(dm_make_index_sequence<std::tuple_size_v<subArgs>>());
                }
                else { return 0; }
            }
        }

        template <size_t depth, typename... Args>
        constexpr int check_if_compatible_element_exist() {
            bool flag = true;
            int ret = 0;
            (
                [&]() {
                    auto tmp = check_if_compatible_element_exist_helper<depth, Args>(flag);
                    if (tmp == -1) { ret = -1; }
                    else if (tmp == 1 && ret != -1) { ret = 1; }
                    return ret;
                }(), ...);
            return ret;
        }

        template <typename... Args>
        constexpr uint32_t get_types_code_impl() {
            constexpr auto str = get_types_literal<dm_remove_cvref_t<Args>...>();
            constexpr auto ret = check_if_compatible_element_exist<0, dm_remove_cvref_t<Args>...>();
            static_assert(ret == 0 || ret == 1, "The relative position of compatible<T> in struct is not allowed!");
            auto md5 = MD5::MD5Hash32Constexpr(str.data(), str.size());
            auto types_code = md5 - (md5 % 2) + ret;
            return types_code;
        }

        template <typename T, size_t... I>
        constexpr int check_if_compatible_element_exist(std::index_sequence<I...>) {
            return check_if_compatible_element_exist<0, std::tuple_element_t<I, T>...>();
        }

        template <typename T, size_t... I>
        constexpr uint32_t get_types_code(std::index_sequence<I...>) {
            return get_types_code_impl<std::tuple_element_t<I, T>...>();
        }

        [[noreturn]] DMPACK_INLINE void exit_container_size() {
            std::cerr << "Serialize Error! The container's size is greater than " << MAX_SIZE << std::endl;
            std::exit(EXIT_FAILURE);
        }
        [[noreturn]] DMPACK_INLINE void exit_valueless_variant() {
            std::cerr << "Serialize Error! The variant is valueless!" << std::endl;
            std::exit(EXIT_FAILURE);
        }

        template <typename T, typename... Args>
        constexpr std::size_t DMPACK_INLINE calculate_needed_size(const T& item, const Args &...items);

        constexpr std::size_t DMPACK_INLINE calculate_needed_size() { return 0; }

        template <typename T>
        constexpr std::size_t DMPACK_INLINE calculate_one_size(const T& item) {
            using type = dm_remove_cvref_t<decltype(item)>;
            static_assert(!dm_is_pointer_v<type>);
            std::size_t total = 0;
            if constexpr (dm_is_monostate_v<type>) {}
            else if constexpr (dm_is_fundamental_v<type> || dm_is_enum_v<type>) { total += sizeof(type); }
            else if constexpr (dm_is_same_v<std::string, type>) { total += (item.size() + sizeof(size_type)); }
            else if constexpr (dm_is_c_array_v<type> || dm_is_std_array_v<type>) {
                if constexpr (dm_is_trivially_copyable_v<type>) { total += sizeof(type); }
                else { for (auto& i : item) { total += calculate_one_size(i); } }
            }
            else if constexpr (dm_is_map_container_v<type> || (dm_is_container_v<type> && !dm_pack_string_v<type>)) {
                total += sizeof(size_type);
                if constexpr (dm_pack_trivially_copyable_container_v<type>) { total += item.size() * sizeof(typename type::value_type); }
                else { for (auto&& i : item) { total += calculate_one_size(i); } }
            }
            else if constexpr (dm_is_tuple_like_v<type>) {
                if constexpr (dm_is_pair_v<type>) {
                    total += calculate_one_size(item.first);
                    total += calculate_one_size(item.second);
                }
                else {
                    std::apply([&](auto &&...items) DMPACK_CONSTEXPR_INLINE_LAMBDA{ total += calculate_needed_size(items...); }, item);
                }
            }
            else if constexpr (dm_is_optional_v<type>) {
                total += sizeof(char);
                if (item.has_value()) { total += calculate_one_size(*item); }
            }
            else if constexpr (dm_is_variant_v<type>) {
                total += sizeof(uint32_t);
                if (item.index() != std::variant_npos) [[likely]] {
                    total += std::visit([](const auto& e) -> std::size_t { return calculate_one_size(e); }, item);
                }
                else [[unlikely]] { exit_valueless_variant(); }
            }
            else if constexpr (dm_pack_expected_v<type>) {
                total += sizeof(bool);
                if (item.has_value()) {
                    if constexpr (!dm_is_void_v<typename type::value_type>) total += calculate_one_size(item.value());
                }
                else { total += calculate_one_size(item.error()); }
            }
            else if constexpr (dm_is_class_v<type>) {
                if constexpr (dm_is_trivially_copyable_v<type>) { total += sizeof(type); }
                else { visit_members(item, [&](auto &&...items) DMPACK_CONSTEXPR_INLINE_LAMBDA{ total += calculate_needed_size(items...); }); }
            }
            else { static_assert(!sizeof(type), "the type is not supported yet"); }
            return total;
        }

        template <typename Key, typename Value>
        constexpr std::size_t DMPACK_INLINE calculate_one_size(const std::pair<Key, Value>& item) {
            return calculate_one_size(item.first) + calculate_one_size(item.second);
        }

        template <typename T, typename... Args>
        constexpr std::size_t DMPACK_INLINE calculate_needed_size(const T& item, const Args &...items) {
            return calculate_one_size(item) + calculate_needed_size(items...);
        }

        template <typename T>
        constexpr uint32_t get_types_code() {
            return detail::get_types_code<T>(dm_make_index_sequence<std::tuple_size_v<T>>{});
        }

        template <typename T>
        constexpr int check_if_compatible_element_exist() {
            return detail::check_if_compatible_element_exist<T>(dm_make_index_sequence<std::tuple_size_v<T>>{});
        }

        template <typename T>
        inline constexpr bool exist_compatible_member_v = check_if_compatible_element_exist<decltype(get_types(dm_remove_cvref_t<T>{})) > () == 1;

        template <typename T>
        inline constexpr bool unexist_compatible_member_v = check_if_compatible_element_exist<decltype(get_types(dm_remove_cvref_t<T>{})) > () == 0;

        template <typename Byte>
        class packer {
        public:
            packer(Byte* data) : data_(data) {}
            packer(const packer&) = delete;
            packer& operator=(const packer&) = delete;

            template <typename T, typename... Args>
            DMPACK_INLINE void serialize(const T& t, const Args &...args) {
                if constexpr (sizeof...(args) == 0) {
                    constexpr uint32_t types_code = get_types_code<decltype(get_types(t))>();
                    std::memcpy(data_ + pos_, &types_code, sizeof(uint32_t));
                    pos_ += sizeof(uint32_t);
                    serialize_one(t);
                }
                else {
                    constexpr uint32_t types_code = get_types_code<std::tuple<dm_remove_cvref_t<T>, dm_remove_cvref_t<Args>...>>();
                    std::memcpy(data_ + pos_, &types_code, sizeof(uint32_t));
                    pos_ += sizeof(uint32_t);
                    serialize_many(t, args...);
                }
            }

            template <typename T, typename... Args>
            DMPACK_INLINE void serialize_with_size(uint64_t sz, const T& t, const Args &...args) {
                if constexpr (sizeof...(args) == 0) {
                    constexpr uint32_t types_code = get_types_code<decltype(get_types(t))>();
                    std::memcpy(data_ + pos_, &types_code, sizeof(uint32_t));
                    std::memcpy(data_ + pos_ + sizeof(uint32_t), &sz, sizeof(uint64_t));
                    pos_ += sizeof(uint32_t) + sizeof(uint64_t);
                    serialize_one(t);
                }
                else {
                    constexpr uint32_t types_code = get_types_code<std::tuple<dm_remove_cvref_t<T>, dm_remove_cvref_t<Args>...>>();
                    std::memcpy(data_ + pos_, &types_code, sizeof(uint32_t));
                    std::memcpy(data_ + pos_ + sizeof(uint32_t), &sz, sizeof(uint64_t));
                    pos_ += sizeof(uint32_t) + sizeof(uint64_t);
                    serialize_many(t, args...);
                }
            }

            DMPACK_INLINE const Byte* data() { return data_; }
            DMPACK_INLINE size_t size() { return pos_; }

        private:
            template<typename T, typename... Args>
            constexpr void DMPACK_INLINE serialize_many(const T& first_item, const Args &...items) {
                serialize_one(first_item);
                if constexpr (sizeof...(items) > 0) {
                    serialize_many(items...);
                }
            }

            template<typename T>
            constexpr void DMPACK_INLINE serialize_one(const T& item) {
                using type = dm_remove_cvref_t<decltype(item)>;
                static_assert(!dm_is_pointer_v<type>);
                if constexpr (dm_is_monostate_v<type>) {}
                else if constexpr (dm_is_fundamental_v<type> || dm_is_enum_v<type>) {
                    std::memcpy(data_ + pos_, &item, sizeof(type));
                    pos_ += sizeof(type);
                }
                else if constexpr (dm_is_c_array_v<type> || dm_is_std_array_v<type>) {
                    if constexpr (dm_is_trivially_copyable_v<type>) {
                        std::memcpy(data_ + pos_, &item, sizeof(type));
                        pos_ += sizeof(type);
                    }
                    else {
                        for (auto& i : item) { serialize_one(i); }
                    }
                }
                else if constexpr (dm_is_map_container_v<type> || dm_is_container_v<type>) {
                    if (item.size() > MAX_SIZE) [[unlikely]] { exit_container_size(); }
                    size_type size = item.size();
                    std::memcpy(data_ + pos_, &size, sizeof(size_type));
                    pos_ += sizeof(size_type);

                    if constexpr (dm_pack_trivially_copyable_container_v<type>) {
                        using value_type = typename type::value_type;
                        auto sz = item.size() * sizeof(value_type);
                        std::memcpy(data_ + pos_, item.data(), sz);

                        pos_ += sz;
                        return;
                    }
                    for (auto&& i : item) { serialize_one(i); }
                }
                else if constexpr (dm_is_tuple_like_v<type>) {
                    if constexpr (dm_is_pair_v<type>) {
                        serialize_one(item.first);
                        serialize_one(item.second);
                    }
                    else {
                        std::apply([this](auto &&...items) DMPACK_CONSTEXPR_INLINE_LAMBDA{ this->serialize_many(items...); }, item);
                    }
                }
                else if constexpr (dm_is_optional_v<type>) {
                    bool has_value = item.has_value();
                    std::memcpy(data_ + pos_, &has_value, sizeof(char));
                    pos_ += sizeof(char);
                    if (has_value) { serialize_one(*item); }
                }
                else if constexpr (dm_is_variant_v<type>) {
                    if (item.index() == std::variant_npos) [[unlikely]] { exit_valueless_variant(); }
                    else {
                        uint32_t index = item.index();
                        std::memcpy(data_ + pos_, &index, sizeof(index));
                        pos_ += sizeof(index);
                        std::visit([this](auto&& e) { this->serialize_one(e); }, item);
                    }
                }
                else if constexpr (dm_pack_expected_v<type>) {
                    bool has_value = item.has_value();
                    std::memcpy(data_ + pos_, &has_value, sizeof(has_value));
                    pos_ += sizeof(has_value);
                    if (has_value) {
                        if constexpr (!dm_is_void_v<typename type::value_type>) serialize_one(item.value());
                    }
                    else {
                        serialize_one(item.error());
                    }
                }
                else if constexpr (dm_is_class_v<type>) {
                    static_assert(dm_is_aggregate_v<dm_remove_cvref_t<type>>);
                    if constexpr (dm_is_trivially_copyable_v<type>) {
                        std::memcpy(data_ + pos_, &item, sizeof(type));
                        pos_ += sizeof(type);
                    }
                    else {
                        visit_members(item, [this](auto &&...items) DMPACK_CONSTEXPR_INLINE_LAMBDA{ this->serialize_many(items...); });
                    }
                }
                else { static_assert(!sizeof(type), "the type is not supported yet"); }
                return;
            }

            Byte* data_;
            std::size_t pos_{};
        };

        template <typename Byte>
        class unpacker {
        public:
            unpacker() = delete;
            unpacker(const unpacker&) = delete;
            unpacker& operator=(const unpacker&) = delete;

            DMPACK_INLINE unpacker(const Byte* data, std::size_t size)
                : data_{ data }, size_(size) {
            }

            template <class T>
            DMPACK_INLINE std::errc deserialize(T& t) {
                auto&& [err_code, data_len] = check_types(t);
                if (err_code != std::errc{}) [[unlikely]] {
                    return err_code;
                }
                return deserialize_one(t);
            }

            template <class T>
            DMPACK_INLINE std::errc deserialize(T& t, std::size_t& len) {
                auto&& [err_code, data_len] = check_types(t);
                if (err_code != std::errc{}) [[unlikely]] {
                    return err_code;
                }
                auto ret = deserialize_one(t);
                len = (ret == std::errc{} ? std::max(pos_, data_len) : 0);
                return ret;
            }

            template <typename U, size_t I>
            DMPACK_INLINE
                ::dm::pack::deserialize_result<dm_remove_cvref_t<std::tuple_element_t<
                I, decltype(get_types(std::declval<dm_remove_cvref_t<U>>()))>>>
                get_field() {
                using T = dm_remove_cvref_t<U>;

                T t{};
                using types = decltype(get_types(t));
                using Filed = dm_remove_cvref_t<std::tuple_element_t<I, types>>;

                std::errc code{};
                Filed field{};
                if (auto [err_code, _] = check_types(t); err_code != std::errc{}) [[unlikely]] {
                    return { err_code, field };
                }

                if constexpr (dm_is_tuple_v<T>) {
                    std::apply(
                        [this, &code, &field](auto &&...items) DMPACK_CONSTEXPR_INLINE_LAMBDA{
                          static_assert(I < sizeof...(items), "out of range");
                          code = this->template for_each<I>(field, items...);
                        },
                        t);
                }
                else if constexpr (dm_is_class_v<T>) {
                    static_assert(dm_is_aggregate_v<T>);
                    visit_members(t, [this, &code, &field](auto &&...items) DMPACK_CONSTEXPR_INLINE_LAMBDA{
                      static_assert(I < sizeof...(items), "out of range");
                      code = this->template for_each<I>(field, items...);
                        });
                }

                pos_ = 0;
                return { code, std::move(field) };
            }

        private:
            template <size_t index, typename unpack, typename variant_t>
            struct variant_construct_helper_not_skipped {
                static DMPACK_INLINE void run(unpack& unpacker, variant_t& v) {
                    if constexpr (index >= std::variant_size_v<variant_t>) {
                        return;
                    }
                    else {
                        v.template emplace<index>();
                        unpacker.template deserialize_one<true>(std::get<index>(v));
                    }
                }
            };

            template <size_t index, typename unpack, typename variant_t>
            struct variant_construct_helper_skipped {
                static DMPACK_INLINE void run(unpack& unpacker, variant_t& v) {
                    if constexpr (index >= std::variant_size_v<variant_t>) {
                        return;
                    }
                    else {
                        v.template emplace<index>();
                        unpacker.template deserialize_one<false>(std::get<index>(v));
                    }
                };
            };

            template <class T>
            DMPACK_INLINE std::pair<std::errc, std::size_t> check_types(T& t) {
                if (size_ < sizeof(uint32_t)) [[unlikely]] {
                    return { std::errc::no_buffer_space, 0 };
                }

                constexpr uint32_t types_code = get_types_code<decltype(get_types(t))>();
                uint32_t current_types_code{};
                std::memcpy(&current_types_code, data_ + pos_, sizeof(uint32_t));
                if ((current_types_code / 2) != (types_code / 2)) [[unlikely]] {
                    return { std::errc::invalid_argument, 0 };
                }
                pos_ += sizeof(uint32_t);
                if (current_types_code % 2)
                {
                    if (size_ < sizeof(uint64_t) + sizeof(uint32_t)) [[unlikely]] {
                        return { std::errc::no_buffer_space, 0 };
                    }
                    uint64_t data_len;
                    std::memcpy(&data_len, data_ + pos_, sizeof(uint64_t));
                    if (data_len > size_) [[unlikely]] {
                        return { std::errc::no_buffer_space, 0 };
                    }
                    pos_ += sizeof(uint64_t);
                    return { std::errc{}, data_len };
                }
                return { {}, 0 };
            }

            template <bool NotSkip = true>
            constexpr std::errc DMPACK_INLINE deserialize_many() {
                return {};
            }

            template <bool NotSkip = true, typename T, typename... Args>
            constexpr std::errc DMPACK_INLINE deserialize_many(T& first_item,
                Args &...items) {
                auto code = deserialize_one<NotSkip>(first_item);
                if (code != std::errc{}) [[unlikely]] {
                    return code;
                }
                if constexpr (sizeof...(items) > 0) {
                    return deserialize_many<NotSkip>(items...);
                }
                return std::errc{};
            }

            template <bool NotSkip = true, typename T>
            constexpr std::errc DMPACK_INLINE deserialize_one(T& item) {
                std::errc code{};
                using type = dm_remove_cvref_t<decltype(item)>;
                static_assert(!dm_is_pointer_v<type>);
                if constexpr (dm_is_monostate_v<type>) {}
                else if constexpr (dm_is_fundamental_v<type> || dm_is_enum_v<type>) {
                    if (pos_ + sizeof(type) > size_) [[unlikely]] { return std::errc::no_buffer_space; }
                    if constexpr (NotSkip) { std::memcpy(&item, data_ + pos_, sizeof(type)); }
                    pos_ += sizeof(type);
                }
                else if constexpr (dm_is_c_array_v<type> || dm_is_std_array_v<type>) {
                    if constexpr (dm_is_trivially_copyable_v<type>) {
                        if (pos_ + sizeof(type) > size_) [[unlikely]] { return std::errc::no_buffer_space; }
                        if constexpr (NotSkip) { std::memcpy(&item, data_ + pos_, sizeof(type)); }
                        pos_ += sizeof(type);
                    }
                    else {
                        for (auto& i : item) {
                            code = deserialize_one<NotSkip>(i);
                            if (code != std::errc{}) [[unlikely]] { return code; }
                        }
                    }
                }
                else if constexpr (dm_is_map_container_v<type>) {
                    size_type container_size = 0;
                    if (pos_ + sizeof(size_type) > size_) [[unlikely]] { return std::errc::no_buffer_space; }
                    std::memcpy(&container_size, data_ + pos_, sizeof(size_type));
                    pos_ += sizeof(size_type);
                    if (container_size == 0) [[likely]] { return {}; }
                    if constexpr (NotSkip) { item.clear(); }
                    using key_type = typename type::key_type;
                    using value_type = typename type::mapped_type;
                    for (size_t i = 0; i < container_size; ++i) {
                        std::pair<key_type, value_type> pair{};
                        code = deserialize_one<NotSkip>(pair);
                        if (code != std::errc{}) [[unlikely]] { return code; }
                        if constexpr (NotSkip) { item.emplace(std::move(pair)); }
                    }
                }
                else if constexpr (dm_is_container_v<type>) {
                    size_type container_size = 0;
                    if (pos_ + sizeof(size_type) > size_) [[unlikely]] { return std::errc::no_buffer_space; }
                    std::memcpy(&container_size, data_ + pos_, sizeof(size_type));
                    pos_ += sizeof(size_type);
                    if (container_size == 0) [[likely]] { return {}; }
                    if constexpr (NotSkip) { item.clear(); }

                    if constexpr (dm_is_set_container_v<type>) {
                        typename type::value_type value;
                        for (size_t i = 0; i < container_size; ++i) {
                            code = deserialize_one<NotSkip>(value);
                            if (code != std::errc{}) [[unlikely]] { return code; }
                            if constexpr (NotSkip) { item.emplace(std::move(value)); }
                        }
                    }
                    else {
                        using value_type = typename type::value_type;
                        size_t mem_sz = container_size * sizeof(value_type);
                        if constexpr (dm_pack_trivially_copyable_container_v<type>) {
                            if (pos_ + mem_sz > size_) [[unlikely]] { return std::errc::no_buffer_space; }
                            if constexpr (NotSkip) {
                                if constexpr (dm_pack_string_view_v<type>) { item = { reinterpret_cast<const char*>(data_ + pos_), container_size }; }
                                else {
                                    item.resize(container_size);
                                    std::memcpy(item.data(), data_ + pos_, mem_sz);
                                }
                            }
                            pos_ += mem_sz;
                        }
                        else {
                            if constexpr (NotSkip) { item.resize(container_size); }
                            for (size_t i = 0; i < container_size; ++i) {
                                if constexpr (NotSkip) {
                                    code = deserialize_one<NotSkip>(item[i]);
                                }
                                else {
                                    value_type useless;
                                    code = deserialize_one<NotSkip>(useless);
                                }
                                if (code != std::errc{}) [[unlikely]] { return code; }
                            }
                        }
                    }
                }
                else if constexpr (dm_is_tuple_like_v<type>) {
                    if constexpr (dm_is_pair_v<type>) {
                        code = deserialize_one<NotSkip>(item.first);
                        if (code != std::errc{}) return code;
                        code = deserialize_one<NotSkip>(item.second);
                    }
                    else {
                        std::apply([this](auto &&...items) DMPACK_CONSTEXPR_INLINE_LAMBDA{ this->template deserialize_many<NotSkip>(items...); }, item);
                    }
                }
                else if constexpr (dm_is_optional_v<type>) {
                    if (pos_ + sizeof(bool) > size_) [[unlikely]] { return std::errc::no_buffer_space; }
                    bool has_value{};
                    std::memcpy(&has_value, data_ + pos_, sizeof(bool));
                    pos_ += sizeof(bool);
                    if (!has_value) [[unlikely]] { if constexpr (NotSkip) { item.reset(); } return {}; }
                    if constexpr (NotSkip) { item.emplace(); code = deserialize_one<NotSkip>(item.value()); }
                    else { typename type::value_type val; code = deserialize_one<NotSkip>(val); }
                }
                else if constexpr (dm_is_variant_v<type>) {
                    if (pos_ + sizeof(uint32_t) > size_) [[unlikely]] { return std::errc::no_buffer_space; }
                    uint32_t index;
                    std::memcpy(&index, data_ + pos_, sizeof(index));
                    pos_ += sizeof(index);
                    if (index >= std::variant_size_v<type>) [[unlikely]] { return std::errc::invalid_argument; }
                    if constexpr (NotSkip) { template_switch<variant_construct_helper_not_skipped>(index, *this, item); }
                    else { template_switch<variant_construct_helper_skipped>(index, *this, item); }
                }
                else if constexpr (dm_pack_expected_v<type>) {
                    if (pos_ + sizeof(bool) > size_) [[unlikely]] { return std::errc::no_buffer_space; }
                    bool has_value{};
                    std::memcpy(&has_value, data_ + pos_, sizeof(bool));
                    pos_ += sizeof(bool);
                    if (has_value) {
                        if constexpr (!dm_is_void_v<typename type::value_type>) {
                            if constexpr (NotSkip) { if (!item.has_value()) item.emplace(); code = deserialize_one<NotSkip>(item.value()); }
                            else { typename type::value_type val; code = deserialize_one<NotSkip>(val); }
                        }
                    }
                    else {
                        typename type::error_type err_val;
                        code = deserialize_one<NotSkip>(err_val);
                        if constexpr (NotSkip) { item = typename type::unexpected_type{ std::move(err_val) }; }
                    }
                }
                else if constexpr (dm_is_class_v<type>) {
                    static_assert(dm_is_aggregate_v<dm_remove_cvref_t<type>>);
                    if constexpr (dm_is_trivially_copyable_v<type>) {
                        if (pos_ + sizeof(type) > size_) [[unlikely]] { return std::errc::no_buffer_space; }
                        if constexpr (NotSkip) { std::memcpy(&item, data_ + pos_, sizeof(type)); }
                        pos_ += sizeof(type);
                    }
                    else {
                        visit_members(item, [this](auto &&...items) DMPACK_CONSTEXPR_INLINE_LAMBDA{ this->template deserialize_many<NotSkip>(items...); });
                    }
                }
                else { static_assert(!sizeof(type), "the type is not supported yet"); }
                return code;
            }

            template <size_t I, size_t FiledIndex, typename FiledType, typename T>
            DMPACK_INLINE bool set_value(std::errc& err_code, FiledType& field, T&& t) {
                if constexpr (FiledIndex == I) {
                    static_assert(dm_is_same_v<dm_remove_cvref_t<FiledType>, dm_remove_cvref_t<T>>);
                    err_code = deserialize_one<true>(field);
                    return true;
                }
                err_code = deserialize_one<false>(t);
                return false;
            }

            template <size_t FiledIndex, typename FiledType, typename... Args>
            DMPACK_INLINE constexpr decltype(auto) for_each(FiledType& field, Args &&...items) {
                bool stop = false;
                std::errc code{};
                using I_SEQ = dm_make_index_sequence<sizeof...(Args)>;
                auto l = {[&](auto &&... I_items) {
                  auto ll = {[this, &stop, &code, &field](auto i, auto&& item) {
                    if (!stop) {
                      stop = set_value<decltype(i)::value, FiledIndex>(code, field, item);
                    }
                  }};
                  (ll(std::integral_constant<size_t, I_items>{}, items), ...);
                } };
                l(I_SEQ{});
                return code;
            }

            const Byte* data_;
            std::size_t size_;
            std::size_t pos_{};
        };


    }
}
#endif