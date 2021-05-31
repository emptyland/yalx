#pragma once
#ifndef YALX_RUNTIME_MACROS_H_
#define YALX_RUNTIME_MACROS_H_

// OS platform macros
#if defined(_WIN32) || defined(WIN32) || defined(_WIN64)
#   define YALX_OS_WINDOWS 1
#   define YALX_OS_POSIX   0
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
    
#if defined(__amd64) || defined(__amd64__) || defined(__x86_64) || defined(__x86_64__)
#   define YALX_ARCH_X64 1
#endif

#if defined(__ARM64_ARCH_8__)
#   define YALX_ARCH_ARM64 1
#endif
    
#define NOTHING(...)

#define USE(x) ((void)(x))


#endif // YALX_RUNTIME_MACROS_H_
