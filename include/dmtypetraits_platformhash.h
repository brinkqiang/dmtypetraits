#ifndef ___DMTYPETRAITS_PLATFORMHASH_H_INCLUDE__
#define ___DMTYPETRAITS_PLATFORMHASH_H_INCLUDE__

#include "dmtypetraits_base.h"
#include "dmtypetraits_md5.h"
#include <cstdint>

namespace dm {
    namespace platform {

        // 编译时计算平台名称哈希
        namespace detail {
            constexpr auto get_os_name() {
                // 假设 string_literal 在 dmtypetraits_md5.h 或 dmtypetraits_base.h 中已定义
                using dm::pack::detail::string_literal;
#if defined(_WIN32)
                return string_literal("Windows");
#elif defined(__APPLE__)
                return string_literal("macOS");
#elif defined(__linux__)
                return string_literal("Linux");
#else
                return string_literal("Unknown");
#endif
            }

            constexpr uint32_t current_os_hash = dm::pack::detail::MD5::MD5Hash32Constexpr(get_os_name().data());
            constexpr uint32_t windows_hash = dm::pack::detail::MD5::MD5Hash32Constexpr("Windows");
            constexpr uint32_t macos_hash = dm::pack::detail::MD5::MD5Hash32Constexpr("macOS");
            constexpr uint32_t linux_hash = dm::pack::detail::MD5::MD5Hash32Constexpr("Linux");
            constexpr uint32_t unknown_hash = dm::pack::detail::MD5::MD5Hash32Constexpr("Unknown");
        }

        // 基于哈希值的类型标签
        template<uint32_t Hash>
        struct PlatformHashTag {
            static constexpr uint32_t hash_value = Hash;
        };

        using WindowsHashType = PlatformHashTag<detail::windows_hash>;
        using MacOSHashType = PlatformHashTag<detail::macos_hash>;
        using LinuxHashType = PlatformHashTag<detail::linux_hash>;
        using UnknownHashType = PlatformHashTag<detail::unknown_hash>;

        template<uint32_t CurrentHash>
        struct CurrentPlatformType {
            using type = dm_conditional_t<
                CurrentHash == detail::windows_hash,
                WindowsHashType,
                dm_conditional_t<
                CurrentHash == detail::macos_hash,
                MacOSHashType,
                dm_conditional_t<
                CurrentHash == detail::linux_hash,
                LinuxHashType,
                UnknownHashType
                >
                >
            >;
        };

        using CurrentPlatform = typename CurrentPlatformType<detail::current_os_hash>::type;

        template<typename PlatformHashType>
        struct PlatformHashTraits;

        template<>
        struct PlatformHashTraits<WindowsHashType> {
            static constexpr const char* name() { return "Windows"; }
            static constexpr bool is_unix_like() { return false; }
            static constexpr char file_separator() { return '\\'; }
        };

        template<>
        struct PlatformHashTraits<MacOSHashType> {
            static constexpr const char* name() { return "macOS"; }
            static constexpr bool is_unix_like() { return true; }
            static constexpr char file_separator() { return '/'; }
        };

        template<>
        struct PlatformHashTraits<LinuxHashType> {
            static constexpr const char* name() { return "Linux"; }
            static constexpr bool is_unix_like() { return true; }
            static constexpr char file_separator() { return '/'; }
        };

        template<>
        struct PlatformHashTraits<UnknownHashType> {
            static constexpr const char* name() { return "Unknown"; }
            static constexpr bool is_unix_like() { return false; }
            static constexpr char file_separator() { return '/'; }
        };
        inline void PrintCurrentPlatformInfo() {
            using Traits = dm::platform::PlatformHashTraits<CurrentPlatform>;

            std::cout << "=== 平台信息 ===" << std::endl;
            std::cout << "平台名称: " << Traits::name() << std::endl;
            std::cout << "是否Unix-like: " << (Traits::is_unix_like() ? "是" : "否") << std::endl;
            std::cout << "文件分隔符: " << Traits::file_separator() << std::endl;
            std::cout << "平台哈希值: 0x" << std::hex << CurrentPlatform::hash_value << std::endl;
        }
    } // namespace platform
} // namespace dm

#endif // ___DMTYPETRAITS_PLATFORMHASH_H_INCLUDE__