#ifndef TYPES_H
#define TYPES_H

// ClaudeOS - Custom Type Definitions
// Replaces stdint.h for kernel development

// Exact-width integer types
typedef unsigned char      uint8_t;
typedef signed char        int8_t;
typedef unsigned short     uint16_t;
typedef signed short       int16_t;
typedef unsigned int       uint32_t;
typedef signed int         int32_t;
typedef unsigned long long uint64_t;
typedef signed long long   int64_t;

// Minimum-width integer types
typedef uint8_t   uint_least8_t;
typedef int8_t    int_least8_t;
typedef uint16_t  uint_least16_t;
typedef int16_t   int_least16_t;
typedef uint32_t  uint_least32_t;
typedef int32_t   int_least32_t;
typedef uint64_t  uint_least64_t;
typedef int64_t   int_least64_t;

// Fastest minimum-width integer types
typedef uint8_t   uint_fast8_t;
typedef int8_t    int_fast8_t;
typedef uint32_t  uint_fast16_t;
typedef int32_t   int_fast16_t;
typedef uint32_t  uint_fast32_t;
typedef int32_t   int_fast32_t;
typedef uint64_t  uint_fast64_t;
typedef int64_t   int_fast64_t;

// Integer types capable of holding pointers
typedef uint32_t  uintptr_t;
typedef int32_t   intptr_t;

// Greatest-width integer types
typedef uint64_t  uintmax_t;
typedef int64_t   intmax_t;

// Size and difference types
typedef uint32_t  size_t;
typedef int32_t   ssize_t;
typedef int32_t   ptrdiff_t;

// Boolean type
typedef uint8_t   bool;
#define true      1
#define false     0

// NULL pointer
#ifndef NULL
#define NULL ((void*)0)
#endif

// Limits for integer types
#define UINT8_MAX   0xFF
#define INT8_MAX    0x7F
#define INT8_MIN    (-INT8_MAX - 1)

#define UINT16_MAX  0xFFFF
#define INT16_MAX   0x7FFF
#define INT16_MIN   (-INT16_MAX - 1)

#define UINT32_MAX  0xFFFFFFFF
#define INT32_MAX   0x7FFFFFFF
#define INT32_MIN   (-INT32_MAX - 1)

#define UINT64_MAX  0xFFFFFFFFFFFFFFFF
#define INT64_MAX   0x7FFFFFFFFFFFFFFF
#define INT64_MIN   (-INT64_MAX - 1)

#define SIZE_MAX    UINT32_MAX

#endif // TYPES_H