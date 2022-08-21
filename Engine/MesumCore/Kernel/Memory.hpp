#pragma once

#include "../Common/CoreCommon.hpp"
#include "Types.hpp"

#include <string>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
/// \addtogroup Core
/// \{
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
/// \brief Namespace grouping memory utilities
///////////////////////////////////////////////////////////////////////////////
namespace m::memory
{
///////////////////////////////////////////////////////////////////////////////
/// \brief Concept defining objects that can be constructed with a given list
/// of parameters
///
/// \tparam t_Type The type of object to construct
/// \tparam t_Arg The list of arguments used for construction
///////////////////////////////////////////////////////////////////////////////
template <typename t_Type, typename... t_Arg>
concept mConstructible = requires(t_Arg... a_args)
{
    new t_Type(a_args...);
};

///////////////////////////////////////////////////////////////////////////////
/// \brief Type of memory used for debug information
/// memory types ARE NOT THREAD SAFE
///////////////////////////////////////////////////////////////////////////////
using mMemoryType                            = mUInt;
static const mMemoryType g_defaultMemoryType = 0;

///////////////////////////////////////////////////////////////////////////////
/// \brief Stat for a type of memory
///////////////////////////////////////////////////////////////////////////////
struct mMemoryStat
{
    std::string name{};                      //!< Name of the memory type
    mBool       isVerbose          = false;  //!< Does this type log allocation
    mSize       totalAllocatedSize = 0;      //!< Size allocated for this type
};

///////////////////////////////////////////////////////////////////////////////
/// \brief Groups all stats for memory
///////////////////////////////////////////////////////////////////////////////
struct mMemoryStats
{
    mSize globalAllocationSizes;                //!< Size of all allocations
    std::vector<mMemoryStat> typedMemoryStats;  //!< Stats for all memory types
};

extern mMemoryStats g_memStats;

///////////////////////////////////////////////////////////////////////////////
/// \brief Initialize memory tracking
///
/// Called before application launch
///////////////////////////////////////////////////////////////////////////////
void initialize_memoryTracking();

///////////////////////////////////////////////////////////////////////////////
/// \brief Terminate memory tracking, check for memory leaks.
///
/// Called after application termination
///////////////////////////////////////////////////////////////////////////////
void terminate_memoryTracking();

///////////////////////////////////////////////////////////////////////////////
/// \brief Create a new memory type
///
/// \param a_name The name of the new memory type
/// \return The memory type newly created
///////////////////////////////////////////////////////////////////////////////
mMemoryType create_newMemoryType(std::string const& a_name);
///////////////////////////////////////////////////////////////////////////////
/// \brief Record stats for a allocation of a certain memory type
///
/// \param a_type The type of memory we are doing the allocation in
/// \param a_size The size of the allocation
///////////////////////////////////////////////////////////////////////////////
void log_memoryAllocation(mMemoryType a_type, mSize a_size);
///////////////////////////////////////////////////////////////////////////////
/// \brief Record stats for a deallocation of a certain memory type
///
/// \param a_type The type of memory we are doing the deallocation in
/// \param a_size The size of the deallocation
///////////////////////////////////////////////////////////////////////////////
void log_memoryDeallocation(mMemoryType a_type, mSize a_size);

///////////////////////////////////////////////////////////////////////////////
/// \brief Internal implementation of a basic allocation
///
/// \param a_size The size of the allocation
/// \return a pointer to the newly allocated memory
///////////////////////////////////////////////////////////////////////////////
mPtr allocate(m::mSize a_size);
///////////////////////////////////////////////////////////////////////////////
/// \brief Internal implementation of a basic deallocation
///
/// \param a_object The pointer to the object to deallocate
///////////////////////////////////////////////////////////////////////////////
void deallocate(mPtr& a_object);

///////////////////////////////////////////////////////////////////////////////
/// \brief Basic allocator allowing to log memory debug stats
///
/// Prefer using these to construct objects instead of new and delete directly
///////////////////////////////////////////////////////////////////////////////
class mObjectAllocator
{
   public:
    ///////////////////////////////////////////////////////////////////////////
    /// \brief Initialize the allocator
    ///
    /// \param a_memoryType The type of memory where to perform the allocations
    /// \pre the memory type must have been created using create_newMemoryType()
    ///////////////////////////////////////////////////////////////////////////
    void init(mMemoryType const& a_memoryType) { m_memoryType = a_memoryType; }

    ///////////////////////////////////////////////////////////////////////////
    /// \brief Construct a new object
    ///
    /// \tparam t_Type The type of object to construct
    /// \tparam t_Arg The list of type of arguments to pass the constructor
    /// \param a_args The list of parameters to pass the constructor
    /// \return A pointer to the newly created object
    /// \post allocated object must be deallocated using the same allocator
    ///////////////////////////////////////////////////////////////////////////
    template <typename t_Type, typename... t_Arg>
    requires memory::mConstructible<t_Type, t_Arg...> t_Type* construct(
        t_Arg... a_args)
    {
#ifdef M_TRACK_MEMORY_ALLOCATIONS
        memory::log_memoryAllocation(m_memoryType, sizeof(t_Type));
#endif  // M_TRACK_MEMORY_ALLOCATIONS
        return new t_Type(a_args...);
    }

    ///////////////////////////////////////////////////////////////////////////
    /// \brief Construct a new object but return a reference
    ///
    /// \tparam t_Type The type of object to construct
    /// \tparam t_Arg The list of type of arguments to pass the constructor
    /// \param a_args The list of parameters to pass the constructor
    /// \return A reference to the newly created object
    /// \post allocated object must be deallocated using the same allocator
    ///////////////////////////////////////////////////////////////////////////
    template <typename t_Type, typename... t_Arg>
    t_Type& construct_ref(
        t_Arg... a_args)
    {
        return *(construct<t_Type, t_Arg...>(a_args...));
    }

    ///////////////////////////////////////////////////////////////////////////
    /// \brief Deconstruct an object
    ///
    /// \tparam t_Type The type of object to deconstruct
    /// \param a_object The pointer of the object to delete
    /// \pre a_object must have been allocated using the same allocator
    ///////////////////////////////////////////////////////////////////////////
    template <typename t_Type>
    void destroy(t_Type* a_object)
    {
#ifdef M_TRACK_MEMORY_ALLOCATIONS
        memory::log_memoryDeallocation(m_memoryType, sizeof(t_Type));
#endif  // M_TRACK_MEMORY_ALLOCATIONS
        delete a_object;
    }

    ///////////////////////////////////////////////////////////////////////////
    /// \brief Deconstruct an object from a reference
    ///
    /// \tparam t_Type The type of object to deconstruct
    /// \param a_object The pointer of the object to delete
    /// \pre a_object must have been allocated using the same allocator
    ///////////////////////////////////////////////////////////////////////////
    template <typename t_Type>
    void destroy_ref(t_Type& a_object)
    {
        destroy(&a_object);
    }

   private:
    ///////////////////////////////////////////////////////////////////////////////
    /// \brief The type of memory used by the allocator
    ///////////////////////////////////////////////////////////////////////////////
    mMemoryType m_memoryType = g_defaultMemoryType;
};

};  // namespace m::memory