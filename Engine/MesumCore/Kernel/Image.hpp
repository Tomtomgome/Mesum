#ifndef M_Image
#define M_Image
#pragma once

#include <Logger.hpp>
#include <Types.hpp>
#include <string>

namespace m::resource
{
MesumCoreApi extern const logging::ChannelID LOG_ID_IMAGE;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
struct RequestImage
{
    std::string path;
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
class Image
{
   public:
    Int                    width() const { return m_width; }
    Int                    height() const { return m_height; }
    std::vector<U8> const& data() const { return m_data; }

   private:
    Image() = default;  // Can only get image through copy or helpers function

    Int m_width{0};
    Int m_height{0};

    std::vector<U8> m_data{};

    friend Image load_image(RequestImage const& a_request);
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
Image load_image(RequestImage const& a_request);

};  // namespace m::resource

#endif  // M_Image