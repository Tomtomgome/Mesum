#ifndef M_GRAPHICS_COMMON
#define M_GRAPHICS_COMMON
#pragma once

#ifdef _M_DLL_DYNAMIC_LINK
#ifdef _M_EXPORT
#define MesumGraphicsApi __declspec(dllexport)
#else
#define MesumGraphicsApi __declspec(dllimport)
#endif
#else
#define MesumGraphicsApi
#endif

#endif //M_CORE_COMMON