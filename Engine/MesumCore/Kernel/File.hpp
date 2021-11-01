#pragma once

#include <Log.hpp>
#include <MesumCore/Common.hpp>
#include <Types.hpp>
#include <filesystem>
#include <string>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
/// \addtogroup Guidelines
/// \{
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
/// \brief Namespace grouping file related utilities
///////////////////////////////////////////////////////////////////////////////
namespace m::files
{
extern MesumCoreApi const logging::ChannelID FILE_ID;

///////////////////////////////////////////////////////////////////////////////
/// \brief Open a file and copy it's content as a binary stream
///
/// \param a_filePath The path to the file to open
/// \param a_output The array to copy the data to
/// \post if a_filePath is a proper filepath, a_output previous content will be
/// lost
/// \return true if the file as been successfully open and copied
///////////////////////////////////////////////////////////////////////////////
Bool copy_fileToBinary(std::filesystem::path const& a_filePath,
                       std::vector<char>&           a_output);
}  // namespace m::files

///////////////////////////////////////////////////////////////////////////////
/// \}
///////////////////////////////////////////////////////////////////////////////