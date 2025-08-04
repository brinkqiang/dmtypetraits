#include <iostream>
#include "dmtypetraits_md5.h" // 您的头文件

// --- 步骤 1: 编译时获取OS名称 ---
constexpr auto get_os_name() {
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

// --- 步骤 2: 编译时计算哈希 ---
constexpr uint32_t os_name_hash =
dm::pack::detail::MD5::MD5Hash32Constexpr(get_os_name().data());

// --- 步骤 3: 编译时根据哈希选择字符串 ---
namespace detail {
    // 将已知的哈希值作为常量，便于比较
    constexpr uint32_t windows_hash = dm::pack::detail::MD5::MD5Hash32Constexpr("Windows");
    constexpr uint32_t macos_hash = dm::pack::detail::MD5::MD5Hash32Constexpr("macOS");
    constexpr uint32_t linux_hash = dm::pack::detail::MD5::MD5Hash32Constexpr("Linux");
}

constexpr auto get_system_details_by_md5() {
    using dm::pack::detail::string_literal;

    if constexpr (os_name_hash == detail::windows_hash) {
        return string_literal("System branch selected by MD5 hash: This is a Windows system.");
    }
    else if constexpr (os_name_hash == detail::macos_hash) {
        return string_literal("System branch selected by MD5 hash: This is an Apple macOS system.");
    }
    else if constexpr (os_name_hash == detail::linux_hash) {
        return string_literal("System branch selected by MD5 hash: This is a Linux-based system.");
    }
    else {
        return string_literal("System branch selected by MD5 hash: This is an unknown system.");
    }
}


int main() {
    // 整个 `get_system_details_by_md5()` 函数在编译时执行完毕
    // 它的结果是一个编译期常量，可以直接赋给一个 constexpr 变量
    constexpr auto system_info = get_system_details_by_md5();

    // 在运行时打印出这个在编译期就已确定的字符串
    std::cout << system_info.data() << std::endl;

    // 为了证明 os_name_hash 确实是编译期常量，我们可以将它用在 static_assert 中
    static_assert(os_name_hash != 0, "The OS hash was successfully computed at compile-time!");

    std::cout << "Compile-time OS name hash: 0x" << std::hex << os_name_hash << std::endl;

    return 0;
}