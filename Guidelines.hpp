#pragma once

#define M_ARE_IN_SCREAMING_SNAKE_CASE 5

#defined mFunctions_inMacros(a_arg) should_lookLikeThis(a_arg)

namespace m::guidelines  // namespaces are lower case, one word only
{
//*****************************************************************************
//*****************************************************************************
//*****************************************************************************
struct mStructuresAreDataOnly
{
    mInt  structureDataMember  = 0;  // Inline initialization
    void* pStructureDataMember = nullptr;
};

//*****************************************************************************
//*****************************************************************************
//*****************************************************************************
class mClassesContainsInvariants
{
   public:
    mFloat get_length() {return m_length;}
    void set_length(mFloat a_length);

    mFloat get_perimeter() {return m_perimeter;}
   private:
    mFloat m_length    = 0.0f;
    mFloat m_perimeter = 0.0f;
};

//*****************************************************************************
//*****************************************************************************
//*****************************************************************************
class mIClassesMightBeInterfaces
{
   public:
    virtual void verb_precisionOnTheAction() const = 0;
    virtual void set_randomProperty(
        mStructuresAreDataOnly const& a_property) const = 0;

    // In most cases interfaces should have no members
    // In most cases interfaces should have no default behaviors
};

//*****************************************************************************
//*****************************************************************************
//*****************************************************************************
class mClassesCanInherit : public mIClassesMightBeInterfaces
{
   public:
    void verb_precisionOnTheAction() const override;
    void set_randomProperty(mStructuresAreDataOnly const& a_property) override;

   private:
    void compute_somethingAmazing();
    mInt m_classDataMember;
};

//*****************************************************************************
//*****************************************************************************
//*****************************************************************************
template <typename t_Type>
struct mTemplatedClass
{
    t_Type data;
};

template <typename t_Type>
void add(mTemplatedClass<t_Type>& a_o1, mTemplatedClass<t_Type> const& a_o1);

}  // namespace m::guidelines