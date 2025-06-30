
#include "dmfix_win_utf8.h"
#include "dmstruct.meta.h"

#include "dmjson.h"

int main() {

    MyStruct s{ 10, "hello" };

    // 1. 基础信息查询
    std::cout << "--- 1. 基础信息查询 ---\n";
    std::cout << "类名: " << dm::refl::get_class_name<MyStruct>() << std::endl;
    std::cout << "字段数量: " << dm::refl::get_field_count<MyStruct>() << std::endl;

    // 2. 现代化字段遍历
    std::cout << "\n--- 2. 现代化字段遍历 ---\n";
    dm::refl::visit_fields(s, [](const auto& field, const auto& value) {
        std::cout << "  - 字段: " << std::left << std::setw(15) << (std::string(field.name()) + ",")
            << "类型: " << std::left << std::setw(20) << (std::string(field.type_name()) + ",")
            << "值: " << value << std::endl;
        });

    // 3. 对象访问器 - 最优雅的使用方式
    std::cout << "\n--- 3. 对象访问器 ---\n";
    auto accessor = dm::refl::make_accessor(s);

    std::cout << "  - 按索引访问 [0]: " << accessor.get<0>() << std::endl;
    accessor.set<0>(42);
    std::cout << "  - 设置后 [0] 的值: " << accessor.get<0>() << std::endl;

    if (auto value = accessor.get("bar")) {
        std::cout << "  - 按名称访问 'bar': " << *value << std::endl;
    }
    accessor.set("bar", std::string("world"));
    std::cout << "  - 设置后 'bar' 的值: " << accessor.get<1>() << std::endl;

    // 4. 字段查找和直接操作
    std::cout << "\n--- 4. 字段查找与操作 ---\n";
    bool found = false;
    // 这里使用 s 的当前值: {foo: 42, bar: "world"}
    dm::refl::visit_fields(s, [&](const auto& field, auto& value) {
        if (field.name() == "foo" && !found) {
            std::cout << "  - 找到字段 '" << field.name() << "'，当前值: " << value << std::endl;
            value = 100; // 直接修改值
            std::cout << "  - 修改后新值: " << value << std::endl;
            found = true;
        }
        });

    // 5. 复杂数据结构处理
    ComplexData data{
        101, Status::Ok, {"tom", 1156},
        {{"property1", 10}, {"property2", 20}},
        {0.1f, 0.2f, 0.3f}, {{"jerry", 1347}}
    };

    std::string json_str = dmcast::lexical_cast<std::string>(data);

    try
    {
        // 将解析代码放入 try 块
        nlohmann::json json = nlohmann::json::parse(json_str);
        // 只有在解析成功后，这行代码才会被执行
        std::cout << "解析成功，格式化输出:\n" << json.dump(4) << std::endl;
    }
    catch (const nlohmann::json::parse_error& e)
    {
        // 如果解析失败，捕获异常并打印错误信息
        std::cerr << "JSON 解析失败!\n";
        std::cerr << "错误信息: " << e.what() << std::endl;
    }

    std::cout << "\n--- 5. 复杂数据结构处理 ---\n";
    std::cout << "遍历对象 '" << dm::refl::get_class_name<ComplexData>() << "':\n";
    dm::refl::visit_fields(data, [](const auto& field, const auto& value) {
        std::cout << "  - 字段 '" << field.name() << "':\n";

        // 修正: 为 decay_t 和 is_reflectable_v 提供正确的模板参数
        using ValueType = std::decay_t<decltype(value)>;
        if constexpr (dm::refl::is_reflectable_v<ValueType>) {
            std::cout << "    -> 是一个可反射对象 '" << dm::refl::get_class_name<ValueType>() << "', 值为: " << dmcast::lexical_cast<std::string>(value) << "\n";
            std::cout << "    -> 递归遍历其成员:\n";
            dm::refl::visit_fields(value, [](const auto& inner_field, const auto& inner_value) {
                std::cout << "        - " << inner_field.name() << ": " << inner_value << std::endl;
                });
        }
        else {
            std::cout << "    -> 是一个普通类型, 值为: " << dmcast::lexical_cast<std::string>(value) << std::endl;
        }
        });

    // 6. 兼容原有接口
    std::cout << "\n--- 6. 兼容原有接口 ---\n";
    // 注意 s 的值现在是 {foo: 100, bar: "world"}
    dm::refl::dm_visit_members(s, [](const char* name, auto&& value) {
        std::cout << "  - 传统方式访问 '" << name << "': " << value << std::endl;
        });

    std::cout << "\n-------------------------\n";

    return 0;
}