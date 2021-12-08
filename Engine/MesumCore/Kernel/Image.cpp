#include "Asserts.hpp"
#include "Image.hpp"
#include "Profile.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

namespace m::resource
{
const logging::mChannelID g_logIDImage = mLog_getId();
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
mOutput<mImage> load_image(mRequestImage const& a_request)
{
    mExpect(!a_request.path.empty());

    mImage   result;
    mInt     texChannels;
    stbi_uc* pPixels = stbi_load(a_request.path.c_str(), &result.width,
                                 &result.height, &texChannels, STBI_rgb_alpha);

    if (pPixels == nullptr)
    {
        mLog_errorTo(g_logIDImage, "Could not load image at ", a_request.path);
        return {ecCouldNotLoad, std::move(result)};
    }

    mAssert(result.width * result.height > 0);

    result.data.clear();
    result.data.insert(result.data.begin(), &pPixels[0],
                       &pPixels[result.width * result.height * 4]);
    return {ecSuccess, std::move(result)};
}
}  // namespace m::resource