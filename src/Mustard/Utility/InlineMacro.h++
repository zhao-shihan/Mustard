#pragma once

#if defined _MSC_VER
#    define MUSTARD_STRONG_INLINE __forceinline
#else
#    define MUSTARD_STRONG_INLINE inline
#endif

#if defined __clang__
#    define MUSTARD_ALWAYS_INLINE [[clang::always_inline]] inline
#elif defined __GNUC__
#    define MUSTARD_ALWAYS_INLINE [[gnu::always_inline]] inline
#else
#    define MUSTARD_ALWAYS_INLINE MUSTARD_STRONG_INLINE
#endif

#if defined __clang__
#    define MUSTARD_NOINLINE [[clang::noinline]]
#elif defined __GNUC__
#    define MUSTARD_NOINLINE [[gnu::noinline]]
#elif defined _MSC_VER
#    define MUSTARD_NOINLINE __declspec(noinline)
#endif
