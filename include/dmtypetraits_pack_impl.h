#ifndef __DMTYPETRAITS_PACK_IMPL_H_INCLUDE__
#define __DMTYPETRAITS_PACK_IMPL_H_INCLUDE__

#include <bit>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <string>
#include <system_error>
#include <type_traits>
#include <vector>
#include <utility>
#include <variant>
#include <optional>

#include "dmtypetraits.h" // 主头文件

#if __cplusplus < 202002L
#error "dmtypetraits_pack_impl.h requires C++20 or later."
#endif

static_assert(std::endian::native == std::endian::little,
              "dmtypetraits_pack_impl only supports little-endian systems currently.");

namespace dm {
namespace pack_detail {

using size_type = uint32_t;
constexpr uint32_t MAX_SIZE = UINT32_MAX;

// ===================================================================================
// START: 增强的编译期类型信息系统
// ===================================================================================

// 扩展后的类型ID枚举
enum class DmTypeId : char {
    Int8 = 'a', UInt8 = 'b', Int16 = 'c', UInt16 = 'd',
    Int32 = 'e', UInt32 = 'f', Int64 = 'g', UInt64 = 'h',
    Bool = 'i', Char = 'j', Float = 'k', Double = 'l',
    Char16 = 'm', Char32 = 'n',
    Monostate = 'o',
    String = 's', 
    Array = 't', 
    Map = 'u', 
    Set = 'v', 
    Container = 'w',
    Optional = 'x', 
    Variant = 'y', 
    Aggregate = '0', 
    EndAggregate = 'Z',
};

// --- 编译期辅助工具 ---

template <size_t Size>
consteval auto encode_size_as_literal() {
    static_assert(sizeof(size_t) <= 8);
    return dm::md5_detail::DmStringLiteral<char, 8>{
        {char(Size >> 56), char((Size >> 48) & 0xFF), char((Size >> 40) & 0xFF), char((Size >> 32) & 0xFF),
         char((Size >> 24) & 0xFF), char((Size >> 16) & 0xFF), char((Size >> 8) & 0xFF), char(Size & 0xFF)}};
}


// --- 增强的类型识别与签名生成 ---

template <typename T>
consteval auto get_type_literal();

template <typename T>
consteval DmTypeId get_type_id() {
    using type = dm_remove_cvref_t<T>;

    if constexpr (std::is_enum_v<type>) {
        return get_type_id<std::underlying_type_t<type>>();
    }
    else if constexpr (std::is_same_v<type, int8_t> || std::is_same_v<type, signed char>) return DmTypeId::Int8;
    else if constexpr (std::is_same_v<type, uint8_t> || std::is_same_v<type, unsigned char>) return DmTypeId::UInt8;
    else if constexpr (std::is_same_v<type, int16_t>) return DmTypeId::Int16;
    else if constexpr (std::is_same_v<type, uint16_t>) return DmTypeId::UInt16;
    else if constexpr (std::is_same_v<type, int32_t>) return DmTypeId::Int32;
    else if constexpr (std::is_same_v<type, uint32_t>) return DmTypeId::UInt32;
    else if constexpr (std::is_same_v<type, int64_t> || (sizeof(long long) == 8 && std::is_same_v<type, long long>)) return DmTypeId::Int64;
    else if constexpr (std::is_same_v<type, uint64_t> || (sizeof(unsigned long long) == 8 && std::is_same_v<type, unsigned long long>)) return DmTypeId::UInt64;
    else if constexpr (std::is_same_v<type, bool>) return DmTypeId::Bool;
    else if constexpr (std::is_same_v<type, char> || std::is_same_v<type, char8_t>) return DmTypeId::Char;
    else if constexpr (std::is_same_v<type, char16_t>) return DmTypeId::Char16;
    else if constexpr (std::is_same_v<type, char32_t>) return DmTypeId::Char32;
    else if constexpr (std::is_same_v<type, float>) return DmTypeId::Float;
    else if constexpr (std::is_same_v<type, double>) return DmTypeId::Double;
    else if constexpr (dm_is_monostate_v<type>) return DmTypeId::Monostate;
    else if constexpr (dm_is_string_like_v<type>) return DmTypeId::String;
    else if constexpr (dm_is_any_array_v<type>) return DmTypeId::Array;
    else if constexpr (dm_is_map_v<type>) return DmTypeId::Map;
    else if constexpr (dm_is_set_v<type>) return DmTypeId::Set;
    else if constexpr (dm_is_optional_v<type>) return DmTypeId::Optional;
    else if constexpr (dm_is_variant_v<type>) {
        static_assert(std::variant_size_v<type> > 0, "The variant should contain at least one type!");
        static_assert(std::variant_size_v<type> < 256, "The variant is too complex!");
        return DmTypeId::Variant;
    }
    // 注意: container 检查必须在 optional, variant, string 等更特殊的容器之后
    else if constexpr (dm_is_container_v<type>) return DmTypeId::Container;
    else if constexpr (dm_is_tuple_v<type> || dm_is_pair_v<type>) {
        return DmTypeId::Aggregate;
    }
    else if constexpr (std::is_class_v<type>) {
        static_assert(std::is_aggregate_v<type>, "Only aggregate class types are supported for serialization.");
        return DmTypeId::Aggregate;
    }
    else {
        static_assert(!sizeof(type), "Unsupported type for serialization.");
        return DmTypeId{};
    }
}

template <typename... Members>
consteval auto get_members_literal() {
    if constexpr (sizeof...(Members) == 0) {
        return dm::md5_detail::DmStringLiteral("");
    } else {
        return (get_type_literal<Members>() + ...);
    }
}

template <typename T>
consteval auto get_type_literal() {
    using type = dm_remove_cvref_t<T>;
    constexpr DmTypeId id = get_type_id<type>();
    constexpr char id_char_array[] = {static_cast<char>(id), '\0'};
    constexpr auto ret = dm::md5_detail::DmStringLiteral(id_char_array);
    constexpr char end_char_array[] = {static_cast<char>(DmTypeId::EndAggregate), '\0'};
    constexpr auto end_literal = dm::md5_detail::DmStringLiteral(end_char_array);

    if constexpr (id == DmTypeId::Aggregate) {
        auto visitor = [&](auto&&... members) {
            return ret + get_members_literal<dm_remove_cvref_t<decltype(members)>...>() + end_literal;
        };
        return dm_visit_members(type{}, visitor);
    } else if constexpr (id == DmTypeId::Variant) {
        return dm_visit_variant(type{}, []<typename... Alternatives>() {
             return ret + (get_type_literal<Alternatives>() + ...) + end_literal;
        });
    } else if constexpr (id == DmTypeId::Array) {
        constexpr auto size_literal = encode_size_as_literal<dm_get_array_size_v<type>>();
        return ret + get_type_literal<dm_element_type_t<type>>() + size_literal;
    } else if constexpr (id == DmTypeId::Map) {
        return ret + get_type_literal<typename type::key_type>() + get_type_literal<typename type::mapped_type>();
    } else if constexpr (id == DmTypeId::Optional || id == DmTypeId::String || id == DmTypeId::Set || id == DmTypeId::Container) {
        // 这些容器都只有一个 value_type
        return ret + get_type_literal<typename type::value_type>();
    }
    else {
        // 基础类型, Monostate 等
        return ret;
    }
}

template <typename... Args>
consteval uint32_t get_types_code() {
    constexpr auto final_literal = (get_type_literal<Args>() + ...);
    return dm::dm_md5_hash32(final_literal.data(), final_literal.size());
}

// ===================================================================================
// END: 增强的编译期类型信息系统
// ===================================================================================

// 前向声明
template <typename T, typename... Args>
constexpr size_t calculate_needed_size(const T& item, const Args&... items);

// 递归基例
constexpr size_t calculate_needed_size() { return 0; }

// 计算大小的主递归函数
template <typename T, typename... Args>
constexpr size_t calculate_needed_size(const T& item, const Args&... items) {
    using type = dm_remove_cvref_t<T>;
    static_assert(!std::is_pointer_v<type>, "Raw pointers cannot be serialized.");
    size_t current_size = 0;

    if constexpr (std::is_fundamental_v<type> || std::is_enum_v<type>) {
        current_size = sizeof(type);
    } else if constexpr (dm_is_string_like_v<type>) {
        current_size = sizeof(size_type) + item.size();
    } else if constexpr (dm_is_optional_v<type>) {
        current_size = sizeof(bool); // for has_value flag
        if (item.has_value()) {
            current_size += calculate_needed_size(*item);
        }
    } else if constexpr (dm_is_variant_v<type>) {
        current_size = sizeof(uint8_t); // for active index
        current_size += std::visit([](const auto& value) {
            return calculate_needed_size(value);
        }, item);
    } else if constexpr (dm_is_container_v<type> && !dm_is_string_like_v<type>) {
        current_size = sizeof(size_type); // for container size
        if constexpr (std::is_trivially_copyable_v<typename type::value_type>) {
            current_size += item.size() * sizeof(typename type::value_type);
        } else {
            for (const auto& i : item) {
                current_size += calculate_needed_size(i);
            }
        }
    } else if constexpr (get_type_id<type>() == DmTypeId::Aggregate) {
        dm_visit_members(item, [&](const auto&... members) {
            current_size += calculate_needed_size(members...);
        });
    } else {
        static_assert(!sizeof(type), "This type is not supported for serialization.");
    }
    
    return current_size + calculate_needed_size(items...);
}

// 通过索引访问 variant 的辅助函数
template<typename Variant, typename F>
bool visit_variant_at(size_t index, Variant& var, F&& func) {
    constexpr size_t n = std::variant_size_v<Variant>;
    bool result = false;
    // 使用折叠表达式和 lambda 在给定的索引处执行正确的 emplace 和函数调用
    ([&]<size_t... I>(std::index_sequence<I...>) {
        ( (I == index ? (var.template emplace<I>(), result = func(std::get<I>(var)), true) : false) || ... );
    }(std::make_index_sequence<n>{}));
    return result;
}


} // namespace pack_detail

class DmPacker {
public:
    DmPacker(char* buffer, size_t capacity) : buffer_(buffer), capacity_(capacity), pos_(0) {}

    template <typename... Args>
    bool serialize(const Args&... args) {
        constexpr uint32_t type_code = pack_detail::get_types_code<Args...>();
        
        size_t needed_size = sizeof(type_code) + pack_detail::calculate_needed_size(args...);
        if (needed_size > capacity_) {
            return false;
        }

        if (!write(&type_code, sizeof(type_code))) return false;
        return (serialize_one(args) && ...);
    }

    size_t get_size() const { return pos_; }

private:
    bool write(const void* data, size_t size) {
        if (pos_ + size > capacity_) return false;
        std::memcpy(buffer_ + pos_, data, size);
        pos_ += size;
        return true;
    }

    template <typename T>
    bool serialize_one(const T& item) {
        using type = dm_remove_cvref_t<T>;
        if constexpr (std::is_fundamental_v<type> || std::is_enum_v<type>) {
            return write(&item, sizeof(item));
        } else if constexpr (dm_is_string_like_v<type>) {
            pack_detail::size_type size = item.size();
            if (!write(&size, sizeof(size))) return false;
            return write(item.data(), size);
        } else if constexpr (dm_is_optional_v<type>) {
            bool has_value = item.has_value();
            if (!write(&has_value, sizeof(has_value))) return false;
            if (has_value) {
                return serialize_one(*item);
            }
            return true;
        } else if constexpr (dm_is_variant_v<type>) {
            uint8_t index = item.index();
            if(!write(&index, sizeof(index))) return false;
            return std::visit([this](const auto& value){
                return this->serialize_one(value);
            }, item);
        } else if constexpr (dm_is_container_v<type> && !dm_is_string_like_v<type>) {
            pack_detail::size_type size = item.size();
            if (!write(&size, sizeof(size))) return false;
            if constexpr (std::is_trivially_copyable_v<typename type::value_type>) {
                return write(item.data(), size * sizeof(typename type::value_type));
            } else {
                for (const auto& i : item) {
                    if (!serialize_one(i)) return false;
                }
            }
        } else if constexpr (pack_detail::get_type_id<type>() == pack_detail::DmTypeId::Aggregate) {
            bool success = true;
            dm_visit_members(item, [this, &success](const auto&... members) {
                success = (... && this->serialize_one(members));
            });
            return success;
        }
        return true;
    }

    char* buffer_;
    size_t capacity_;
    size_t pos_;
};


class DmUnpacker {
public:
    DmUnpacker(const char* buffer, size_t size) : buffer_(buffer), size_(size), pos_(0) {}

    template <typename... Args>
    bool deserialize(Args&... args) {
        uint32_t stored_code;
        if (!read(&stored_code, sizeof(stored_code))) return false;
        
        constexpr uint32_t expected_code = pack_detail::get_types_code<Args...>();
        if (stored_code != expected_code) {
            return false; // 类型不匹配
        }

        return (deserialize_one(args) && ...);
    }

private:
    bool read(void* data, size_t size) {
        if (pos_ + size > size_) return false;
        std::memcpy(data, buffer_ + pos_, size);
        pos_ += size;
        return true;
    }

    template <typename T>
    bool deserialize_one(T& item) {
        using type = dm_remove_cvref_t<T>;
        if constexpr (std::is_fundamental_v<type> || std::is_enum_v<type>) {
            return read(&item, sizeof(item));
        } else if constexpr (dm_is_string_like_v<type>) {
            pack_detail::size_type size;
            if (!read(&size, sizeof(size))) return false;
            if (pos_ + size > size_) return false;
            item.resize(size);
            return read(item.data(), size);
        } else if constexpr (dm_is_optional_v<type>) {
            bool has_value;
            if (!read(&has_value, sizeof(has_value))) return false;
            if (has_value) {
                item.emplace();
                return deserialize_one(*item);
            } else {
                item.reset();
            }
            return true;
        } else if constexpr (dm_is_variant_v<type>) {
            uint8_t index;
            if (!read(&index, sizeof(index))) return false;
            if (index >= std::variant_size_v<type>) return false;
            return pack_detail::visit_variant_at(index, item, [this](auto& value) {
                return this->deserialize_one(value);
            });
        } else if constexpr (dm_is_container_v<type> && !dm_is_string_like_v<type>) {
            pack_detail::size_type size;
            if (!read(&size, sizeof(size))) return false;
            item.resize(size);
            if constexpr (std::is_trivially_copyable_v<typename type::value_type>) {
                return read(item.data(), size * sizeof(typename type::value_type));
            } else {
                for (auto& i : item) {
                    if (!deserialize_one(i)) return false;
                }
            }
        } else if constexpr (pack_detail::get_type_id<type>() == pack_detail::DmTypeId::Aggregate) {
            bool result = true;
            dm_visit_members(item, [&](auto&... members) {
                result = ((this->deserialize_one(members)) && ...);
            });
            return result;
        }
        return true;
    }

    const char* buffer_;
    size_t size_;
    size_t pos_;
};

} // namespace dm

#endif // __DMTYPETRAITS_PACK_IMPL_H_INCLUDE__
