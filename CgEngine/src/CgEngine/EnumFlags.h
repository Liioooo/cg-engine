#pragma once

#include <type_traits>

// Generates bitwise operators (|, &, ^, ~, |=, &=, ^=) and a hasFlag(value, flag) helper for an enum class,
// so its enumerators can be combined and tested like bitflags. Invoke once per enum, at namespace scope.
#define CG_ENUM_FLAGS(EnumType) \
inline EnumType operator|(EnumType lhs, EnumType rhs) { \
using T = std::underlying_type_t<EnumType>; \
return static_cast<EnumType>(static_cast<T>(lhs) | static_cast<T>(rhs)); \
} \
inline EnumType operator&(EnumType lhs, EnumType rhs) { \
using T = std::underlying_type_t<EnumType>; \
return static_cast<EnumType>(static_cast<T>(lhs) & static_cast<T>(rhs)); \
} \
inline EnumType operator^(EnumType lhs, EnumType rhs) { \
using T = std::underlying_type_t<EnumType>; \
return static_cast<EnumType>(static_cast<T>(lhs) ^ static_cast<T>(rhs)); \
} \
inline EnumType operator~(EnumType value) { \
using T = std::underlying_type_t<EnumType>; \
return static_cast<EnumType>(~static_cast<T>(value)); \
} \
inline EnumType& operator|=(EnumType& lhs, EnumType rhs) { return lhs = lhs | rhs; } \
inline EnumType& operator&=(EnumType& lhs, EnumType rhs) { return lhs = lhs & rhs; } \
inline EnumType& operator^=(EnumType& lhs, EnumType rhs) { return lhs = lhs ^ rhs; } \
inline bool hasFlag(EnumType value, EnumType flag) { return (value & flag) == flag; }