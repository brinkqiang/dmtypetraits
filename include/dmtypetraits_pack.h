#ifndef __DMTYPETRAITS_PACK_H_INCLUDE__
#define __DMTYPETRAITS_PACK_H_INCLUDE__

#include "dmtypetraits_pack_impl.h"

namespace dm::pack {

/*! \defgroup dm_pack dm_pack
 * \brief A serialization Library based on compile-time reflection.
 *
 * (Documentation from original struct_pack is preserved and adapted)
 *
 * \section dm_pack_compiler_support Compiler Support
 *
 * | Compiler      | Version |
 * | ----------- | ------------------ |
 * | GCC         | 10.2               |
 * | Clang       | 13.0.1             |
 * | Apple Clang | 13.1.16            |
 * | MSVC        | 14.32              |
 *
 */

template <size_t N>
struct members_count_t : public std::integral_constant<size_t, N> {};

/*!
 * \ingroup dm_pack
 * Get the error message corresponding to the error code `err`.
 */
DMPACK_INLINE std::string error_message(std::errc err) {
  return std::make_error_code(err).message();
}

/*!
 * \ingroup dm_pack
 * \brief return type of dm::pack deserialization
 */
template <typename T>
struct deserialize_result {
  std::errc errc; //!< error code
  T value;        //!< deserialization object
};

/*!
 * \ingroup dm_pack
 * \brief a forward compatible field, similar to std::optional.
 */
template <typename T>
struct compatible : public std::optional<T> {
  using base = std::optional<T>;
  using base::base;
  constexpr compatible() = default;
  constexpr compatible(const compatible &other) = default;
  constexpr compatible(compatible &&other) = default;
  constexpr compatible(std::optional<T> &&other) : std::optional<T>(std::move(other)){};
  constexpr compatible(const std::optional<T> &other) : std::optional<T>(other){};
  constexpr compatible &operator=(const compatible &other) = default;
  constexpr compatible &operator=(compatible &&other) = default;
};

// --- Public API functions ---

template <typename... Args>
DMPACK_INLINE consteval std::size_t get_type_code() {
  static_assert(sizeof...(Args) > 0);
  if constexpr (sizeof...(Args) == 1) {
    return detail::get_types_code<decltype(detail::get_types(
        std::declval<Args...>()))>();
  }
  else {
    return detail::get_types_code<std::tuple<Args...>>();
  }
}

template <typename... Args>
[[nodiscard]] DMPACK_INLINE constexpr size_t get_needed_size(
    const Args &...args) {
  if constexpr ((detail::unexist_compatible_member<Args> && ...))
    return detail::calculate_needed_size(args...) + sizeof(uint32_t);
  else
    return detail::calculate_needed_size(args...) + sizeof(uint32_t) +
           sizeof(uint64_t);
}

template <detail::dm_pack_byte Byte, typename... Args>
std::size_t DMPACK_INLINE serialize_to(Byte *buffer, std::size_t len,
                                      const Args &...args) noexcept {
  static_assert(sizeof...(args) > 0);
  auto size = get_needed_size(args...);
  if (size > len) [[unlikely]] {
    return 0;
  }
  detail::packer o(buffer);
  if constexpr ((detail::unexist_compatible_member<Args> && ...)) {
    o.serialize(args...);
  }
  else {
    o.serialize_with_size(size, args...);
  }
  return size;
}

template <detail::dm_pack_buffer Buffer, typename... Args>
void DMPACK_INLINE serialize_to(Buffer &buffer, const Args &...args) {
  static_assert(sizeof...(args) > 0);
  auto data_offset = buffer.size();
  auto need_size = get_needed_size(args...);
  auto total = data_offset + need_size;
  buffer.resize(total);
  detail::packer o(buffer.data() + data_offset);
  if constexpr ((detail::unexist_compatible_member<Args> && ...)) {
    o.serialize(args...);
  }
  else {
    o.serialize_with_size(need_size, args...);
  }
}

template <detail::dm_pack_buffer Buffer, typename... Args>
void DMPACK_INLINE serialize_to_with_offset(Buffer &buffer,
                                                 std::size_t offset,
                                                 const Args &...args) {
  static_assert(sizeof...(args) > 0);
  buffer.resize(buffer.size() + offset);
  serialize_to(buffer, args...);
}

template <detail::dm_pack_buffer Buffer = std::vector<char>,
          typename... Args>
[[nodiscard]] DMPACK_INLINE Buffer serialize(const Args &...args) {
  static_assert(sizeof...(args) > 0);
  Buffer buffer;
  serialize_to(buffer, args...);
  return buffer;
}

template <detail::dm_pack_buffer Buffer = std::vector<char>,
          typename... Args>
[[nodiscard]] DMPACK_INLINE Buffer
serialize_with_offset(std::size_t offset, const Args &...args) {
  static_assert(sizeof...(args) > 0);
  Buffer buffer;
  buffer.resize(offset);
  serialize_to(buffer, args...);
  return buffer;
}


template <typename T, detail::dm_pack_deserialize_view View>
[[nodiscard]] DMPACK_INLINE std::errc deserialize_to(T &t, const View &v) {
  detail::unpacker in(v.data(), v.size());
  return in.deserialize(t);
}

template <typename T, detail::dm_pack_byte Byte>
[[nodiscard]] DMPACK_INLINE std::errc deserialize_to(T &t,
                                                    const Byte *data,
                                                    size_t size) {
  detail::unpacker in(data, size);
  return in.deserialize(t);
}

template <typename T, detail::dm_pack_deserialize_view View>
[[nodiscard]] DMPACK_INLINE std::errc deserialize_to(T &t, const View &v,
                                                    size_t &consume_len) {
  detail::unpacker in(v.data(), v.size());
  return in.deserialize(t, consume_len);
}

template <typename T, detail::dm_pack_byte Byte>
[[nodiscard]] DMPACK_INLINE std::errc deserialize_to(T &t,
                                                    const Byte *data,
                                                    size_t size,
                                                    size_t &consume_len) {
  detail::unpacker in(data, size);
  return in.deserialize(t, consume_len);
}

template <typename T, detail::dm_pack_deserialize_view View>
[[nodiscard]] DMPACK_INLINE std::errc deserialize_to_with_offset(
    T &t, const View &v, size_t &offset) {
  detail::unpacker in(v.data() + offset, v.size() - offset);
  size_t sz;
  auto ret = in.deserialize(t, sz);
  offset += sz;
  return ret;
}

template <typename T, detail::dm_pack_byte Byte>
[[nodiscard]] DMPACK_INLINE std::errc deserialize_to_with_offset(
    T &t, const Byte *data, size_t size, size_t &offset) {
  detail::unpacker in(data + offset, size - offset);
  size_t sz;
  auto ret = in.deserialize(t, sz);
  offset += sz;
  return ret;
}


template <typename T, detail::dm_pack_deserialize_view View>
[[nodiscard]] DMPACK_INLINE deserialize_result<T> deserialize(
    const View &v) {
  deserialize_result<T> ret;
  ret.errc = deserialize_to(ret.value, v);
  return ret;
}

template <typename T, detail::dm_pack_byte Byte>
[[nodiscard]] DMPACK_INLINE deserialize_result<T> deserialize(
    const Byte *data, size_t size) {
  deserialize_result<T> ret;
  ret.errc = deserialize_to(ret.value, data, size);
  return ret;
}

template <typename T, detail::dm_pack_deserialize_view View>
[[nodiscard]] DMPACK_INLINE deserialize_result<T> deserialize(
    const View &v, size_t &consume_len) {
  deserialize_result<T> ret;
  ret.errc = deserialize_to(ret.value, v, consume_len);
  return ret;
}

template <typename T, detail::dm_pack_byte Byte>
[[nodiscard]] DMPACK_INLINE deserialize_result<T> deserialize(
    const Byte *data, size_t size, size_t &consume_len) {
  deserialize_result<T> ret;
  ret.errc = deserialize_to(ret.value, data, size, consume_len);
  return ret;
}

template <typename T, detail::dm_pack_deserialize_view View>
[[nodiscard]] DMPACK_INLINE deserialize_result<T> deserialize_with_offset(
    const View &v, size_t &offset) {
  deserialize_result<T> ret;
  ret.errc = deserialize_to_with_offset(ret.value, v, offset);
  return ret;
}

template <typename T, detail::dm_pack_byte Byte>
[[nodiscard]] DMPACK_INLINE deserialize_result<T> deserialize_with_offset(
    const Byte *data, size_t size, size_t &offset) {
  deserialize_result<T> ret;
  ret.errc = deserialize_to_with_offset(ret.value, data, size, offset);
  return ret;
}

template <typename T, size_t I, detail::dm_pack_deserialize_view View>
[[nodiscard]] DMPACK_INLINE decltype(auto) get_field(const View &v) {
  detail::unpacker in(v.data(), v.size());
  return in.template get_field<T, I>();
}

template <typename T, size_t I, detail::dm_pack_byte Byte>
[[nodiscard]] DMPACK_INLINE decltype(auto) get_field(const Byte *data,
                                                    size_t size) {
  detail::unpacker in(data, size);
  return in.template get_field<T, I>();
}


}  // namespace dm::pack

#endif // __DMTYPETRAITS_PACK_H_INCLUDE__