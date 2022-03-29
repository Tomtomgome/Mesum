#pragma once

#include <Kernel/Types.hpp>
#include <fstream>

#define Serializable(a_versionNumber, t_ClassName)           \
    static const m::mU32 s_version = a_versionNumber;        \
    void                 read(std::ifstream& a_inputStream); \
    void                 write(std::ofstream& a_outputStream) const;