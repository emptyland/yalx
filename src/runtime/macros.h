#pragma once
#ifndef YALX_RUNTIME_MACROS_H_
#define YALX_RUNTIME_MACROS_H_

// OS platform macros
#if defined(_WIN32) || defined(WIN32) || defined(_WIN64)
#   define YALX_OS_WINDOWS 1
#endif

#if defined(unix) || defined(__unix) || defined(__unix__)
#   define YALX_OS_UNIX   1
#   define YALX_OS_POSIX  1
#endif

#if defined(linux) || defined(__linux) || defined(__linux__) || defined(__gnu_linux__)
#   define YALX_OS_LINUX  1
#   define YALX_OS_POSIX  1
#endif

#if defined(__APPLE__)
#   define YALX_OS_DARWIN 1
#   define YALX_OS_POSIX  1
#endif


// CPU Arch macros
#if defined(_M_IX86)
#error Not support 32bit x86 cpus
#endif
    
#if defined(__amd64) || defined(__amd64__) || defined(__x86_64) || defined(__x86_64__) || defined(_M_X64) || defined(_M_AMD64)
#   define YALX_ARCH_X64 1
#endif

#if defined(__ARM64_ARCH_8__)
#   define YALX_ARCH_ARM64 1
#endif

#if defined(__GNUC__)
#   define YALX_USE_GCC 1
#   undef YALX_CC
#   define YALX_CC "gcc"
#endif

#if defined(_MSC_VER)
#   define YALX_USE_MSC 1
#   undef YALX_CC
#   define YALX_CC "cl"
#endif

#if defined(__clang__)
#   define YALX_USE_CLANG 1
#   undef YALX_CC
#   define YALX_CC "clang"
#endif

#ifndef YALX_CC
#error No c/c++ compiler supports.
#endif
    
#define NOTHING(...)

#define USE(x) ((void)(x))


// List Macros:

#define QUEUE_HEADER(type) \
    type *next; \
    type *prev

#define QUEUE_INSERT_HEAD(h, x) \
    (x)->next = (h)->next; \
    (x)->next->prev = x; \
    (x)->prev = h; \
    (h)->next = x

#define QUEUE_INSERT_TAIL(h, x) \
    (x)->prev = (h)->prev; \
    (x)->prev->next = x; \
    (x)->next = h; \
    (h)->prev = x \

#define QUEUE_REMOVE(x) \
    (x)->next->prev = (x)->prev; \
    (x)->prev->next = (x)->next;

//    (x)->prev = NULL; \
//    (x)->next = NULL

#define QUEUE_EMPTY(h) ((h)->next == (h))


#endif // YALX_RUNTIME_MACROS_H_
