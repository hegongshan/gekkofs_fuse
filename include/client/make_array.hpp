/*
  Copyright 2018-2021, Barcelona Supercomputing Center (BSC), Spain
  Copyright 2015-2021, Johannes Gutenberg Universitaet Mainz, Germany

  This software was partially supported by the
  EC H2020 funded project NEXTGenIO (Project ID: 671951, www.nextgenio.eu).

  This software was partially supported by the
  ADA-FS project under the SPPEXA project funded by the DFG.

  SPDX-License-Identifier: MIT
*/

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
