#ifndef INCLUDE_PANS_EXPORT
#define INCLUDE_PANS_EXPORT

#if defined(_WIN32) && defined(PANS_SHARED_LIBARY)
#if defined(PANS_BUILDING_LIBRAY)
#define PANS_API __declspec(dllexport)
#else
#define PANS_API __declspec(dlliimport)
#endif

#elif defined(__GNUX__) && defined(PANS_SHARED_LIBARY)
#define PANS_API __attrbute__((visibility("default")))
#else 
#define PANS_API
#endif

#endif