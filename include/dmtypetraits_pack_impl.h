
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

#include "dmtypetraits.h" // Includes all dm type traits

#if __cplusplus < 202002L
#error "dmtypetraits_pack_impl.h requires C++20 or later."
#endif

// This implementation assumes a little-endian system, matching the reference.
static_assert(std::endian::native == std::endian::little,
              "dmtypetraits_pack_impl only supports little-endian systems currently.");

namespace dm {
namespace pack_detail {

using size_type = uint32_t;
constexpr uint32_t MAX_SIZE = UINT32_MAX;

// Forward declarations
template <typename T, typename... Args>
constexpr std::size_t calculate_needed_size(const T& item, const Args&... items);

constexpr std::size_t calculate_needed_size() { return 0; }

// Calculates the serialized size of a single item.
template <typename T>
constexpr std::size_t calculate_one_size(const T& item) {
    using type = dm_remove_cvref_t<decltype(item)>;
    static_assert(!std::is_pointer_v<type>, "Raw pointers cannot be serialized.");
    std::size_t total = 0;

    if constexpr (std::is_fundamental_v<type> || std::is_enum_v<type>) {
        total += sizeof(type);
    } else if constexpr (dm_is_string_like_v<type>) { // For std::string, std::string_view
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

// Generates a unique 32-bit code for a type layout using MD5 hash.
template <typename... Args>
consteval uint32_t get_types_code() {
    // This part requires a constexpr string generation from types,
    // which is complex. We'll use a simplified version for this example,
    // hashing the concatenated type names. For a robust implementation,
    // a detailed type-id system like in struct-pack is needed.
    // Here we use a placeholder hash for demonstration.
    // To fully implement this, one would build a string from `get_type_id`
    // results and then MD5 hash it.
    constexpr auto type_name_str = (std::string_view(typeid(Args).name()) + ...);
    // In a real scenario, you would use your dm_md5_hash32 here.
    // return dm_md5_hash32(type_name_str.data(), type_name_str.length());
    return 0xDEADBEEF; // Placeholder
}

} // namespace pack_detail

// ===================================================================================
// DmPacker - Serializes objects into a byte buffer
// ===================================================================================
class DmPacker {
public:
    DmPacker(char* buffer, size_t size) : buffer_(buffer), capacity_(size) {}

    template <typename... Args>
    bool serialize(const Args&... args) {
        constexpr uint32_t type_code = pack_detail::get_types_code<Args...>();
        
        // Check capacity
        size_t needed_size = sizeof(type_code) + pack_detail::calculate_needed_size(args...);
        if (pos_ + needed_size > capacity_) {
            return false;
        }

        // Write type code
        write(&type_code, sizeof(type_code));

        // Write object data
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

// ===================================================================================
// DmUnpacker - Deserializes objects from a byte buffer
// ===================================================================================

class DmUnpacker {
public:
    DmUnpacker(const char* buffer, size_t size) : buffer_(buffer), size_(size) {}

    template <typename... Args>
    bool deserialize(Args&... args) {
        // Read and verify type code
        uint32_t stored_code;
        if (!read(&stored_code, sizeof(stored_code))) return false;

        constexpr uint32_t expected_code = pack_detail::get_types_code<Args...>();
        if (stored_code != expected_code) {
            return false; // Type mismatch
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