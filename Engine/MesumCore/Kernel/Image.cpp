#include <Asserts.hpp>
#include <Image.hpp>
#include <Profile.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

namespace m::resource
{
const logging::ChannelID LOG_ID_IMAGE = mLOG_GET_ID();

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
Image load_image(RequestImage const& a_request)
{
    Image    result;
    Int      texChannels;
    stbi_uc* pPixels =
        stbi_load(a_request.path.c_str(), &result.m_width, &result.m_height,
                  &texChannels, STBI_rgb_alpha);

    if (pPixels == nullptr)
    {
        mLOG_ERR_TO(LOG_ID_IMAGE, "Could not load image at ", a_request.path);
        return result;
    }

    mHardAssert(result.m_width * result.m_height > 0);

    result.m_data.clear();
    result.m_data.insert(result.m_data.begin(), &pPixels[0],
                         &pPixels[result.m_width * result.m_height * 4]);

    return result;
}
}  // namespace m::resource