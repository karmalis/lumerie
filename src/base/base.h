#ifndef BASE_H_
#define BASE_H_

#include <stdio.h>

#ifndef EXIT_FAILURE
#define EXIT_FAILURE -1
#endif

#ifndef __cplusplus // C++ has its own bool, true, false
    #ifndef bool
        typedef int bool;
    #endif
    #ifndef true
        #define true 1
    #endif
    #ifndef false
        #define false 0
    #endif
#endif

#ifndef NULL
    #define NULL ((void *)0)
#endif

// Void function type
typedef void void_func(void);
typedef void (*void_func_ptr)(void);

#define BILLION 1000000000L

#define u32_max 4294967295

#ifndef __cplusplus
#define nullptr ((void*)0)
#endif

#if defined (__clang__)
#define COMPILER_CLANG
#elif defined(_MSC_VER)
#define COMPILER_CL
#elif defined(__GNUC__)
#define COMPILER_GCC
#endif

#if defined(COMPILER_CLANG)
#define FILE_NAME __FILE_NAME__
#else
#define FILE_NAME __FILE__
#endif

#define Statement(s) do {s} while(0)

#define flush fflush(stdout)
#define _v_trace Statement(printf("%s:%d: Trace\n", FILE_NAME, __LINE__); flush;)
#define unreachable Statement(printf("How did we get here? In %s:d\n", FILE_NAME, __LINE__); flush;)

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
#error "Codebase supports only linux"
#elif defined(__linux__) || defined(__gnu_linux__)
#define PLATFORM_LINUX
#else
#error "Codebase supports only linux"
#endif

//#define PATH_MAX = 4096

#if defined(PLATFORM_LINUX)
#include <unistd.h>
#define get_cw getcwd
#endif

#if defined(COMPILER_CL) || defined(COMPILER_CLANG)
#  define dll_export __declspec(dllexport)
#  define dll_import __declspec(dllimport)
#elif defined (COMPILER_GCC)
#  define dll_export __attribute__((dllexport))
#  define dll_import __attribute__((dllimport))
#else
#  error dll_export not defined for this compiler
#endif

#ifdef IS_PLUGIN
#define dll_plugin_api
#else
#define dll_plugin_api dll_export
#endif

#define array_size(arr) (sizeof(arr)/sizeof((arr)[0]))

#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))

#define clamp(val, min, max) (MAX((min), MIN((val), (max))))
#define reverse_clamp(val, min, max) (MIN((min), MAX((val), (max))))
#define wrap(a,x,b) reverse_clamp(a,x,b)

#define swap(a, b, type) Statement(type temp = (a); (a) = (b); (b) = temp;)

#define unused(x) (void)(x)

#if defined(_DEBUG)
#define assert_that(c, format, ...) Statement(           \
    if(!(c)){                                           \
    printf("%s:%d: Error: ", FILE_NAME, __LINE__);      \
    printf("Assertion Failure: ");                      \
    printf(format, ##__VA_ARGS__);                      \
    printf("\n");                                       \
    exit(-1)                                            \
}                                                       \
)
#else
#define assert_that(c, format, ...) Statement()
#endif

#define error_print(msg, ...) fprintf(stderr, "ERROR: %s:%d: " msg "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#define fatal_msg(msg, ...) Statement ( error_print(msg, ##__VA_ARGS__);  exit(EXIT_FAILURE);)

#define set_bit(val, bit)       ((val) |= (1ULL << (bit)))
#define clear_bit(val, bit)     ((val) &= ~(1ULL << (bit)))
#define toggle_bit(val, bit)    ((val) ^= (1ULL << (bit)))
#define check_bit(val, bit)     (((val) >> (bit)) & 1ULL)

#ifndef abs
   // #define abs(x) ((x)<0?-(x):(x))
#endif

#define gigabytes(count) (u64) (count * 1024 * 1024 * 1024)
#define megabytes(count) (u64) (count * 1024 * 1024)
#define kilobytes(count) (u64) (count * 1024)


#define memory_copy(d,s,z) memove((d), (s), (z))
#define memory_copy_struct(d,s) memory_copy((d),(s), Min(sizeof(*(d)), sizeof(*(s))))
#define memory_zero(d,z) memset((d), 0, (z))
#define memory_zero_struct(d,z) memory_zero((d), sizeof(s))

#define slice_prototype(type) typedef struct type##_slice { type* elems; u32 len; } type##_slice;
#define slice(type) type##_slice

#define iterate(array, var) for (int var = 0; var < array.len; var++)
#define iterate_ptr(array, var) for (int var = 0; var < array->len; var++)



///////////////////////////////////////////////
// Math
#define EPSILON 0.001f
#define PI 3.141592653589f
#define PHI 1.61803399f
#define HALF_PI 1.570796326794f
#define DEG_TO_RAD 0.0174532925f
#define RAD_TO_DEG 57.2957795131
#define FLOAT_MAX 340282346638528859811704183484516925440.0000000000000000
#define FLOAT_MIN -FLOAT_MAX

//#define EpsilonEquals(x, y) ((fabs))

#endif // BASE_H_
