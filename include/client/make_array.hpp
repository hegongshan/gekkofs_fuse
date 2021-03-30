#ifndef LIBGKFS_UTILS_MAKE_ARRAY_HPP
#define LIBGKFS_UTILS_MAKE_ARRAY_HPP

#include <array>

namespace gkfs::utils {

template <typename... T>
constexpr auto
make_array(T&&... values) -> std::array<
        typename std::decay<typename std::common_type<T...>::type>::type,
        sizeof...(T)> {
    return std::array<
            typename std::decay<typename std::common_type<T...>::type>::type,
            sizeof...(T)>{std::forward<T>(values)...};
}

} // namespace gkfs::utils

#endif // LIBGKFS_UTILS_MAKE_ARRAY_HPP
