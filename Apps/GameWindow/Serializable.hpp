#pragma once

#include <Kernel/Types.hpp>
#include <Kernel/Asserts.hpp>
#include <fstream>

#define m_read_flag 1
#define m_write_flag 2
#define m_textual_flag 4

template <typename t_Type>
constexpr bool has_customSerialization()
{
    return false;
}

template <typename t_Type>
concept customSerializable = has_customSerialization<t_Type>();

class mSerializerIfstream
{
   public:
    mSerializerIfstream(std::ifstream& a_inputStream)
        : m_inputStream(a_inputStream)
    {
    }

    template <typename t_Type>
    void begin(t_Type& a_object, m::mUInt& a_version,
               std::string const& a_debugName);

    template <typename t_Type>
    void serialize_primitive(t_Type& a_object, std::string const& a_debugName);
    template <customSerializable t_Type>
    void serialize_primitive(t_Type& a_object, std::string const& a_debugName);

    template <typename t_Type>
    void serialize_fromVersion(t_Type& a_object, m::mUInt a_version,
                               m::mUInt           a_objectVersion,
                               std::string const& a_debugName);

    void end(){};

   private:
    std::ifstream& m_inputStream;
};

template <typename t_Type>
void mSerializerIfstream::serialize_primitive(t_Type&            a_object,
                                              std::string const& a_debugName)
{
    std::string debugName;
    m_inputStream >> debugName >> a_object;
}

template <customSerializable t_Type>
void mSerializerIfstream::serialize_primitive(t_Type&            a_object,
                                              std::string const& a_debugName)
{
    serialize(a_object, *this);
}

template <typename t_Type>
void mSerializerIfstream::begin(t_Type& a_object, m::mUInt& a_version,
                                std::string const& a_debugName)
{
    std::string debugName;
    m_inputStream >> debugName >> a_version;
}

template <typename t_Type>
void mSerializerIfstream::serialize_fromVersion(t_Type&  a_object,
                                                m::mUInt a_version,
                                                m::mUInt a_objectVersion,
                                                std::string const& a_debugName)
{
    if (a_objectVersion >= a_version)
    {
        serialize_primitive(a_object, a_debugName);
    }
}

class mSerializerOfstream
{
   public:
    explicit mSerializerOfstream(std::ofstream& a_outputStream)
        : m_outputStream(a_outputStream)
    {
    }

    template <typename t_Type>
    void begin(t_Type& a_object, m::mUInt& a_version,
               std::string const& a_debugName);

    template <typename t_Type>
    void serialize_primitive(t_Type& a_object, std::string const& a_debugName);
    template <customSerializable t_Type>
    void serialize_primitive(t_Type& a_object, std::string const& a_debugName);

    template <typename t_Type>
    void serialize_fromVersion(t_Type& a_object, m::mUInt a_version,
                               m::mUInt           a_objectVersion,
                               std::string const& a_debugName);

    void end();

   private:
    void print_spacing();

    m::mUInt       m_spacingNumber = 0;
    std::ofstream& m_outputStream;
};

template <typename t_Type>
void mSerializerOfstream::serialize_primitive(t_Type&            a_object,
                                              std::string const& a_debugName)
{
    print_spacing();
    m_outputStream << a_debugName << " " << a_object << std::endl;
}

template <customSerializable t_Type>
void mSerializerOfstream::serialize_primitive(t_Type&            a_object,
                                              std::string const& a_debugName)
{
    serialize(a_object, *this);
}

template <typename t_Type>
void mSerializerOfstream::begin(t_Type& a_object, m::mUInt& a_version,
                                std::string const& a_debugName)
{
    print_spacing();
    m_outputStream << a_debugName << " " << a_version << std::endl;
    m_spacingNumber++;
}

template <typename t_Type>
void mSerializerOfstream::serialize_fromVersion(t_Type&  a_object,
                                                m::mUInt a_version,
                                                m::mUInt a_objectVersion,
                                                std::string const& a_debugName)
{
    serialize_primitive(a_object, a_debugName);
}

#define mSerialize_from(a_version, a_variable)                                 \
    a_serializer.serialize_fromVersion(a_variable, a_version, internalVersion, \
                                       #a_variable)

#define mSerialize_memberFrom(a_version, a_variable)                   \
    a_serializer.serialize_fromVersion(a_object.a_variable, a_version, \
                                       internalVersion, #a_variable)

#define mEnd_serialization(t_ClassName) \
    a_serializer.end();                 \
    }

#define mBegin_serialization(t_ClassName, a_versionNumber)                \
    template <>                                                           \
    constexpr bool has_customSerialization<t_ClassName>()                 \
    {                                                                     \
        return true;                                                      \
    }                                                                     \
    static const m::mU32 t_ClassName##_version = a_versionNumber;         \
    template <typename t_SerializerType>                                  \
    void serialize(t_ClassName& a_object, t_SerializerType& a_serializer) \
    {                                                                     \
        m::mUInt internalVersion = t_ClassName##_version;                 \
        a_serializer.begin(a_object, internalVersion, #t_ClassName);
