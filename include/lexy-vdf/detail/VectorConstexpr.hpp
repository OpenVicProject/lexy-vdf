#pragma once

#if __cpp_lib_constexpr_vector >= 201907L
#define LVDF_VECTOR_CONSTEXPR constexpr
#else
#define LVDF_VECTOR_CONSTEXPR inline
#endif