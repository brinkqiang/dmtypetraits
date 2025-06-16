#include "dmtypetraits_pack.h" // The only header you need to include
#include <cassert>
#include <iostream>
#include <string>
#include <vector>

struct MyData {
    int id;
    std::string message;
    std::vector<double> values;

    bool operator==(const MyData& other) const {
        return id == other.id && message == other.message && values == other.values;
    }
};

int main() {
    // 1. Create an object
    MyData original_data = {42, "Hello, high-level API!", {1.0, 2.0, 3.0}};

    // 2. Serialize using the most convenient function
    std::vector<char> buffer = dm::pack::serialize(original_data);

    std::cout << "Serialized " << buffer.size() << " bytes." << std::endl;

    // 3. Deserialize using the result-based function
    auto [error_code, new_data] = dm::pack::deserialize<MyData>(buffer);

    // 4. Check for errors and verify data
    assert(error_code == std::errc{});
    assert(original_data == new_data);

    std::cout << "Successfully serialized and deserialized!" << std::endl;
    std::cout << "Deserialized Message: " << new_data.message << std::endl;
    
    return 0;
}