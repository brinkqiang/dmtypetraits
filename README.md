# dmtypetraits: 现代C++17反射与序列化库

Copyright (c) 2013-2018 brinkqiang (brink.qiang@gmail.com)

[![dmtypetraits](https://img.shields.io/badge/brinkqiang-dmtypetraits-blue.svg?style=flat-square)](https://github.com/brinkqiang/dmtypetraits)
[![License](https://img.shields.io/badge/license-MIT-brightgreen.svg)](https://github.com/brinkqiang/dmtypetraits/blob/master/LICENSE)
[![blog](https://img.shields.io/badge/Author-Blog-7AD6FD.svg)](https://brinkqiang.github.io/)
[![Open Source Love](https://badges.frapsoft.com/os/v3/open-source.png)](https://github.com/brinkqiang)
[![GitHub stars](https://img.shields.io/github/stars/brinkqiang/dmtypetraits.svg?label=Stars)](https://github.com/brinkqiang/dmtypetraits) 
[![GitHub forks](https://img.shields.io/github/forks/brinkqiang/dmtypetraits.svg?label=Fork)](https://github.com/brinkqiang/dmtypetraits)

## Build status
| [Linux][lin-link] | [Mac][mac-link] | [Windows][win-link] |
| :---------------: | :----------------: | :-----------------: |
| ![lin-badge]      | ![mac-badge]       | ![win-badge]        |

[lin-badge]: https://github.com/brinkqiang/dmtypetraits/workflows/linux/badge.svg "linux build status"
[lin-link]:  https://github.com/brinkqiang/dmtypetraits/actions/workflows/linux.yml "linux build status"
[mac-badge]: https://github.com/brinkqiang/dmtypetraits/workflows/mac/badge.svg "mac build status"
[mac-link]:  https://github.com/brinkqiang/dmtypetraits/actions/workflows/mac.yml "mac build status"
[win-badge]: https://github.com/brinkqiang/dmtypetraits/workflows/win/badge.svg "win build status"
[win-link]:  https://github.com/brinkqiang/dmtypetraits/actions/workflows/win.yml "win build status"


`dmtypetraits` 是一个轻量级、纯头文件的C++17库，它为C++类型（特别是聚合类型）提供了强大的编译期反射、动态访问和序列化能力。其设计旨在简化元编程，减少样板代码，并以一种现代化、类型安全的方式处理复杂数据结构。

## ✨ 核心特性

  - **纯头文件**: 无需编译和链接，只需包含头文件即可使用。
  - **C++17 标准**: 充分利用 C++17 的特性（如折叠表达式、`if constexpr`）提供简洁高效的实现。
  - **双层反射系统**:
    1.  **基础反射**: 对聚合类型提供零开销的编译期成员访问。
    2.  **高级反射**: 通过元数据文件，实现字段名、类型名、动态读写等更强大的功能。
  - **序列化支持**:
      - **JSON 序列化**: 一行代码将反射对象转换为 JSON 字符串。
      - **二进制打包**: 高效的二进制序列化与反序列化，适用于存储或网络传输。
  - **类型安全访问器**: 提供优雅的 `accessor` 模式，支持按编译期索引和运行期名称对成员进行类型安全的读写操作。
  - **易于扩展**: 设计上支持嵌套结构、STL容器（如 `vector`, `map`, `array`, `pair`）等复杂类型。

## ⚙️ 快速入门

由于是纯头文件库，集成非常简单：

1.  将 `dmtypetraits` 的头文件目录添加到您的项目包含路径中。
2.  在您的代码中包含主头文件：`#include "dmtypetraits.h"`。

对于高级反射和JSON序列化，您可能需要包含一个由工具生成的元数据头文件（例如示例中的 `dmstruct.meta.h`）。

-----

## 📚 API 使用指南

### 1\. 基础反射 (适用于聚合类型)

这套API适用于任何聚合类型（无自定义构造函数、无私有/保护成员、无基类、无虚函数），无需任何宏或代码生成。

#### 获取成员数量

使用 `dm_member_count_v` 可以得到一个聚合类型的成员数量。

```cpp
#include "dmtypetraits.h"
#include <iostream>

struct Player {
    int id;
    std::string name;
    double score;
};

int main() {
    // Player 是聚合类型, 成员数量为 3
    std::cout << "Player has " << dm_member_count_v<Player> << " members." << std::endl;
}
```

#### 遍历并访问成员

使用 `dm_visit_members` 可以遍历一个对象的所有成员。通过传递一个接受引用的 lambda，您可以直接修改成员的值。

```cpp
#include "dmtypetraits.h"
#include <iostream>
#include <string>

struct Player {
    int id;
    std::string name;
    double score;
};

int main() {
    Player player{ 101, "brink", 99.5 };

    // 遍历并打印成员
    std::cout << "Visiting members of player:" << std::endl;
    dm_visit_members(player, [](const auto&... members) {
        // 使用C++17折叠表达式打印所有成员
        ((std::cout << "  - Member value: " << members << std::endl), ...);
    });

    // 遍历并修改成员
    std::cout << "\nOriginal score: " << player.score << std::endl;
    dm_visit_members(player, [](int& id, std::string& name, double& score) {
        score = 100.0; // 直接修改分数
    });
    std::cout << "Modified score: " << player.score << std::endl;
}
```

### 2\. 高级反射 (基于元数据)

这套API功能更强大，依赖于一个描述结构体信息的元数据文件（例如 `dmstruct.meta.h`）。它提供了对字段名、类型名和动态访问的支持。

元数据文件 通过dmgen4meta --PKG=dmstruct.pkg生成. 具体可参考 ./examples/dmtypetraits_meta 

```cpp
// 假设 dmstruct.meta.h 定义了 ComplexData
#include "dmstruct.meta.h"
#include <iostream>

// ComplexData, Status, Metadata 等类型定义在元数据文件中
ComplexData data{ 101, Status::Ok, {"tom", 1156}, ... };
```

#### 获取类信息

```cpp
// 获取类名
std::cout << "Class Name: " << dm::refl::get_class_name<ComplexData>() << std::endl;

// 获取字段数量
std::cout << "Field Count: " << dm::refl::get_field_count<ComplexData>() << std::endl;
```

#### 增强的字段遍历

`dm::refl::visit_fields` 的访问器lambda会接收到一个 `field` 对象和一个 `value`，`field` 对象包含了索引、名称、类型名等元信息。

```cpp
dm::refl::visit_fields(data, [](const auto& field, const auto& value) {
    std::cout << "  - Field: " << field.name()
              << ", Type: " << field.type_name()
              << ", Value: " << dmcast::lexical_cast<std::string>(value) << std::endl;
});
```

#### 对象访问器 (Accessor)

`make_accessor` 是最优雅的成员访问方式。它创建了一个访问器对象，允许按索引（编译期）或名称（运行期）进行类型安全的读写。

```cpp
auto accessor = dm::refl::make_accessor(data);

// 按索引读写 (编译期检查，零开销)
accessor.set<0>(999); // 设置 id
std::cout << "ID by index: " << accessor.get<0>() << std::endl; // 输出 999

// 按名称读写 (运行期查找)
if (auto value = accessor.get("status")) {
    // value 是一个 optional-like 的对象
    std::cout << "Status by name: " << static_cast<int>(*value) << std::endl;
}

// 类型安全设置: 如果类型不匹配，设置会失败
accessor.set("status", Status::Warning); // 成功
bool failed = accessor.set("status", 12345); // 失败, int 不能赋值给 Status
ASSERT_FALSE(failed);
```

### 3\. 序列化

`dmtypetraits` 提供了开箱即用的 JSON 和二进制序列化功能。

#### JSON 序列化

使用 `dmcast::lexical_cast` 可以轻松地将一个可反射的对象转换为格式化的 JSON 字符串。这对于调试、日志记录或与Web API交互非常有用。

```cpp
#include "dmstruct.meta.h"
#include "dmjson.h" // 引入JSON转换能力
#include <iostream>

int main() {
    ComplexData data{
        101, Status::Ok, {"tom", 1156},
        {{"property1", 10}, {"property2", 20}},
        {0.1f, 0.2f, 0.3f},
        {{"jerry", 1347}}
    };

    // 一行代码转换为JSON字符串
    std::string json_str = dmcast::lexical_cast<std::string>(data);

    std::cout << json_str << std::endl;
    // 输出:
    // {"id":101,"status":0,"metadata":{"author":"tom","timestamp":1156}, ...}
}
```

#### 二进制序列化 (Pack)

对于需要高性能和紧凑体积的场景（如IPC、文件存储），可以使用 `dm::pack` 模块。

```cpp
#include "dmtypetraits.h"
#include <iostream>
#include <vector>
#include <cassert>

// 使用 dmtypetraits_pack.cpp 中的 ComplexData 结构体
// ...

int main() {
    ComplexData original_data = { 101, Status::Ok, ... };

    // 1. 序列化
    std::vector<char> buffer = dm::pack::serialize(original_data);
    std::cout << "Serialized into " << buffer.size() << " bytes." << std::endl;

    // 2. 反序列化
    auto [error_code, new_data] = dm::pack::deserialize<ComplexData>(buffer);

    // 3. 验证
    assert(error_code == std::errc{}); // 检查错误码
    assert(original_data == new_data); // 验证数据一致性
    std::cout << "Verification successful!" << std::endl;
}
```

-----

## 💡 总结

`dmtypetraits` 是一个功能全面且设计现代的C++工具库。它通过强大的反射机制，极大地简化了对象的通用编程，无论是简单的成员遍历，还是复杂的动态访问和跨格式序列化，都能以非常简洁和高效的方式完成。它是构建需要进行大量数据操作和转换的C++应用程序的理想选择。

## Contacts

## Thanks
