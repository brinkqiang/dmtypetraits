#ifndef __DMTYPETRAITS_PACK_H_INCLUDE__
#define __DMTYPETRAITS_PACK_H_INCLUDE__

#include "dmtypetraits_pack_impl.h"

namespace dm::pack {

template <size_t N>
struct members_count_t : public std::integral_constant<size_t, N> {};

DMPACK_INLINE std::string error_message(std::errc err) {
  return std::make_error_code(err).message();
}

template <typename T>
struct deserialize_result {
  std::errc errc;
  T value;
};

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

template <typename... Args>
DMPACK_INLINE constexpr std::size_t get_type_code() {
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
  if constexpr ((detail::unexist_compatible_member_v<Args> && ...))
    return detail::calculate_needed_size(args...) + sizeof(uint32_t);
  else
    return detail::calculate_needed_size(args...) + sizeof(uint32_t) +
           sizeof(uint64_t);
}

template <typename Byte, typename... Args,
          typename = std::enable_if_t<detail::dm_pack_byte_v<Byte>>>
std::size_t DMPACK_INLINE serialize_to(Byte *buffer, std::size_t len,
                                      const Args &...args) noexcept {
  static_assert(sizeof...(args) > 0);
  auto size = get_needed_size(args...);
  if (size > len) [[unlikely]] {
    return 0;
  }
  detail::packer<Byte> o(buffer);
  if constexpr ((detail::unexist_compatible_member_v<Args> && ...)) {
    o.serialize(args...);
  }
  else {
    o.serialize_with_size(size, args...);
  }
  return size;
}

template <typename Buffer, typename... Args,
          typename = std::enable_if_t<detail::dm_pack_buffer_v<Buffer>>>
void DMPACK_INLINE serialize_to(Buffer &buffer, const Args &...args) {
  static_assert(sizeof...(args) > 0);
  auto data_offset = buffer.size();
  auto need_size = get_needed_size(args...);
  auto total = data_offset + need_size;
  buffer.resize(total);
  detail::packer<typename Buffer::value_type> o(buffer.data() + data_offset);
  if constexpr ((detail::unexist_compatible_member_v<Args> && ...)) {
    o.serialize(args...);
  }
  else {
    o.serialize_with_size(need_size, args...);
  }
}

template <typename Buffer, typename... Args,
          typename = std::enable_if_t<detail::dm_pack_buffer_v<Buffer>>>
void DMPACK_INLINE serialize_to_with_offset(Buffer &buffer,
                                                 std::size_t offset,
                                                 const Args &...args) {
  static_assert(sizeof...(args) > 0);
  buffer.resize(buffer.size() + offset);
  serialize_to(buffer, args...);
}

template <typename Buffer = std::vector<char>,
          typename... Args,
          typename = std::enable_if_t<detail::dm_pack_buffer_v<Buffer>>>
[[nodiscard]] DMPACK_INLINE Buffer serialize(const Args &...args) {
  static_assert(sizeof...(args) > 0);
  Buffer buffer;
  serialize_to(buffer, args...);
  return buffer;
}

template <typename Buffer = std::vector<char>,
          typename... Args,
          typename = std::enable_if_t<detail::dm_pack_buffer_v<Buffer>>>
[[nodiscard]] DMPACK_INLINE Buffer
serialize_with_offset(std::size_t offset, const Args &...args) {
  static_assert(sizeof...(args) > 0);
  Buffer buffer;
  buffer.resize(offset);
  serialize_to(buffer, args...);
  return buffer;
}


template <typename T, typename View,
          typename = std::enable_if_t<detail::dm_pack_deserialize_view_v<View>>>
[[nodiscard]] DMPACK_INLINE std::errc deserialize_to(T &t, const View &v) {
  detail::unpacker<typename View::value_type> in(v.data(), v.size());
  return in.deserialize(t);
}

template <typename T, typename Byte,
          typename = std::enable_if_t<detail::dm_pack_byte_v<Byte>>>
[[nodiscard]] DMPACK_INLINE std::errc deserialize_to(T &t,
                                                    const Byte *data,
                                                    size_t size) {
  detail::unpacker<Byte> in(data, size);
  return in.deserialize(t);
}

template <typename T, typename View,
          typename = std::enable_if_t<detail::dm_pack_deserialize_view_v<View>>>
[[nodiscard]] DMPACK_INLINE std::errc deserialize_to(T &t, const View &v,
                                                    size_t &consume_len) {
  detail::unpacker<typename View::value_type> in(v.data(), v.size());
  return in.deserialize(t, consume_len);
}

template <typename T, typename Byte,
          typename = std::enable_if_t<detail::dm_pack_byte_v<Byte>>>
[[nodiscard]] DMPACK_INLINE std::errc deserialize_to(T &t,
                                                    const Byte *data,
                                                    size_t size,
                                                    size_t &consume_len) {
  detail::unpacker<Byte> in(data, size);
  return in.deserialize(t, consume_len);
}

template <typename T, typename View,
          typename = std::enable_if_t<detail::dm_pack_deserialize_view_v<View>>>
[[nodiscard]] DMPACK_INLINE std::errc deserialize_to_with_offset(
    T &t, const View &v, size_t &offset) {
  detail::unpacker<typename View::value_type> in(v.data() + offset, v.size() - offset);
  size_t sz;
  auto ret = in.deserialize(t, sz);
  offset += sz;
  return ret;
}

template <typename T, typename Byte,
          typename = std::enable_if_t<detail::dm_pack_byte_v<Byte>>>
[[nodiscard]] DMPACK_INLINE std::errc deserialize_to_with_offset(
    T &t, const Byte *data, size_t size, size_t &offset) {
  detail::unpacker<Byte> in(data + offset, size - offset);
  size_t sz;
  auto ret = in.deserialize(t, sz);
  offset += sz;
  return ret;
}


template <typename T, typename View,
          typename = std::enable_if_t<detail::dm_pack_deserialize_view_v<View>>>
[[nodiscard]] DMPACK_INLINE deserialize_result<T> deserialize(
    const View &v) {
  deserialize_result<T> ret;
  ret.errc = deserialize_to(ret.value, v);
  return ret;
}

template <typename T, typename Byte,
          typename = std::enable_if_t<detail::dm_pack_byte_v<Byte>>>
[[nodiscard]] DMPACK_INLINE deserialize_result<T> deserialize(
    const Byte *data, size_t size) {
  deserialize_result<T> ret;
  ret.errc = deserialize_to(ret.value, data, size);
  return ret;
}

template <typename T, typename View,
          typename = std::enable_if_t<detail::dm_pack_deserialize_view_v<View>>>
[[nodiscard]] DMPACK_INLINE deserialize_result<T> deserialize(
    const View &v, size_t &consume_len) {
  deserialize_result<T> ret;
  ret.errc = deserialize_to(ret.value, v, consume_len);
  return ret;
}

template <typename T, typename Byte,
          typename = std::enable_if_t<detail::dm_pack_byte_v<Byte>>>
[[nodiscard]] DMPACK_INLINE deserialize_result<T> deserialize(
    const Byte *data, size_t size, size_t &consume_len) {
  deserialize_result<T> ret;
  ret.errc = deserialize_to(ret.value, data, size, consume_len);
  return ret;
}

template <typename T, typename View,
          typename = std::enable_if_t<detail::dm_pack_deserialize_view_v<View>>>
[[nodiscard]] DMPACK_INLINE deserialize_result<T> deserialize_with_offset(
    const View &v, size_t &offset) {
  deserialize_result<T> ret;
  ret.errc = deserialize_to_with_offset(ret.value, v, offset);
  return ret;
}

template <typename T, typename Byte,
          typename = std::enable_if_t<detail::dm_pack_byte_v<Byte>>>
[[nodiscard]] DMPACK_INLINE deserialize_result<T> deserialize_with_offset(
    const Byte *data, size_t size, size_t &offset) {
  deserialize_result<T> ret;
  ret.errc = deserialize_to_with_offset(ret.value, data, size, offset);
  return ret;
}

template <typename T, size_t I, typename View,
          typename = std::enable_if_t<detail::dm_pack_deserialize_view_v<View>>>
[[nodiscard]] DMPACK_INLINE decltype(auto) get_field(const View &v) {
  detail::unpacker<typename View::value_type> in(v.data(), v.size());
  return in.template get_field<T, I>();
}

template <typename T, size_t I, typename Byte,
          typename = std::enable_if_t<detail::dm_pack_byte_v<Byte>>>
[[nodiscard]] DMPACK_INLINE decltype(auto) get_field(const Byte *data,
                                                    size_t size) {
  detail::unpacker<Byte> in(data, size);
  return in.template get_field<T, I>();
}


}
#endif