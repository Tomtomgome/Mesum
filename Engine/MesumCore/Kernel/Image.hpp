#pragma once

#include "Kernel.hpp"
#include "Logger.hpp"
#include "Types.hpp"

#include <string>

///////////////////////////////////////////////////////////////////////////////
/// \addtogroup Core
/// \{
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
/// \brief Namespace grouping file resoures
///////////////////////////////////////////////////////////////////////////////
namespace m::resource
{
MesumCoreApi extern const logging::mChannelID g_logIDImage;

///////////////////////////////////////////////////////////////////////////////
/// \brief Defines a basic error code
///////////////////////////////////////////////////////////////////////////////
static const mErrorCode ecCouldNotLoad = 1;

///////////////////////////////////////////////////////////////////////////////
/// \brief Structure used to request the loading of an image on disk
///
///////////////////////////////////////////////////////////////////////////////
struct mRequestImage
{
    std::string path;  //!< The path of the image to load
};

///////////////////////////////////////////////////////////////////////////////
/// \brief Class representing the data of an image
///////////////////////////////////////////////////////////////////////////////
struct mImage
{
    mInt width{0};   //!< The width of the image
    mInt height{0};  //!< The height of the image

    ///////////////////////////////////////////////////////////////////////////
    /// \brief The array of values for each channel of each pixel
    ///////////////////////////////////////////////////////////////////////////
    std::vector<mU8> data{};
};

///////////////////////////////////////////////////////////////////////////////
/// \brief Class representing the data of an image of templated pixels
///
/// \tparam t_PixelType The type encapsulating one pixel of the image
///////////////////////////////////////////////////////////////////////////////
template<typename t_PixelType>
struct mTypedImage
{
    mInt width{0};   //!< The width of the image
    mInt height{0};  //!< The height of the image

    ///////////////////////////////////////////////////////////////////////////
    /// \brief The array of values for each pixel
    ///////////////////////////////////////////////////////////////////////////
    std::vector<t_PixelType> data{};
};

///////////////////////////////////////////////////////////////////////////////
/// \brief Loads an image from the disk through a request
///
/// \param a_request The request to load the image
/// \pre The path of the request must not be empty
/// \return The data of the loaded image
///////////////////////////////////////////////////////////////////////////////
mOutput<mImage> load_image(mRequestImage const& a_request);

};  // namespace m::resource
