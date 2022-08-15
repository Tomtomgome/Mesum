#pragma once

#include "Types.hpp"
#include "Asserts.hpp"

#include <fstream>

///////////////////////////////////////////////////////////////////////////////
/// \addtogroup Core
/// \{
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
/// \addtogroup Serialization
/// \{
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
/// \brief Concept defining objects with a defined serialize function
///
/// \tparam t_Serializer The type of serializer
/// \tparam t_Type The type for which the custom serialization is tested
///////////////////////////////////////////////////////////////////////////////
template <typename t_Serializer, typename t_Type>
concept mCustomSerializable = requires(t_Serializer serializer, t_Type object)
{
    mSerialize(object, serializer);
};

///////////////////////////////////////////////////////////////////////////////
/// \brief Serialize an object using a serializer directly
///
/// \tparam t_Serializer The type of serializer
/// \tparam t_Type The type of object to serialize
/// \param a_serializer The serializer to use for the serialization
/// \param a_object The object to serialize
/// \param a_debugName A debug name for the object, not always used
///////////////////////////////////////////////////////////////////////////////
template <typename t_Serializer, typename t_Type>
void serialize_primitive(t_Serializer& a_serializer, t_Type& a_object,
                         std::string const& a_debugName)
{
    a_serializer.serialize(a_object, a_debugName);
}

///////////////////////////////////////////////////////////////////////////////
/// \brief Serialize an object using its custom serialization
///
/// \tparam t_Serializer The type of serializer
/// \tparam t_Type The type of object to serialize
/// \param a_serializer The serializer to use for the serialization
/// \param a_object The object to serialize
/// \param a_debugName A debug name for the object, not always used
///////////////////////////////////////////////////////////////////////////////
template <typename t_Serializer, typename t_Type>
requires mCustomSerializable<t_Serializer, t_Type>
void serialize_primitive(t_Serializer& a_serializer, t_Type& a_object,
                         std::string const& a_debugName)
{
    mSerialize(a_object, a_serializer);
}

///////////////////////////////////////////////////////////////////////////////
/// \brief Serialize an object only if a_objectVersion > a_version
///
/// \tparam t_Serializer The type of serializer
/// \tparam t_Type The type of object to serialize
/// \param a_serializer The serializer to use for the serialization
/// \param a_object The object to serialize
/// \param a_version The specified version at which to do the serialization
/// \param a_objectVersion The version read or writen from the serialization
/// \param a_debugName A debug name for the object, not always used
///////////////////////////////////////////////////////////////////////////////
template <typename t_Serializer, typename t_Type>
void serialize_fromVersion(t_Serializer& a_serializer, t_Type& a_object,
                           m::mUInt a_version, m::mUInt a_objectVersion,
                           std::string const& a_debugName)
{
    if (a_objectVersion >= a_version)
    {
        serialize_primitive(a_serializer, a_object, a_debugName);
    }
}

///////////////////////////////////////////////////////////////////////////////
/// \brief Put in a class, allows private members to be serialized
///
/// \param t_ClassName The name of the class in which the macro is called
///////////////////////////////////////////////////////////////////////////////
#define mAllow_privateSerialization(t_ClassName)       \
    template <typename t_SerializerType>               \
    friend void mSerialize(t_ClassName&      a_object, \
                           t_SerializerType& a_serializer);

///////////////////////////////////////////////////////////////////////////////
/// \brief Put outside a struct, Start the definition of a custom serialization
///
/// \code{.cpp}
/// struct mExample
/// {
///     mInt variable;
///     std::String nameAddedLaterOn
/// };
/// mBegin_serialization(mExample, 2);
/// mSerialize_memberFrom(1, variable);
/// mSerialize_memberFrom(2, nameAddedLaterOn);
/// mEnd_serialization(mExample);
/// \endcode
///
/// \param t_ClassName The name of the struct for which to define the custom
/// serialization
/// \param a_versionNumber The version of the serialization defined by this
/// function. This number is expected to be incremented each time the content
/// of the function is modified (in order to keep compatibility)
///////////////////////////////////////////////////////////////////////////////
#define mBegin_serialization(t_ClassName, a_versionNumber)                 \
    static const m::mU32 t_ClassName##_version = a_versionNumber;          \
    template <typename t_SerializerType>                                   \
    void mSerialize(t_ClassName& a_object, t_SerializerType& a_serializer) \
    {                                                                      \
        m::mUInt internalVersion = t_ClassName##_version;                  \
        a_serializer.begin(a_object, internalVersion, #t_ClassName);

///////////////////////////////////////////////////////////////////////////////
/// \brief Serialize a variable from a given version
///
/// Put between mBegin_serialization and mEnd_serialization
///
/// \param a_version The version at which the serialization started
/// \param a_variable The name of the variable to serialize
///////////////////////////////////////////////////////////////////////////////
#define mSerialize_from(a_version, a_variable)                 \
    serialize_fromVersion(a_serializer, a_variable, a_version, \
                          internalVersion, #a_variable)

///////////////////////////////////////////////////////////////////////////////
/// \brief Serialize a member of the struct from a given version
///
/// Put between mBegin_serialization and mEnd_serialization
///
/// \param a_version The version at which the serialization started
/// \param a_variable The name of the member to serialize
///////////////////////////////////////////////////////////////////////////////
#define mSerialize_memberFrom(a_version, a_variable)                    \
    serialize_fromVersion(a_serializer, a_object.a_variable, a_version, \
                          internalVersion, #a_variable)

///////////////////////////////////////////////////////////////////////////////
/// \brief End the definition of a custom serialization
///
/// \param t_ClassName The name of the struct for which the custom
/// serialization was defined
///////////////////////////////////////////////////////////////////////////////
#define mEnd_serialization(t_ClassName) \
    a_serializer.end();                 \
    }

///////////////////////////////////////////////////////////////////////////////
/// \brief Groups default serializer classes
///////////////////////////////////////////////////////////////////////////////
namespace m::serializer
{
///////////////////////////////////////////////////////////////////////////////
/// \brief Default serializer to read an object from a text file
///////////////////////////////////////////////////////////////////////////////
class mSerializerIfstream
{
   public:
    ///////////////////////////////////////////////////////////////////////////
    /// \brief Construct an input text file serializer
    ///
    /// \param a_inputStream the stream from which the serializer is going to
    /// read
    ///////////////////////////////////////////////////////////////////////////
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
/// \brief Default serializer to write an object on a text file
///////////////////////////////////////////////////////////////////////////////
class mSerializerOfstream
{
   public:
    ///////////////////////////////////////////////////////////////////////////
    /// \brief Construct an output text file serializer
    ///
    /// \param a_outputStream The stream on which the serializer is going to
    /// write
    ///////////////////////////////////////////////////////////////////////////
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

};  // namespace m::serializer

///////////////////////////////////////////////////////////////////////////////
/// \}
/// \}
///////////////////////////////////////////////////////////////////////////////