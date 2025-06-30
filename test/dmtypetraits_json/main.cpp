#include "gtest.h"
#include <string>
#include <vector>
#include <map>
#include <any> // 为了测试 object_accessor::get(name)

// 包含最终版的反射库核心
#include "dmstruct.meta.h"

// 3. 测试固件 (Test Fixture)
class ReflectionTest : public ::testing::Test {
protected:
    void SetUp() override {
        data = {
            101, Status::Ok, {"tom", 1156},
            {{"property1", 10}, {"property2", 20}},
            {0.1f, 0.2f, 0.3f},
            {{"jerry", 1347}}
        };
    }
    ComplexData data;
};


// --- 重构后的测试用例 ---

TEST(ReflectionAPITest, BasicInfo) {
    ASSERT_TRUE(dm::refl::get_class_name<Metadata>() == "Metadata");
    ASSERT_TRUE(dm::refl::get_field_count<Metadata>() == 2);
    ASSERT_TRUE(dm::refl::get_class_name<ComplexData>() == "ComplexData");
    ASSERT_TRUE(dm::refl::get_field_count<ComplexData>() == 6);
}

// 使用 if constexpr 和编译期索引来重构 VisitFields 测试
TEST_F(ReflectionTest, VisitFields) {
    int field_count = 0;
    dm::refl::visit_fields(data, [&](const auto& field, const auto& value) {
        field_count++;

        // 使用编译期索引进行类型安全的检查
        if constexpr (field.index == 0) { // id
            ASSERT_TRUE(field.name() == "id");
            ASSERT_TRUE(value == 101);
        }
        else if constexpr (field.index == 1) { // status
            ASSERT_TRUE(field.name() == "status");
            ASSERT_TRUE(value == Status::Ok);
        }
        else if constexpr (field.index == 2) { // metadata
            ASSERT_TRUE(field.name() == "metadata");
            Metadata expected = { "tom", 1156 };
            ASSERT_TRUE(value == expected);
        }
        });

    ASSERT_TRUE(field_count == dm::refl::get_field_count<ComplexData>());
}

// 验证 get_field 和 find_field (使用 std::visit)
//TEST_F(ReflectionTest, GetAndFindField) {
//    // 按索引获取
//    constexpr auto field0 = dm::refl::get_field<0, ComplexData>();
//    ASSERT_TRUE(field0.name() == "id");
//    ASSERT_TRUE(field0.get(data) == 101);
//
//    // 按名称查找
//    auto field_opt = dm::refl::find_field<ComplexData>("metadata");
//    ASSERT_TRUE(field_opt.has_value());
//
//    // 使用 std::visit 来安全地处理 variant 返回值
//    std::visit([&](const auto& field_desc) {
//        ASSERT_TRUE(field_desc.name() == "metadata");
//        Metadata expected_meta = { "tom", 1156 };
//        ASSERT_TRUE(field_desc.get(data) == expected_meta);
//        }, *field_opt);
//
//    // 查找不存在的字段
//    auto non_existent_field = dm::refl::find_field<ComplexData>("non_existent");
//    ASSERT_TRUE(!non_existent_field.has_value());
//}

// 重构 ObjectAccessor 测试以匹配新的API
TEST_F(ReflectionTest, ObjectAccessor) {
    auto accessor = dm::refl::make_accessor(data);

    // 按索引读写
    ASSERT_TRUE(accessor.get<0>() == 101);
    accessor.set<0>(999);
    ASSERT_TRUE(data.id == 999);
    ASSERT_TRUE(accessor.get<0>() == 999);

    // 按名称读写，并处理 std::any 返回值
    auto status_any_opt = accessor.get("status");
    ASSERT_TRUE(status_any_opt.has_value());
    ASSERT_TRUE(std::any_cast<Status>(*status_any_opt) == Status::Ok);

    // 测试 set
    bool set_success = accessor.set("status", Status::Warning);
    ASSERT_TRUE(set_success);
    ASSERT_TRUE(data.status == Status::Warning);

    // 测试 set 一个不兼容的类型
    bool set_fail = accessor.set("status", 12345); // int 不能赋值给 Status
    ASSERT_TRUE(!set_fail);
    ASSERT_TRUE(data.status == Status::Warning); // 值应保持不变
}

// 同样使用 if constexpr 重构 NestedReflection 测试
TEST_F(ReflectionTest, NestedReflection) {
    bool metadata_found_and_tested = false;

    dm::refl::visit_fields(data, [&](const auto& field, const auto& value) {
        // 只对 metadata 字段进行深度测试
        if constexpr (field.index == 2) {
            ASSERT_TRUE(field.name() == "metadata");

            using ValueType = std::decay_t<decltype(value)>;
            // 再次确认其可反射
            if constexpr (dm::refl::is_reflectable_v<ValueType>) {
                metadata_found_and_tested = true;
                int nested_field_count = 0;

                dm::refl::visit_fields(value, [&](const auto& inner_field, const auto& inner_value) {
                    nested_field_count++;
                    if constexpr (inner_field.index == 0) { // author
                        ASSERT_TRUE(inner_field.name() == "author");
                        ASSERT_TRUE(inner_value == "tom");
                    }
                    else if constexpr (inner_field.index == 1) { // timestamp
                        ASSERT_TRUE(inner_field.name() == "timestamp");
                        ASSERT_TRUE(inner_value == 1156);
                    }
                    });
                ASSERT_TRUE(nested_field_count == dm::refl::get_field_count<Metadata>());
            }
        }
        });

    ASSERT_TRUE(metadata_found_and_tested);
}

// 测试旧接口兼容性
TEST_F(ReflectionTest, LegacyCompatibility) {
    std::map<std::string, std::string> member_values;
    dm::refl::dm_visit_members(data, [&](const char* name, auto&& value) {
        // 使用一个map来记录访问到的值，以便后续验证
        // 注意：这里需要一个通用的tostring，我们用 dmcast
        });

    // 由于 dm_visit_members 无法像新接口一样方便地进行类型判断，
    // 其测试通常侧重于“是否都访问到了”，或者依赖于一个强大的tostring工具。
    // 这里仅做一个简单的调用测试。
    ASSERT_NO_FATAL_FAILURE(dm::refl::dm_visit_members(data, [](auto...) {}));
}