#ifndef __DMTYPETRAITS_REFLECTION_INTRUSIVE_H_INCLUDE__
#define __DMTYPETRAITS_REFLECTION_INTRUSIVE_H_INCLUDE__

#include <tuple>
#include <utility>
#include <type_traits>
#include "dmtypetraits_extensions.h"

// Note: The specific .meta.h file is NOT included here.
// It should be included by the user code (e.g., main.cpp)
// before this header or before the call to the functions below.

namespace dm::refl {

// The traits base template is now expected to be in the generated .meta.h file
// But we can forward-declare it for safety.
template<typename T>
struct traits;

// --- Check if a type T has reflection data via traits specialization ---
template<typename T>
inline constexpr bool dm_is_reflected_v = traits<dm_remove_cvref_t<T>>::is_reflected;


// --- Utility to get the member count from traits ---
template<typename T>
constexpr size_t dm_member_count() {
    using CleanT = dm_remove_cvref_t<T>;
    if constexpr (dm_is_reflected_v<CleanT>) {
        constexpr auto members_tuple = traits<CleanT>::members();
        return std::tuple_size_v<decltype(members_tuple)>;
    }
    else {
        return 0;
    }
}

/**
 * @brief Visits all members of a struct using its dm::refl::traits specialization.
 */
template<typename T, typename Visitor>
void dm_visit_members(T&& object, Visitor&& visitor) {
    using CleanT = dm_remove_cvref_t<T>;

    // Use `if constexpr` inside the body instead of SFINAE in the signature.
    if constexpr (dm_is_reflected_v<CleanT>) {
        // 1. Get the metadata tuple from the generated traits specialization
        constexpr auto members_tuple = traits<CleanT>::members();

        // 2. Use std::apply to unpack the tuple of pairs
        std::apply(
            // 3. The lambda receives each std::pair as a separate argument
            [&object, &visitor](auto&&... pairs) {
                // 4. Use a C++17 fold expression to iterate over all pairs
                (
                    // For each pair, call the user's visitor
                    visitor(
                        pairs.first,            // The member name (e.g., "author")
                        object.*(pairs.second)  // The member's value
                    ),
                    ...
                );
            },
            members_tuple
        );
    }
    else {
        // Provide a clear compile-time error if the type is not reflected.
        static_assert(dm_is_reflected_v<CleanT>,
            "dm::refl::dm_visit_members requires the type T to have a dm::refl::traits<T> specialization. Please ensure the correct .meta.h file is generated and included.");
    }
}

} // namespace dm::refl

#endif // __DMTYPETRAITS_REFLECTION_INTRUSIVE_H_INCLUDE__