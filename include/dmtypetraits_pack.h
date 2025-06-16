
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

#ifndef __DMTYPETRAITS_PACK_H_INCLUDE__
#define __DMTYPETRAITS_PACK_H_INCLUDE__

#include "dmtypetraits_pack_impl.h"
#include <vector>
#include <string_view>
#include <system_error>

/**
 * @brief Main namespace for the DM serialization library.
 */
namespace dm::pack {

/**
 * @brief The result of a deserialization operation.
 *
 * Contains an error code and the potentially deserialized value.
 * @tparam T The type of the object to be deserialized.
 */
template <typename T>
struct DmDeserializeResult {
    std::errc error_code;
    T value;
};

/**
 * @brief Calculates the total byte size required to serialize one or more objects.
 *
 * @param args The objects to be serialized.
 * @return The required size in bytes, including the type-code header.
 */
template <typename... Args>
[[nodiscard]] constexpr size_t get_needed_size(const Args&... args) {
    // Adds size of type code header to the calculated size of the objects.
    return sizeof(uint32_t) + dm::pack_detail::calculate_needed_size(args...);
}

/**
 * @brief Serializes one or more objects into a pre-allocated buffer.
 *
 * @param buffer A pointer to the start of the buffer.
 * @param len The total size of the buffer.
 * @param args The objects to serialize.
 * @return The number of bytes written, or 0 on failure (e.g., buffer too small).
 */
template <typename... Args>
inline size_t serialize_to(char* buffer, size_t len, const Args&... args) noexcept {
    static_assert(sizeof...(args) > 0, "At least one object must be provided to serialize.");
    size_t needed_size = get_needed_size(args...);
    if (needed_size > len) {
        return 0; // Buffer is too small.
    }
    DmPacker packer(buffer, len);
    if (packer.serialize(args...)) {
        return packer.get_size();
    }
    return 0;
}

/**
 * @brief Serializes one or more objects into a resizable container (e.g., std::vector<char>).
 *
 * The container will be resized to fit the serialized data.
 * @param buffer The buffer to serialize into.
 * @param args The objects to serialize.
 */
template <typename Buffer, typename... Args>
void serialize_to(Buffer& buffer, const Args&... args) {
    static_assert(sizeof...(args) > 0, "At least one object must be provided to serialize.");
    size_t needed_size = get_needed_size(args...);
    size_t original_size = buffer.size();
    buffer.resize(original_size + needed_size);
    DmPacker packer(buffer.data() + original_size, needed_size);
    packer.serialize(args...);
}

/**
 * @brief Serializes one or more objects and returns them in a new buffer.
 *
 * @tparam Buffer The desired output buffer type (defaults to std::vector<char>).
 * @param args The objects to serialize.
 * @return A new buffer containing the serialized data.
 */
template <typename Buffer = std::vector<char>, typename... Args>
[[nodiscard]] inline Buffer serialize(const Args&... args) {
    static_assert(sizeof...(args) > 0, "At least one object must be provided to serialize.");
    Buffer buffer;
    serialize_to(buffer, args...);
    return buffer;
}

/**
 * @brief Deserializes an object from a view-like container (e.g., std::string_view).
 *
 * @tparam T The type of the object to deserialize.
 * @param t The object to populate with deserialized data.
 * @param view The container holding the serialized data.
 * @return An error code indicating the result of the operation.
 */
template <typename T, typename View>
[[nodiscard]] inline std::errc deserialize_to(T& t, const View& view) {
    DmUnpacker unpacker(view.data(), view.size());
    if (unpacker.deserialize(t)) {
        return {};
    }
    return std::errc::invalid_argument; // Or a more specific error
}

/**
 * @brief Deserializes an object from a raw byte buffer.
 *
 * @tparam T The type of the object to deserialize.
 * @param t The object to populate.
 * @param data Pointer to the buffer.
 * @param size The size of the buffer.
 * @return An error code indicating the result.
 */
template <typename T>
[[nodiscard]] inline std::errc deserialize_to(T& t, const char* data, size_t size) {
    DmUnpacker unpacker(data, size);
    if (unpacker.deserialize(t)) {
        return {};
    }
    return std::errc::invalid_argument;
}

/**
 * @brief Deserializes an object from a view-like container.
 *
 * @tparam T The type of the object to deserialize.
 * @param view The view containing the data.
 * @return A DmDeserializeResult containing the error code and the deserialized object.
 */
template <typename T, typename View>
[[nodiscard]] inline DmDeserializeResult<T> deserialize(const View& view) {
    T obj{};
    std::errc ec = deserialize_to(obj, view);
    return {ec, std::move(obj)};
}

/**
 * @brief Deserializes an object from a raw byte buffer.
 *
 * @tparam T The type of the object to deserialize.
 * @param data Pointer to the buffer.
 * @param size The size of the buffer.
 * @return A DmDeserializeResult containing the error code and the deserialized object.
 */
template <typename T>
[[nodiscard]] inline DmDeserializeResult<T> deserialize(const char* data, size_t size) {
    T obj{};
    std::errc ec = deserialize_to(obj, data, size);
    return {ec, std::move(obj)};
}

} // namespace dm::pack

#endif // __DMTYPETRAITS_PACK_H_INCLUDE__