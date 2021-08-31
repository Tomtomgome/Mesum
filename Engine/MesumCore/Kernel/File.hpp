#ifndef M_File
#define M_File
#pragma once

#include <string>
#include <vector>

namespace m::files
{
void open_fileToString(std::string const& a_fileName, std::string& a_output);
void open_fileToBinary(std::string const& a_fileName,
                       std::vector<char>& a_output);
} //m::files

#endif  // M_File