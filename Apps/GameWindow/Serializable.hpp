#pragma once

#include <Kernel/Types.hpp>
#include <Kernel/Asserts.hpp>
#include <fstream>

#define m_read_flag 1
#define m_write_flag 2
#define m_textual_flag 4

inline void inc_spacing(m::mUInt& a_spacing)
{
    a_spacing++;
}

inline void dec_spacing(m::mUInt& a_spacing)
{
    mAssert(a_spacing > 0);
    a_spacing--;
}

inline void print_spacing(std::ostream& a_stream, m::mUInt const a_spacing)
{
    for (m::mUInt i = 0; i < a_spacing; ++i) { a_stream << " "; }
}

inline void begin_serialization(std::string const& a_name,
                                std::ostream&      a_stream,
                                m::mInt const      a_serializationFlags,
                                m::mU32& a_version, m::mUInt& a_spacingNumber)
{
    mAssert(a_serializationFlags & m_write_flag);
    if (a_serializationFlags & m_textual_flag)
    {
        print_spacing(a_stream, a_spacingNumber);
        a_stream << a_name << " " << a_version << std::endl;
        inc_spacing(a_spacingNumber);
    }
    else
    {
        mNotImplemented;
    }
}

inline void begin_serialization(std::string const& a_name,
                                std::istream&      a_stream,
                                m::mInt const      a_serializationFlags,
                                m::mU32& a_version, m::mUInt& a_spacingNumber)
{
    mAssert(a_serializationFlags & m_read_flag);
    if (a_serializationFlags & m_textual_flag)
    {
        std::string debugName;
        a_stream >> debugName >> a_version;
    }
    else
    {
        mNotImplemented;
    }
}

inline void end_serialization(m::mInt   a_serializationFlags,
                              m::mUInt& a_spacingNumber)
{
    if (a_serializationFlags & m_textual_flag)
    {
        if (a_serializationFlags & m_read_flag) {}
        else if (a_serializationFlags & m_write_flag)
        {
            dec_spacing(a_spacingNumber);
        }
    }
    else
    {
        mNotImplemented;
    }
}

template <typename t_Type>
void serialize_from(std::ostream& a_stream, m::mInt a_serializationFlags,
                    std::string const& a_valueName, t_Type& a_value,
                    m::mUInt a_specVersion, m::mUInt a_fileVersion,
                    m::mUInt& a_spacingNumber)
{
    mAssert(a_serializationFlags & m_write_flag);
    if (a_serializationFlags & m_textual_flag)
    {
        print_spacing(a_stream, a_spacingNumber);
        a_stream << a_valueName << " " << a_value << std::endl;
    }
    else
    {
        mNotImplemented;
    }
}

template <typename t_Type>
void serialize_from(std::istream& a_stream, m::mInt a_serializationFlags,
                    std::string const& a_valueName, t_Type& a_value,
                    m::mUInt a_specVersion, m::mUInt a_fileVersion,
                    m::mUInt& a_spacingNumber)
{
    mAssert(a_serializationFlags & m_read_flag);
    if (a_serializationFlags & m_textual_flag)
    {
        if (a_fileVersion >= a_specVersion)
        {
            std::string debugName;
            a_stream >> debugName >> a_value;
        }
    }
    else
    {
        mNotImplemented;
    }
}

#define mSerialize_from(a_version, a_variable)                              \
    serialize_from(a_stream, a_serializationFlags, #a_variable, a_variable, \
                   a_version, internalVersion, a_spacingNumber)

#define mSerialize_memberFrom(a_version, a_variable)                \
    serialize_from(a_stream, a_serializationFlags, #a_variable,     \
                   a_object.a_variable, a_version, internalVersion, \
                   a_spacingNumber)

#define mEnd_serialization(t_ClassName)                            \
    end_serialization(a_serializationFlags, a_spacingNumber);      \
    }                                                              \
    template <>                                                    \
    inline void serialize_from<t_ClassName>(                       \
        std::ostream & a_stream, m::mInt a_serializationFlags,     \
        std::string const& a_valueName, t_ClassName& a_value,      \
        m::mUInt a_specVersion, m::mUInt a_fileVersion,            \
        m::mUInt& a_spacingNumber)                                 \
    {                                                              \
        mAssert(a_serializationFlags& m_write_flag);               \
        if (a_serializationFlags & m_textual_flag)                 \
        {                                                          \
            serialize(a_value, a_stream, a_serializationFlags,     \
                      a_spacingNumber);                            \
        }                                                          \
        else                                                       \
        {                                                          \
            mNotImplemented;                                       \
        }                                                          \
    }                                                              \
    template <>                                                    \
    inline void serialize_from<t_ClassName>(                       \
        std::istream & a_stream, m::mInt a_serializationFlags,     \
        std::string const& a_valueName, t_ClassName& a_value,      \
        m::mUInt a_specVersion, m::mUInt a_fileVersion,            \
        m::mUInt& a_spacingNumber)                                 \
    {                                                              \
        mAssert(a_serializationFlags& m_read_flag);                \
        if (a_serializationFlags & m_textual_flag)                 \
        {                                                          \
            if (a_fileVersion >= a_specVersion)                    \
            {                                                      \
                serialize(a_value, a_stream, a_serializationFlags, \
                          a_spacingNumber);                        \
            }                                                      \
        }                                                          \
        else                                                       \
        {                                                          \
            mNotImplemented;                                       \
        }                                                          \
    }

#define mBegin_serialization(t_ClassName, a_versionNumber)                     \
    static const m::mU32 t_ClassName##_version = a_versionNumber;              \
    template <typename t_StreamType>                                           \
    void serialize(t_ClassName& a_object, t_StreamType& a_stream,              \
                   m::mInt a_serializationFlags, m::mUInt a_spacingNumber = 0) \
    {                                                                          \
        m::mUInt internalVersion = t_ClassName##_version;                      \
        begin_serialization(#t_ClassName, a_stream, a_serializationFlags,      \
                            internalVersion, a_spacingNumber);
