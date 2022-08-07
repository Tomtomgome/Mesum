#pragma once

#include <Kernel/Types.hpp>
#include <Kernel/Asserts.hpp>
#include <fstream>

template <typename t_Type>
constexpr bool has_customSerialization()
{
    return false;
}

template <typename t_Type>
concept mCustomSerializable = has_customSerialization<t_Type>();

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class mSerializerIfstream
{
   public:
    explicit mSerializerIfstream(std::ifstream& a_inputStream)
        : m_inputStream(a_inputStream)
    {
    }

    template <typename t_Type>
    void begin(t_Type& a_object, m::mUInt& a_version,
               std::string const& a_debugName);

    template <typename t_Type>
    void serialize(t_Type& a_object, std::string const& a_debugName);

    void end(){};

   private:
    std::ifstream& m_inputStream;
};

template <typename t_Type>
void mSerializerIfstream::serialize(t_Type&            a_object,
                                    std::string const& a_debugName)
{
    std::string debugName;
    m_inputStream >> debugName >> a_object;
}

template <typename t_Type>
void mSerializerIfstream::begin(t_Type& a_object, m::mUInt& a_version,
                                std::string const& a_debugName)
{
    std::string debugName;
    m_inputStream >> debugName >> a_version;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
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
    void serialize(t_Type& a_object, std::string const& a_debugName);

    void end();

   private:
    void print_spacing();

    m::mUInt       m_spacingNumber = 0;
    std::ofstream& m_outputStream;
};

template <typename t_Type>
void mSerializerOfstream::serialize(t_Type&            a_object,
                                    std::string const& a_debugName)
{
    print_spacing();
    m_outputStream << a_debugName << " " << a_object << std::endl;
}

template <typename t_Type>
void mSerializerOfstream::begin(t_Type& a_object, m::mUInt& a_version,
                                std::string const& a_debugName)
{
    print_spacing();
    m_outputStream << a_debugName << " " << a_version << std::endl;
    m_spacingNumber++;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
template <typename t_Serialize, typename t_Type>
void serialize_primitive(t_Serialize& a_serializer, t_Type& a_object,
                         std::string const& a_debugName)
{
    a_serializer.serialize(a_object, a_debugName);
}

template <typename t_Serialize, mCustomSerializable t_Type>
void serialize_primitive(t_Serialize& a_serializer, t_Type& a_object,
                         std::string const& a_debugName)
{
    mSerialize(a_object, a_serializer);
}

template <typename t_Serialize, typename t_Type>
void serialize_fromVersion(t_Serialize& a_serializer, t_Type& a_object,
                           m::mUInt a_version, m::mUInt a_objectVersion,
                           std::string const& a_debugName)
{
    if (a_objectVersion >= a_version)
    {
        serialize_primitive(a_serializer, a_object, a_debugName);
    }
}

#define mBegin_serialization(t_ClassName, a_versionNumber)                \
    template <>                                                           \
    constexpr bool has_customSerialization<t_ClassName>()                 \
    {                                                                     \
        return true;                                                      \
    }                                                                     \
    static const m::mU32 t_ClassName##_version = a_versionNumber;         \
    template <typename t_SerializerType>                                  \
    void mSerialize(t_ClassName& a_object, t_SerializerType& a_serializer) \
    {                                                                     \
        m::mUInt internalVersion = t_ClassName##_version;                 \
        a_serializer.begin(a_object, internalVersion, #t_ClassName);

#define mSerialize_from(a_version, a_variable)                 \
    serialize_fromVersion(a_serializer, a_variable, a_version, \
                          internalVersion, #a_variable)

#define mSerialize_memberFrom(a_version, a_variable)                    \
    serialize_fromVersion(a_serializer, a_object.a_variable, a_version, \
                          internalVersion, #a_variable)

#define mEnd_serialization(t_ClassName) \
    a_serializer.end();                 \
    }
