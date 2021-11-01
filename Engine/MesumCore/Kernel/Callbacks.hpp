#pragma once

#include <Asserts.hpp>
#include <Types.hpp>
#include <functional>
#include <list>

///////////////////////////////////////////////////////////////////////////////
/// \addtogroup Core
/// \{
///////////////////////////////////////////////////////////////////////////////

namespace m
{
///////////////////////////////////////////////////////////////////////////////
/// \brief Class encapsulating a callback
///
/// It can hold any kind of function (members, lambdas, etc...).
/// Warning, it is not a performant implementation, and there is a
/// std::function under the hood
///
/// \tparam t_RetType The return type of the callback
/// \tparam t_Args The list of arguments of the function
///////////////////////////////////////////////////////////////////////////////
template <typename t_RetType, typename... t_Args>
class mCallback
{
   public:
    ///////////////////////////////////////////////////////////////////////////
    /// \brief Alias the type of the function in the callback
    ///////////////////////////////////////////////////////////////////////////
    using mFunctionType = t_RetType(t_Args...);

    ///////////////////////////////////////////////////////////////////////////
    /// \brief Default constructor
    ///////////////////////////////////////////////////////////////////////////
    mCallback() = default;

    ///////////////////////////////////////////////////////////////////////////
    /// \brief Constructor for member functions
    ///
    /// \tparam t_CArgs The type of the class from witch we get the function
    /// \param a_owner A pointer to the object to which the callback call the
    ///     member function
    /// \pre a_owner can't be nullptr
    /// \param a_func The member function of the class t_CArg for the callback
    /// \pre a_func can't be nullptr
    ///////////////////////////////////////////////////////////////////////////
    template <typename t_CArgs>
    mCallback(t_CArgs* a_owner, t_RetType (t_CArgs::*a_func)(t_Args...))
    {
        mExpect(a_owner != nullptr);
        mExpect(a_func != nullptr);
        m_func = [a_owner, a_func](t_Args... a_args) -> t_RetType
        { return (a_owner->*a_func)(a_args...); };
    }

    ///////////////////////////////////////////////////////////////////////////
    /// \brief Constructor from usual std::function
    ///
    /// Implicit conversion makes it work with any king of usual function
    /// pointers
    ///
    /// \param a_func The function for the callback
    ///////////////////////////////////////////////////////////////////////////
    explicit mCallback(std::function<mFunctionType> const& a_func)
    {
        m_func = a_func;
    }

    ///////////////////////////////////////////////////////////////////////////
    /// \brief Move constructor from usual std::function
    ///
    /// Implicit conversion makes it work with any king of usual function
    /// pointers
    ///
    /// \param a_func The function for the callback
    ///////////////////////////////////////////////////////////////////////////
    explicit mCallback(std::function<mFunctionType>&& a_func)
    {
        m_func = std::move(a_func);
    }

    ///////////////////////////////////////////////////////////////////////////
    /// \brief Explicit setter for member functions
    ///
    /// \tparam t_CArgs The type of the class from witch we get the function
    /// \param a_owner A pointer to the object to which the callback call the
    ///     member function
    /// \pre a_owner can't be nullptr
    /// \param a_func The member function of the class t_CArg for the callback
    /// \pre a_func can't be nullptr
    ///////////////////////////////////////////////////////////////////////////
    template <typename t_CArgs>
    void set(t_CArgs* a_owner, t_RetType (t_CArgs::*a_func)(t_Args...))
    {
        mExpect(a_owner != nullptr);
        mExpect(a_func != nullptr);
        m_func = [a_owner, a_func](t_Args... a_args) -> t_RetType
        { return (a_owner->*a_func)(a_args...); };
    }

    ///////////////////////////////////////////////////////////////////////////
    /// \brief explicit setter for usual std::function
    ///
    /// Implicit conversion makes it work with any king of usual function
    /// pointers
    ///
    /// \param a_func The function for the callback
    ///////////////////////////////////////////////////////////////////////////
    void set(std::function<mFunctionType> const& a_func) { m_func = a_func; }

    ///////////////////////////////////////////////////////////////////////////
    /// \brief explicit setter for usual std::function
    ///
    /// Implicit conversion makes it work with any king of usual function
    /// pointers
    ///
    /// \param a_func The function for the callback
    ///////////////////////////////////////////////////////////////////////////
    void set(std::function<mFunctionType>&& a_func)
    {
        m_func = std::move(a_func);
    }

    ///////////////////////////////////////////////////////////////////////////
    /// \brief Call the underlying function
    ///
    /// \param a_args The list of arguments for the call
    /// \return the result of the function
    ///////////////////////////////////////////////////////////////////////////
    t_RetType call(t_Args... a_args)
    {
        mAssert(mBool(m_func));
        return m_func(a_args...);
    }

    ///////////////////////////////////////////////////////////////////////////
    /// \brief Function call operator, work just like the call function
    ///
    /// \param a_args The list of arguments for the call
    /// \return the result of the function
    ///////////////////////////////////////////////////////////////////////////
    t_RetType operator()(t_Args... a_args) { call(a_args...); }

    ///////////////////////////////////////////////////////////////////////////
    /// \brief Equal To operator
    ///
    /// \return true if the functions are the same
    ///////////////////////////////////////////////////////////////////////////
    friend mBool operator==(const mCallback& lhs, const mCallback& rhs)
    {
        return lhs.m_func.template target<mFunctionType>() ==
               rhs.m_func.template target<mFunctionType>();
    }

    ///////////////////////////////////////////////////////////////////////////
    /// \brief Explicit convertion operator to bool
    ///
    /// \return true if the function has been set, false otherwise
    ///////////////////////////////////////////////////////////////////////////
    explicit operator bool() const { return mBool(m_func); }

   private:
    ///////////////////////////////////////////////////////////////////////////
    /// \brief The underlying std::function
    ///////////////////////////////////////////////////////////////////////////
    std::function<mFunctionType> m_func;
};

///////////////////////////////////////////////////////////////////////////////
/// \brief Class encapsulating a signal
///
/// Functions registered to a signal can only return void. When a signal is
/// called with arguments, all its registered callback are called. Order
/// is not guaranteed.
///
/// \tparam t_Args The list of arguments of the function
///////////////////////////////////////////////////////////////////////////////
template <typename... t_Args>
class mSignal
{
   public:
    ///////////////////////////////////////////////////////////////////////////
    /// \brief Alias an iterator to a callback
    ///////////////////////////////////////////////////////////////////////////
    using mCallbackHandle =
        typename std::list<mCallback<void, t_Args...>>::iterator;

    ///////////////////////////////////////////////////////////////////////////
    /// \brief Attach a callback for the signal to call
    ///
    /// \param a_callback A callback to attach to the signal
    /// \return a handle to the callback in order to remove it later
    ///////////////////////////////////////////////////////////////////////////
    mCallbackHandle attach_toSignal(
        mCallback<void, t_Args...> const& a_callback)
    {
        m_callbacks.push_front(a_callback);
        return m_callbacks.begin();
    }

    ///////////////////////////////////////////////////////////////////////////
    /// \brief Detach a callback from the signal
    ///
    /// \param a_callback A callback to detach from the signal
    ///////////////////////////////////////////////////////////////////////////
    void detach_fromSignal(mCallback<void, t_Args...> a_callback)
    {
        m_callbacks.remove(a_callback);
    }

    ///////////////////////////////////////////////////////////////////////////
    /// \brief Detach a callback from the signal
    ///
    /// \param a_handle A handle to the callback to detach from the signal
    /// \pre a_handle should have been given by attach_toSignal
    ///////////////////////////////////////////////////////////////////////////
    void detach_fromSignal(const mCallbackHandle& a_handle)
    {
        mExpect(std::find(m_callbacks.begin(), m_callbacks.end(), *a_handle) !=
                m_callbacks.end());
        m_callbacks.erase(a_handle);
    }

    ///////////////////////////////////////////////////////////////////////////
    /// \brief Call all the callback attached to the signal
    ///
    /// \param a_args The list of arguments to pass to the callbacks
    ///////////////////////////////////////////////////////////////////////////
    void call(t_Args... a_args)
    {
        for (mCallback<void, t_Args...> cbs : m_callbacks)
        {
            cbs.call(a_args...);
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    /// \brief Clear the list of callback attached to the signal
    ///
    /// Warnning ! The gathered handles are not valid anymore and should not be
    /// used
    ///////////////////////////////////////////////////////////////////////////
    void clear() { m_callbacks.clear(); }

   private:
    ///////////////////////////////////////////////////////////////////////////
    /// \brief The list of callbacks
    ///////////////////////////////////////////////////////////////////////////
    std::list<mCallback<void, t_Args...>> m_callbacks;
};
};  // namespace m

///////////////////////////////////////////////////////////////////////////////
/// \}
///////////////////////////////////////////////////////////////////////////////