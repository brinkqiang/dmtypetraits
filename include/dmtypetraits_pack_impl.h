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

#include "dmtypetraits.h" // ��ͷ�ļ�

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
// START: ��ǿ�ı�����������Ϣϵͳ
// ===================================================================================

// ��չ�������IDö�٣����ӽ�ԭ��ʵ��
enum class DmTypeId : char {
    // ��������
    Int8 = 'a', UInt8 = 'b', Int16 = 'c', UInt16 = 'd',
    Int32 = 'e', UInt32 = 'f', Int64 = 'g', UInt64 = 'h',
    Bool = 'i', Char = 'j', Float = 'k', Double = 'l',
    Char16 = 'm', Char32 = 'n',

    // ģ��/��������
    Monostate = 'o',
    String = 's', 
    Array = 't', 
    Map = 'u', 
    Set = 'v', 
    Container = 'w', // vector, list �ȷ�������
    Optional = 'x', 
    Variant = 'y', 
    
    // �ṹ����
    Aggregate = '0', 
    EndAggregate = 'Z',
};

// --- �����ڸ������� ---

// ��һ��size_t��������Ϊһ��8�ֽڵı������ַ��� (���� struct_pack �� get_size_literal)
template <size_t Size>
consteval auto encode_size_as_literal() {
    static_assert(sizeof(size_t) <= 8);
    return dm::md5_detail::DmStringLiteral<char, 8>{
        {char(Size >> 56), char((Size >> 48) & 0xFF), char((Size >> 40) & 0xFF), char((Size >> 32) & 0xFF),
         char((Size >> 24) & 0xFF), char((Size >> 16) & 0xFF), char((Size >> 8) & 0xFF), char(Size & 0xFF)}};
}


// --- ��ǿ������ʶ����ǩ������ ---

// ǰ������
template <typename T>
consteval auto get_type_literal();

// ��ǿ�� get_type_id��ʶ������������
template <typename T>
consteval DmTypeId get_type_id() {
    using type = dm_remove_cvref_t<T>;
    // ���˳�����Ҫ

    // ��������
    if constexpr (std::is_same_v<type, int8_t> || std::is_same_v<type, signed char>) return DmTypeId::Int8;
    if constexpr (std::is_same_v<type, uint8_t> || std::is_same_v<type, unsigned char>) return DmTypeId::UInt8;
    if constexpr (std::is_same_v<type, int16_t>) return DmTypeId::Int16;
    if constexpr (std::is_same_v<type, uint16_t>) return DmTypeId::UInt16;
    if constexpr (std::is_same_v<type, int32_t>) return DmTypeId::Int32;
    if constexpr (std::is_same_v<type, uint32_t>) return DmTypeId::UInt32;
    if constexpr (std::is_same_v<type, int64_t> || (sizeof(long long) == 8 && std::is_same_v<type, long long>)) return DmTypeId::Int64;
    if constexpr (std::is_same_v<type, uint64_t> || (sizeof(unsigned long long) == 8 && std::is_same_v<type, unsigned long long>)) return DmTypeId::UInt64;
    if constexpr (std::is_same_v<type, bool>) return DmTypeId::Bool;
    if constexpr (std::is_same_v<type, char> || std::is_same_v<type, char8_t>) return DmTypeId::Char;
    if constexpr (std::is_same_v<type, char16_t>) return DmTypeId::Char16;
    if constexpr (std::is_same_v<type, char32_t>) return DmTypeId::Char32;
    if constexpr (std::is_same_v<type, float>) return DmTypeId::Float;
    if constexpr (std::is_same_v<type, double>) return DmTypeId::Double;

    // ��״̬
    if constexpr (dm_is_monostate_v<type>) return DmTypeId::Monostate;

    // ��������
    if constexpr (dm_is_optional_v<type>) return DmTypeId::Optional;
    if constexpr (dm_is_variant_v<type>) return DmTypeId::Variant;
    if constexpr (dm_is_string_like_v<type>) return DmTypeId::String;
    if constexpr (dm_is_array_v<type> || dm_is_c_array_v<type>) return DmTypeId::Array;
    if constexpr (dm_is_map_v<type>) return DmTypeId::Map;
    if constexpr (dm_is_set_v<type>) return DmTypeId::Set;
    if constexpr (dm_is_container_v<type>) return DmTypeId::Container; // vector, list��
    
    // �ۺ�����
    if constexpr (std::is_aggregate_v<type> || dm_is_pair_v<type> || dm_is_tuple_v<type>) return DmTypeId::Aggregate;

    static_assert(!sizeof(type), "Unsupported type for serialization.");
    return DmTypeId{};
}

// �������������ڻ�ȡ�ۺ������г�Ա������������
template <typename... Members>
consteval auto get_members_literal() {
    if constexpr (sizeof...(Members) == 0) {
        return dm::md5_detail::DmStringLiteral("");
    } else {
        return (get_type_literal<Members>() + ...);
    }
}

// ��ǿ�� get_type_literal, ������ḻ������
template <typename T>
consteval auto get_type_literal() {
    using type = dm_remove_cvref_t<T>;
    constexpr DmTypeId id = get_type_id<type>();
    constexpr char id_char_array[] = {static_cast<char>(id), '\0'};
    constexpr auto ret = dm::md5_detail::DmStringLiteral(id_char_array);
    constexpr char end_char_array[] = {static_cast<char>(DmTypeId::EndAggregate), '\0'};
    constexpr auto end_literal = dm::md5_detail::DmStringLiteral(end_char_array);

    if constexpr (id == DmTypeId::Aggregate) {
        return dm_visit_members(type{}, []<typename... Members>() {
            return ret + get_members_literal<Members...>() + end_literal;
        });
    } else if constexpr (id == DmTypeId::Variant) {
        return dm_visit_variant(type{}, []<typename... Alternatives>() {
             return ret + (get_type_literal<Alternatives>() + ...) + end_literal;
        });
    } else if constexpr (id == DmTypeId::Array) {
        // ǩ���а���Ԫ�����ͺ������С
        constexpr auto size_literal = encode_size_as_literal<dm_get_array_size_v<type>>();
        return ret + get_type_literal<dm_get_element_type_t<type>>() + size_literal;
    } else if constexpr (id == DmTypeId::Map) {
        // ǩ���а���key���ͺ�value����
        return ret + get_type_literal<typename type::key_type>() + get_type_literal<typename type::mapped_type>();
    } else if constexpr (id == DmTypeId::Optional || id == DmTypeId::String || id == DmTypeId::Set || id == DmTypeId::Container) {
        // ��Щ��ֻ����һ��������
        return ret + get_type_literal<typename type::value_type>();
    }
    else {
        // �������ͺ� Monostate ֱ�ӷ����Լ���ID
        return ret;
    }
}

// ��������Ϊһ�������������յ�32λ���͹�ϣ��
template <typename... Args>
consteval uint32_t get_types_code() {
    constexpr auto final_literal = (get_type_literal<Args>() + ...);
    return dm::dm_md5_hash32(final_literal.data(), final_literal.size());
}

// ===================================================================================
// END: ��ǿ�ı�����������Ϣϵͳ
// ===================================================================================


// calculate_needed_size �� DmPacker/DmUnpacker ��ʵ�ֻ������ֲ��䣬
// ��Ϊ���������ĵײ� `get_types_code` ��������ȡ�Ѿ���ø���ǿ��;�ȷ��
// ����Ϊ�������ԣ��ٴ��г���

template <typename T, typename... Args>
constexpr std::size_t calculate_needed_size(const T& item, const Args&... items);

constexpr std::size_t calculate_needed_size() { return 0; }

template <typename T>
constexpr std::size_t calculate_one_size(const T& item) {
    using type = dm_remove_cvref_t<T>;
    static_assert(!std::is_pointer_v<type>, "Raw pointers cannot be serialized.");
    std::size_t total = 0;

    if constexpr (std::is_fundamental_v<type> || std::is_enum_v<type>) {
        total += sizeof(type);
    } else if constexpr (dm_is_string_like_v<type>) {
        total += sizeof(size_type) + item.size();
    } else if constexpr (dm_is_container_v<type> && !dm_is_string_like_v<type>) {
        total += sizeof(size_type);
        if constexpr (std::is_trivially_copyable_v<typename type::value_type>) {
            total += item.size() * sizeof(typename type::value_type);
        } else {
            for (const auto& i : item) {
                total += calculate_one_size(i);
            }
        }
    } else if constexpr (std::is_aggregate_v<type>) {
        dm_visit_members(item, [&](const auto&... members) {
            total += calculate_needed_size(members...);
        });
    } else {
        static_assert(!sizeof(type), "This type is not supported for serialization.");
    }
    return total;
}

template <typename T, typename... Args>
constexpr std::size_t calculate_needed_size(const T& item, const Args&... items) {
    return calculate_one_size(item) + calculate_needed_size(items...);
}

} // namespace pack_detail

class DmPacker {
public:
    DmPacker(char* buffer, size_t capacity) : buffer_(buffer), capacity_(capacity) {}

    template <typename... Args>
    bool serialize(const Args&... args) {
        constexpr uint32_t type_code = pack_detail::get_types_code<Args...>();
        
        size_t needed_size = sizeof(type_code) + pack_detail::calculate_needed_size(args...);
        if (pos_ + needed_size > capacity_) {
            return false;
        }

        write(&type_code, sizeof(type_code));
        (serialize_one(args), ...);
        return true;
    }

    size_t get_size() const { return pos_; }

private:
    void write(const void* data, size_t size) {
        std::memcpy(buffer_ + pos_, data, size);
        pos_ += size;
    }

    template <typename T>
    void serialize_one(const T& item) {
        using type = dm_remove_cvref_t<T>;
        if constexpr (std::is_fundamental_v<type> || std::is_enum_v<type>) {
            write(&item, sizeof(item));
        } else if constexpr (dm_is_string_like_v<type>) {
            pack_detail::size_type size = item.size();
            write(&size, sizeof(size));
            write(item.data(), size);
        } else if constexpr (dm_is_container_v<type>) {
            pack_detail::size_type size = item.size();
            write(&size, sizeof(size));
            if constexpr (std::is_trivially_copyable_v<typename type::value_type>) {
                write(item.data(), size * sizeof(typename type::value_type));
            } else {
                for (const auto& i : item) {
                    serialize_one(i);
                }
            }
        } else if constexpr (std::is_aggregate_v<type>) {
            dm_visit_members(item, [this](const auto&... members) {
                (this->serialize_one(members), ...);
            });
        }
    }

    char* buffer_;
    size_t capacity_;
    size_t pos_ = 0;
};


class DmUnpacker {
public:
    DmUnpacker(const char* buffer, size_t size) : buffer_(buffer), size_(size) {}

    template <typename... Args>
    bool deserialize(Args&... args) {
        uint32_t stored_code;
        if (!read(&stored_code, sizeof(stored_code))) return false;
        
        constexpr uint32_t expected_code = pack_detail::get_types_code<Args...>();
        if (stored_code != expected_code) {
            return false; // ���Ͳ�ƥ��
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
            return read(&item[0], size);
        } else if constexpr (dm_is_container_v<type>) {
            pack_detail::size_type size;
            if (!read(&size, sizeof(size))) return false;
            item.resize(size);
            if constexpr (std::is_trivially_copyable_v<typename type::value_type>) {
                return read(item.data(), size * sizeof(typename type::value_type));
            } else {
                for (auto& i : item) {
                    if (!deserialize_one(i)) return false;
                }
                return true;
            }
        } else if constexpr (std::is_aggregate_v<type>) {
            bool result = true;
            dm_visit_members(item, [&](auto&... members) {
                result = ((this->deserialize_one(members)) && ...);
            });
            return result;
        }
        return false;
    }

    const char* buffer_;
    size_t size_;
    size_t pos_ = 0;
};

} // namespace dm

#endif // __DMTYPETRAITS_PACK_IMPL_H_INCLUDE__