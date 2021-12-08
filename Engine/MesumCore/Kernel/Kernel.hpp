#pragma once

#include "../Common/CoreCommon.hpp"
#include "Asserts.hpp"
#include "Types.hpp"
#include <algorithm>
#include <sstream>
#include <string>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
/// \addtogroup Core
/// \{
///////////////////////////////////////////////////////////////////////////////
namespace m
{
///////////////////////////////////////////////////////////////////////////////
/// \brief Aliases an error code
///////////////////////////////////////////////////////////////////////////////
using mErrorCode = mU64;
static const mErrorCode ecSuccess = 0; //!< default success code

#define mNotSuccess(ec) ec != 0 //!< alias a not success condition
#define mIsSuccess(ec) ec == 0 //!< alias a success condition

///////////////////////////////////////////////////////////////////////////////
/// \brief Structure that defines an output accompanied with a result
///
/// \tparam The type of the output
///////////////////////////////////////////////////////////////////////////////
template<typename t_Output>
struct mOutput
{
    mErrorCode res; //!< The potential output error
    t_Output output; //!< The actual output
};

///////////////////////////////////////////////////////////////////////////////
/// \brief Structure holding a list of command line arguments
///
/// For exemple holds the line executable -w 1080 -h 720 -fullScreen
/// will hold an array of strings with {-w, 1080, -h, 720, -fullScreen}
///////////////////////////////////////////////////////////////////////////////
struct mCmdLine
{
    ///////////////////////////////////////////////////////////////////////////
    /// \brief Initialize the structure given classic cmdline structures
    ///
    /// \param a_argc The number of arguments
    /// \param a_argv The array of arguments to parse
    ///////////////////////////////////////////////////////////////////////////
    void parse_cmdLineAguments(mInt a_argc, mChar** a_argv);

    ///////////////////////////////////////////////////////////////////////////
    /// \brief Get the value of a command line argument
    ///
    /// \brief for a pair of arguments of the form -a_key value
    /// \tparam t_ValueType The type of the value to be returned
    /// \param a_key The key we want to check
    /// \pre a_key should not be empty
    /// \param a_result The value associated with the key
    /// \return true if the key was found, false otherwise
    ///////////////////////////////////////////////////////////////////////////
    template <class t_ValueType>
    mBool get_parameter(const std::string a_key, t_ValueType& a_result) const
    {
        mExpect(!a_key.empty());

        auto found = std::find(m_listArgs.begin(), m_listArgs.end(), a_key);
        if (found == m_listArgs.end())
        {
            return false;
        }

        ++found;
        mAssert(found != m_listArgs.end())

                std::basic_istringstream(*found) >>
            a_result;
        return true;
    }

    ///////////////////////////////////////////////////////////////////////////
    /// \brief Check the presence of a command line argument
    ///
    /// \param a_arg The argument we want to check the presence of
    /// \return true if the key was found, false otherwise
    ///////////////////////////////////////////////////////////////////////////
    [[nodiscard]] inline mBool get_arg(std::string const& a_arg) const
    {
        return std::find(m_listArgs.begin(), m_listArgs.end(), a_arg) !=
               m_listArgs.end();
    }

    std::vector<std::string> m_listArgs;  //!< Holds the list of arguments
};

///////////////////////////////////////////////////////////////////////////////
/// \brief Structure that might hold console app launch data
///////////////////////////////////////////////////////////////////////////////
struct mBasicLaunchData
{
};
#ifdef M_WIN32
///////////////////////////////////////////////////////////////////////////////
/// \brief Hack to convert strings into wide strings
///
/// \param a_as The input string
/// \return true The output string as a wide string
///////////////////////////////////////////////////////////////////////////////
std::wstring convert_string(std::string const& a_as);
#endif

}  // namespace m

///////////////////////////////////////////////////////////////////////////////
/// \}
///////////////////////////////////////////////////////////////////////////////