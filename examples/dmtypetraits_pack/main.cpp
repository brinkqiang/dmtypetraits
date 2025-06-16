#include "dmtypetraits_pack.h" // 唯一需要包含的头文件
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <optional>
#include <variant>
#include <array>
#include <cassert>
#include <system_error>

// 1. 定义要使用的复杂类型

// 枚举类型
enum class Status : uint8_t {
    Ok,
    Warning,
    Error
};

// 嵌套结构体
// 注意：所有需要序列化的自定义结构体都必须是聚合类型
// (没有用户自定义的构造函数、没有私有/保护的非静态成员、没有基类、没有虚函数)
struct Metadata {
    std::string author;
    uint64_t timestamp;

    // 为了断言验证，需要提供 operator==
    bool operator==(const Metadata& other) const {
        return author == other.author && timestamp == other.timestamp;
    }
};

// 主数据结构，包含多种复杂类型
struct ComplexData {
    int id;
    Status status;
    Metadata metadata; // 嵌套聚合类型
    std::optional<std::string> optional_description;
    std::variant<int, double, std::string> data_payload;
    std::map<std::string, int> properties;
    std::vector<float> sensor_readings;
    std::array<char, 4> fixed_id;
    std::pair<int, int> version;

    // 为了断言验证，需要提供 operator==
    bool operator==(const ComplexData& other) const {
        return id == other.id &&
               status == other.status &&
               metadata == other.metadata &&
               optional_description == other.optional_description &&
               data_payload == other.data_payload &&
               properties == other.properties &&
               sensor_readings == other.sensor_readings &&
               fixed_id == other.fixed_id &&
               version == other.version;
    }
};

// 用于打印 Variant 的辅助函数
void print_variant(const std::variant<int, double, std::string>& v) {
    std::visit([](const auto& value) {
        std::cout << value;
    }, v);
}


int main() {
    // 2. 创建一个复杂的对象并填充数据
    ComplexData original_data = {
        101,                                            // id
        Status::Ok,                                     // status (enum)
        {"brinkqiang", 1678886400},                     // metadata (nested struct)
        "This is an optional description.",             // optional_description
        3.14159,                                        // data_payload (variant holding a double)
        {{"property1", 10}, {"property2", 20}},         // properties (map)
        {0.1f, 0.2f, 0.3f, 0.4f, 0.5f},                 // sensor_readings (vector)
        {'A', 'B', 'C', 'D'},                           // fixed_id (array)
        {2, 1}                                          // version (pair)
    };

    // 3. 序列化对象
    std::cout << "--- Serializing ComplexData object ---" << std::endl;
    std::vector<char> buffer = dm::pack::serialize(original_data);
    std::cout << "Successfully serialized object into " << buffer.size() << " bytes." << std::endl;

    // 4. 反序列化对象
    std::cout << "\n--- Deserializing buffer back to object ---" << std::endl;
    auto [error_code, new_data] = dm::pack::deserialize<ComplexData>(buffer);

    // 5. 检查错误并验证数据完整性
    assert(error_code == std::errc{});
    std::cout << "Deserialization completed with no errors." << std::endl;
    
    assert(original_data == new_data);
    std::cout << "Data integrity verified: Original and deserialized objects are identical." << std::endl;

    // 6. 打印一些反序列化后的数据以供查看
    std::cout << "\n--- Verification Details ---" << std::endl;
    std::cout << "ID: " << new_data.id << std::endl;
    std::cout << "Status: " << static_cast<int>(new_data.status) << " (0=Ok, 1=Warning, 2=Error)" << std::endl;
    std::cout << "Metadata Author: " << new_data.metadata.author << std::endl;
    std::cout << "Optional Description: " << new_data.optional_description.value_or("N/A") << std::endl;
    std::cout << "Variant Payload: ";
    print_variant(new_data.data_payload);
    std::cout << std::endl;
    std::cout << "First Property: " << new_data.properties.begin()->first << " -> " << new_data.properties.begin()->second << std::endl;
    std::cout << "Sensor Reading Count: " << new_data.sensor_readings.size() << std::endl;
    std::cout << "Fixed ID: " << new_data.fixed_id[0] << new_data.fixed_id[1] << "..." << std::endl;
    std::cout << "Version: " << new_data.version.first << "." << new_data.version.second << std::endl;

    std::cout << "\n--- All tests passed successfully! ---" << std::endl;
    
    return 0;
}
