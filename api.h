// ordercpp by Ricardo Marmolejo Garcia is licensed under a Creative Commons
// Reconocimiento 4.0 Internacional License.
// http://creativecommons.org/licenses/by/4.0/
//
#ifndef _ORDER_CPP_API_H_
#define _ORDER_CPP_API_H_

#define ORDERCPP_VERSION_MAJOR 1
#define ORDERCPP_VERSION_MINOR 0
#define ORDERCPP_VERSION ((ORDERCPP_VERSION_MAJOR << 16) | ORDERCPP_VERSION_MINOR)

#ifdef _WIN32
    #ifdef ordercpp_EXPORTS
        #define ordercpp_API __declspec(dllexport)
    #else
        #ifndef ordercpp_STATIC
            #define ordercpp_API __declspec(dllimport)
        #else
            #define ordercpp_API
        #endif
    #endif
#else
    #if __GNUC__ >= 4
        #define order_API __attribute__((visibility("default")))
    #else
        #define order_API
    #endif
#endif

#ifdef _WIN32
using int64 = __int64;
using uint64 = unsigned __int64;
#else
using int64 = long long;
using uint64 = unsigned long long;
#endif

using int32 = signed int;
using int16 = signed short;
using int8 = signed char;

using uint32 = unsigned int;
using uint16 = unsigned short;
using uint8 = unsigned char;

using real64 = double;
using real32 = float;

using int = int32;
using uint = uint32;
using real = real32;

#endif

