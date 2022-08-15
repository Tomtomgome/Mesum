#include "Serializable.hpp"

void mSerializerOfstream::print_spacing()
{
    for (m::mUInt i = 0; i < m_spacingNumber; ++i) { m_outputStream << " "; }
}

void mSerializerOfstream::end()
{
    m_spacingNumber--;
}
