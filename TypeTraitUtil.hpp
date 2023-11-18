
#pragma once
#include <concepts>


namespace TypeTraitUtil{

    //arrayなどには非対応
    //https://stackoverflow.com/questions/51032671/idiomatic-way-to-write-concept-that-says-that-type-is-a-stdvector

    template < class T, template<class...> class Primary >
    struct is_specialization_of : std::false_type{
    };

    template < template<class...> class Primary, class... Args >
    struct is_specialization_of< Primary<Args...>, Primary> : std::true_type{
    };

    template < class T, template<class...> class Primary >
    inline constexpr bool is_specialization_of_v = is_specialization_of<T, Primary>::value;



    template < class T, template<class, template<class> class> class Primary>
    struct is_specialization_property_of : std::false_type{
    };

    template < template<class, template<class> class> class Primary, class... Args >
    struct is_specialization_property_of< Primary<Args...>, Primary> : std::true_type{
    };

    template < class T, template<class, template<class> class> class Primary >
    inline constexpr bool is_specialization_property_of_v = is_specialization_property_of<T, Primary>::value;



    template<class T, class U>
    concept same_common_as = std::same_as<std::remove_cvref_t<T>, std::remove_cvref_t<U>>;
}

//#include <array>
//#include <vector>
//     static_assert(TypeTraitUtil::is_specialization_of_v <  std::vector<int>,std::vector>);
//     static_assert(TypeTraitUtil::is_specialization_of_v < std::array<int,4>,std::array >);