#ifndef M_CROSSPLATFORM
#define M_CROSSPLATFORM
#pragma once

#if defined M_UNIX

#include <PlatUnix/UnixApp.hpp>

namespace m
{
    namespace platform
    {
        extern const logging::ChannelID CROSSPLAT_LOG_ID;
		using namespace platUnix;
    }
}

#elif defined M_WIN32
 
#include <PlatWindows/WindowsApp.hpp>

namespace m 
{
    namespace platform
    {
        extern const logging::ChannelID CROSSPLAT_LOG_ID;
        using namespace platWindows;
    }
}


#endif

namespace m
{
    namespace crossPlatform
    {
        using Window = platform::PlatformWindow;
        using IApplication = platform::PlatformApp;
    }
}

#endif //M_CROSSPLATFORM