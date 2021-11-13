#pragma once

// Documentation is this doxygen format
///////////////////////////////////////////////////////////////////////////////
/// \addtogroup Guidelines
/// \{
///////////////////////////////////////////////////////////////////////////////

//! This is a macro !
#define M_ARE_IN_SCREAMING_SNAKE_CASE 5

///////////////////////////////////////////////////////////////////////////////
/// \brief This is a preprocessor function
///
/// \param a_arg The macro argument
///////////////////////////////////////////////////////////////////////////////
#defined mFunctions_inMacros(a_arg) should_lookLikeThis(a_arg)

///////////////////////////////////////////////////////////////////////////////
/// \brief globals are prefixed with g_
///////////////////////////////////////////////////////////////////////////////
const mU64 g_globalConstant;

///////////////////////////////////////////////////////////////////////////////
/// \brief Namespace grouping the guidelines
///////////////////////////////////////////////////////////////////////////////
namespace m::guidelines  // namespaces are lower case, one word only
{
///////////////////////////////////////////////////////////////////////////////
/// \brief Enumerations values
///////////////////////////////////////////////////////////////////////////////
enum mEnumerations
{
    areLikeThis = 1,  //!< are documented
    orThis,           //!< inline if
    andThis           //!< possible
};

///////////////////////////////////////////////////////////////////////////////
/// \brief Struct are documented as follow with a brief description
///
/// And more details here if necessary
///////////////////////////////////////////////////////////////////////////////
struct mStructuresAreDataOnly
{
    ///////////////////////////////////////////////////////////////////////////
    /// \brief Members can be documented like this
    ///////////////////////////////////////////////////////////////////////////
    mInt  structureDataMember  = 0;        // Inline initialization
    void* pStructureDataMember = nullptr;  //!< Or like this if there is room
};

///////////////////////////////////////////////////////////////////////////////
/// \brief Class should have an invariant
///
/// Meaning that if you can't change a member without affecting an other that
/// mean you should put it as a class
///////////////////////////////////////////////////////////////////////////////
class mClassesContainsInvariants
{
   public:
    ///////////////////////////////////////////////////////////////////////////
    /// \brief You can only set the length value
    ///
    /// \param a_length The new length
    /// \pre a_length can't be negative
    ///////////////////////////////////////////////////////////////////////////
    void set_length(mFloat a_length);

    ///////////////////////////////////////////////////////////////////////////
    /// \brief This is a function
    ///
    /// \return and it returns the length of the object
    ///////////////////////////////////////////////////////////////////////////
    mFloat get_length() const { return m_length; }

    ///////////////////////////////////////////////////////////////////////////
    /// \brief This is a function
    ///
    /// \return And it returns the perimeter of the object
    ///////////////////////////////////////////////////////////////////////////
    mFloat get_perimeter() const { return m_perimeter; }

   private:
    mFloat m_length    = 0.0f;  //!< Length affect
    mFloat m_perimeter = 0.0f;  //!< The perimeter
};

///////////////////////////////////////////////////////////////////////////////
/// \brief Interfaces should class
///
/// And in most cases they should not have member nor default behaviors
///////////////////////////////////////////////////////////////////////////////
class mIClassesMightBeInterfaces
{
   public:
    ///////////////////////////////////////////////////////////////////////////
    /// \brief This is a virtual pure function
    ///////////////////////////////////////////////////////////////////////////
    virtual void verb_precisionOnTheAction() const = 0;

    ///////////////////////////////////////////////////////////////////////////
    /// \brief This is an other virtual pure function
    ///
    /// \param a_property The property to set
    ///////////////////////////////////////////////////////////////////////////
    virtual void set_randomProperty(
        mStructuresAreDataOnly const& a_property) const = 0;

    // In most cases interfaces should have no members
    // In most cases interfaces should have no default behaviors
};

///////////////////////////////////////////////////////////////////////////////
/// \brief Inheritance should be used carefully
///
/// Mainly to implement an interface
///////////////////////////////////////////////////////////////////////////////
class mClassesCanInherit : public mIClassesMightBeInterfaces
{
   public:
    ///////////////////////////////////////////////////////////////////////////
    /// \brief This is an overridden function
    ///////////////////////////////////////////////////////////////////////////
    void verb_precisionOnTheAction() const override;

    ///////////////////////////////////////////////////////////////////////////
    /// \brief This is an other overridden function
    ///
    /// \param a_property The property to set
    ///////////////////////////////////////////////////////////////////////////
    void set_randomProperty(mStructuresAreDataOnly const& a_property) override;

   private:
    ///////////////////////////////////////////////////////////////////////////
    /// \brief This is a private function
    ///////////////////////////////////////////////////////////////////////////
    void compute_somethingAmazing();

   private:
    mInt m_classDataMember;  //!< This is some data members
};

///////////////////////////////////////////////////////////////////////////////
/// \brief This is a templated class
///
/// \tparam t_Type The template parameter
///////////////////////////////////////////////////////////////////////////////
template <typename t_Type>
struct mTemplatedClass
{
    t_Type data;  //!< Some data
};

///////////////////////////////////////////////////////////////////////////////
/// \brief This is a templated function
///
/// \tparam t_Type The template parameter
/// \param a_o1 The first element to add
/// \param a_o2 The second element to add
///////////////////////////////////////////////////////////////////////////////
template <typename t_Type>
void add(mTemplatedClass<t_Type>& a_o1, mTemplatedClass<t_Type> const& a_o2);

}  // namespace m::guidelines

///////////////////////////////////////////////////////////////////////////////
/// \}
///////////////////////////////////////////////////////////////////////////////