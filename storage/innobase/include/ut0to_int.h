#pragma once

#include <type_traits>

template <typename T>
constexpr auto to_int(T v) -> typename std::underlying_type<T>::type {
  return (static_cast<typename std::underlying_type<T>::type>(v));
}
