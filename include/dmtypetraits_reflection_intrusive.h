#ifndef __DMTYPETRAITS_REFLECTION_INTRUSIVE_H_INCLUDE__
#define __DMTYPETRAITS_REFLECTION_INTRUSIVE_H_INCLUDE__

#include <tuple>
#include <utility>
#include <type_traits>
#include "dmtypetraits_extensions.h"

namespace dm::refl {


template<typename T>
struct traits;

template<typename T>
inline constexpr bool dm_is_reflected_v = traits<dm_remove_cvref_t<T>>::is_reflected;


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

template<typename T, typename Visitor>
void dm_visit_members(T&& object, Visitor&& visitor) {
    using CleanT = dm_remove_cvref_t<T>;

    if constexpr (dm_is_reflected_v<CleanT>) {
        constexpr auto members_tuple = traits<CleanT>::members();

        std::apply(
            [&object, &visitor](auto&&... pairs) {
                (
                    visitor(
                        pairs.first,
                        object.*(pairs.second)
                    ),
                    ...
                );
            },
            members_tuple
        );
    }
    else {
        static_assert(dm_is_reflected_v<CleanT>,
            "dm::refl::dm_visit_members requires the type T to have a dm::refl::traits<T> specialization. Please ensure the correct .meta.h file is generated and included.");
    }
}

template<typename T>
struct traits {
    static constexpr bool is_reflected = false;
};

template<typename T, typename = void>
struct is_reflectable : std::false_type {};

template<typename T>
struct is_reflectable<T, std::void_t<decltype(traits<T>::is_reflected)>>
    : std::bool_constant<traits<T>::is_reflected> {
};

template<typename T>
constexpr bool is_reflectable_v = is_reflectable<T>::value;

template<typename ClassType, typename MemberType>
class field_descriptor {
private:
    const char* name_;
    MemberType ClassType::* ptr_;

public:
    using class_type = ClassType;
    using member_type = MemberType;

    constexpr field_descriptor(const char* name, MemberType ClassType::* ptr)
        : name_(name), ptr_(ptr) {
    }

    constexpr std::string_view name() const { return name_; }
    constexpr auto ptr() const { return ptr_; }
    constexpr const MemberType& get(const ClassType& obj) const {
        return obj.*ptr_;
    }

    constexpr MemberType& get(ClassType& obj) const {
        return obj.*ptr_;
    }

    template<typename U>
    constexpr void set(ClassType& obj, U&& value) const {
        obj.*ptr_ = std::forward<U>(value);
    }

    constexpr std::string_view type_name() const {
        return dm_type_name<MemberType>();
    }
};

template<typename T, size_t... Is>
constexpr auto make_field_descriptors_impl(std::index_sequence<Is...>) {
    constexpr auto members = traits<T>::members();
    return std::make_tuple(
        field_descriptor<T, std::remove_reference_t<decltype(std::declval<T>().*(std::get<Is>(members).second))>>(
            std::get<Is>(members).first,
            std::get<Is>(members).second
        )...
    );
}

template<typename T>
constexpr auto make_field_descriptors() {
    constexpr auto members = traits<T>::members();
    constexpr size_t member_count = std::tuple_size_v<decltype(members)>;
    return make_field_descriptors_impl<T>(std::make_index_sequence<member_count>{});
}

template<typename T>
struct type_info {
    static_assert(is_reflectable_v<T>, "Type must be reflectable");

    using type = T;
    static constexpr std::string_view name() { return traits<T>::name; }
    static constexpr auto fields() { return make_field_descriptors<T>(); }
    static constexpr size_t field_count() {
        return std::tuple_size_v<decltype(traits<T>::members())>;
    }
};


template<typename T>
constexpr std::string_view get_class_name() {
    return type_info<T>::name();
}

template<typename T>
constexpr size_t get_field_count() {
    return type_info<T>::field_count();
}

template<typename T, typename Visitor>
constexpr void visit_fields(T&& obj, Visitor&& visitor) {
    static_assert(is_reflectable_v<std::decay_t<T>>, "Type must be reflectable");

    constexpr auto fields = type_info<std::decay_t<T>>::fields();
    visit_fields_impl(std::forward<T>(obj), std::forward<Visitor>(visitor),
        fields, std::make_index_sequence<std::tuple_size_v<decltype(fields)>>{});
}

template<typename T, typename Visitor, typename Fields, size_t... Is>
constexpr void visit_fields_impl(T&& obj, Visitor&& visitor, Fields&& fields, std::index_sequence<Is...>) {
    (visitor(std::get<Is>(fields), std::get<Is>(fields).get(obj)), ...);
}

template<typename T, typename Fields, size_t... Is>
constexpr auto find_field_impl(std::string_view name, Fields&& fields, std::index_sequence<Is...>) {
    using FieldType = std::decay_t<decltype(std::get<0>(fields))>;
    std::optional<FieldType> result;

    auto check_field = [&](const auto& field) {
        if (field.name() == name && !result.has_value()) {
            result.emplace(field);
        }
        };

    (check_field(std::get<Is>(fields)), ...);
    return result;
}

template<typename T>
constexpr auto find_field(std::string_view name) {
    constexpr auto fields = type_info<T>::fields();
    return find_field_impl<T>(name, fields, std::make_index_sequence<std::tuple_size_v<decltype(fields)>>{});
}

template<size_t Index, typename T>
constexpr auto get_field() {
    constexpr auto fields = type_info<T>::fields();
    static_assert(Index < std::tuple_size_v<decltype(fields)>, "Field index out of range");
    return std::get<Index>(fields);
}

template<typename T>
class object_accessor {
private:
    T* obj_;

public:
    explicit object_accessor(T& obj) : obj_(&obj) {}

    template<size_t Index>
    auto& get() const {
        constexpr auto field = get_field<Index, T>();
        return field.get(*obj_);
    }

    template<size_t Index, typename U>
    void set(U&& value) const {
        constexpr auto field = get_field<Index, T>();
        field.set(*obj_, std::forward<U>(value));
    }

    auto get(std::string_view name) const -> std::optional<std::string> {
        bool found = false;
        std::string result;

        visit_fields(*obj_, [&](const auto& field, const auto& value) {
            if (!found && field.name() == name) {
                if constexpr (std::is_arithmetic_v<std::decay_t<decltype(value)>>) {
                    result = std::to_string(value);
                }
                else if constexpr (std::is_convertible_v<decltype(value), std::string>) {
                    result = value;
                }
                else {
                    result = "[complex_type]";
                }
                found = true;
            }
            });

        return found ? std::optional<std::string>{result} : std::nullopt;
    }

    template<typename U>
    bool set(std::string_view name, U&& value) const {
        bool found = false;

        visit_fields(*obj_, [&](const auto& field, auto& field_value) {
            if (!found && field.name() == name) {
                using FieldType = std::decay_t<decltype(field_value)>;
                if constexpr (std::is_assignable_v<FieldType&, U>) {
                    field.set(*obj_, std::forward<U>(value));
                    found = true;
                }
            }
            });

        return found;
    }
};

template<typename T>
auto make_accessor(T& obj) {
    return object_accessor<T>(obj);
}

} // namespace dm::refl

#endif // __DMTYPETRAITS_REFLECTION_INTRUSIVE_H_INCLUDE__