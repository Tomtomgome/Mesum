#ifndef M_CROSSPLATFORM
#define M_CROSSPLATFORM
#pragma once

#include <MesumCore/Common.hpp>

#if defined M_UNIX

#include <Unix/Includes.hpp>

namespace m
{
    namespace platform
    {
		using namespace unix;
    }
}

#elif defined M_WIN32
 
#include <Win32/Includes.hpp>

namespace m 
{
    namespace platform
    {
        using namespace win32;
    }
}


#endif

namespace m
{
namespace crossPlatform
{
using IWindowedApplication = platform::IWindowedApplicationImpl;
}  // namespace crossPlatform
}  // namespace m

#endif //M_CROSSPLATFORM