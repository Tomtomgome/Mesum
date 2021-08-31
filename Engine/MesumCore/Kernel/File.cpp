#include <File.hpp>
#include <fstream>
#include <string>

namespace m::files
{
void open_fileToString(std::string const& a_fileName, std::string& a_output)
{
    std::ifstream ifs(a_fileName);
    a_output.assign((std::istreambuf_iterator<char>(ifs)),
                    (std::istreambuf_iterator<char>()));
}

void open_fileToBinary(std::string const& a_fileName,
                       std::vector<char>& a_output)
{
    std::ifstream file(a_fileName, std::ios::ate | std::ios::binary);

    if (!file.is_open())
    {
        throw std::runtime_error("failed to open file!");
    }

    size_t fileSize = (size_t)file.tellg();
    a_output.resize(fileSize);
    file.seekg(0);
    file.read(a_output.data(), fileSize);
    file.close();
}

}  // namespace m::files