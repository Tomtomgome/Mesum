#ifndef M_CORE_COMMON
#define M_CORE_COMMON
#pragma once

#ifdef _M_DLL_DYNAMIC_LINK
#ifdef _M_EXPORT
#define MesumCoreApi __declspec(dllexport)
#else
#define MesumCoreApi __declspec(dllimport)
#endif
#else
#define MesumCoreApi
#endif

//PLatform specific
#if defined _DEBUG || !defined(NDEBUG)
#define M_DEBUG
#else
#define M_RELEASE
#endif

#define M_ENABLE_LOG
//#define M_ENABLE_VERBOSE_LOG

#if defined _M_WIN32
#define M_WIN32
#elif defined _M_UNIX
#define M_UNIX
#endif

#if defined _M_WINDOWED
#define M_WINDOWED_APP
#endif //_M_WINDOWED

#endif //M_CORE_COMMON