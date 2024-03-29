#include <File.hpp>
#include <Logger.hpp>
#include <fstream>

namespace m::files
{
MesumCoreApi const logging::mChannelID g_fileLogID = mLog_getId();

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
mBool copy_fileToBinary(std::filesystem::path const& a_filePath,
                        std::vector<char>&           a_output)
{
    if (!std::filesystem::exists(a_filePath))
    {
        mLog_errorTo(g_fileLogID, "File ", a_filePath, " does not exist");
        return false;
    }

    if (!std::filesystem::is_regular_file(a_filePath))
    {
        mLog_errorTo(g_fileLogID, "File ", a_filePath,
                     " is not a regular file");
        return false;
    }

    std::ifstream file(a_filePath, std::ios::ate | std::ios::binary);

    if (!file.is_open())
    {
        mLog_errorTo(g_fileLogID, "Could not open ", a_filePath);
        return false;
    }

    auto fileSize = std::filesystem::file_size(a_filePath);
    a_output.resize(fileSize);
    file.seekg(0);
    file.read(a_output.data(), fileSize);
    file.close();
    return true;
}

}  // namespace m::files