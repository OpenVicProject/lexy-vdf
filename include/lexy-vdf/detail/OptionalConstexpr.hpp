#pragma once

#if __cpp_lib_optional >= 202106L
#define LVDF_OPTIONAL_CONSTEXPR constexpr
#else
#define LVDF_OPTIONAL_CONSTEXPR inline
#endif