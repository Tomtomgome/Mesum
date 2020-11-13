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

#elif defined M_WINDOWS
 
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

#endif //M_CROSSPLATFORM