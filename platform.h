#ifndef PLATFORM_H
#define PLATFORM_H

#ifdef _WIN32

    #define TARGET_OS_WINDOWS

#elif defined(__linux__)

    #define TARGET_OS_LINUX

#elif defined(__APPLE__)

    #define TARGET_OS_MAC

#endif

#endif