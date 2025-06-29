#include <iostream>

#include "dmstruct.meta.h"

int main() {
    MyStruct s{ 10, "hello" };

    // Print class name from traits
    std::cout << "Class: " << dm::refl::traits<MyStruct>::name << std::endl;

    // Use the generic visitor
    dm::refl::dm_visit_members(s, [](const char* name, auto&& value) {
        // Here you have the member name, value, and can get the type.
        using MemberType = dm_remove_cvref_t<decltype(value)>;
        std::cout << " - Member: " << name
            << ", Type: " << dm_type_name<MemberType>()
            << ", Value: " << value
            << std::endl;
        });


    ComplexData original_data = {
        101,                                            // id
        Status::Ok,                                     // status (enum)
        {"tom", 1156},                     // metadata (nested struct)
        {{"property1", 10}, {"property2", 20}},         // properties (map)
        {0.1f, 0.2f, 0.3f, 0.4f, 0.5f},                 // sensor_readings (vector)
        {{"jerry", 1347}}
    };

    std::cout << "Class: " << dm::refl::traits<ComplexData>::name << std::endl;


    dm::refl::dm_visit_members(original_data, [](const char* name, auto&& value) {
        // Here you have the member name, value, and can get the type.
        using MemberType = dm_remove_cvref_t<decltype(value)>;
        std::cout << " - Member: " << name
            << ", Type: " << dm_type_name<MemberType>()
            << ", Value: " << dmcast::lexical_cast<std::string>(value)
            << std::endl;
        });
}