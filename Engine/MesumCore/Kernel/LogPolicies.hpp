#pragma once
#include <fstream>
#include <iostream>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>
///////////////////////////////////////////////////////////////////////////////
/// \addtogroup Core
/// \{
///////////////////////////////////////////////////////////////////////////////
namespace m::logging
{
///////////////////////////////////////////////////////////////////////////////
/// \brief Interface defining a logging policy
///
/// The policy defines where the logs are outputted (file, stream, etc...)
///////////////////////////////////////////////////////////////////////////////
class mILogPolicy
{
   public:
    ///////////////////////////////////////////////////////////////////////////
    /// \brief Default policy destructor
    ///////////////////////////////////////////////////////////////////////////
    virtual ~mILogPolicy() = default;

    ///////////////////////////////////////////////////////////////////////////
    /// \brief Open the output stream
    ///
    /// \param a_name The name of the stream
    ///////////////////////////////////////////////////////////////////////////
    virtual void open_ostream(const std::string& a_name) = 0;

    ///////////////////////////////////////////////////////////////////////////
    /// \brief Close the output stream
    ///////////////////////////////////////////////////////////////////////////
    virtual void close_ostream() = 0;

    ///////////////////////////////////////////////////////////////////////////
    /// \brief Write into the output stream
    ///
    /// \param a_msg The message to output
    ///////////////////////////////////////////////////////////////////////////
    virtual void write(const std::string& a_msg) = 0;
};

///////////////////////////////////////////////////////////////////////////////
/// \brief Log policy outputting into a file
///////////////////////////////////////////////////////////////////////////////
class mFileLogPolicy final : public mILogPolicy
{
   public:
    ///////////////////////////////////////////////////////////////////////////
    /// \brief Construct a file logging policy
    ///
    /// This creates a new ofstream object
    ///////////////////////////////////////////////////////////////////////////
    mFileLogPolicy() : m_outStream(new std::ofstream) {}

    ///////////////////////////////////////////////////////////////////////////
    /// \brief Destroy a file logging policy
    ///
    /// This destroys the ofstream object
    ///////////////////////////////////////////////////////////////////////////
    ~mFileLogPolicy() final;

    ///////////////////////////////////////////////////////////////////////////
    /// \brief Open the output stream
    ///
    /// \param a_name The name of the file to open
    ///////////////////////////////////////////////////////////////////////////
    void open_ostream(const std::string& a_name) override;

    ///////////////////////////////////////////////////////////////////////////
    /// \brief Close the file stream
    ///////////////////////////////////////////////////////////////////////////
    void close_ostream() override;

    ///////////////////////////////////////////////////////////////////////////
    /// \brief Write into the output file
    ///
    /// \param a_msg The message to output
    ///////////////////////////////////////////////////////////////////////////
    void write(const std::string& a_msg) override;

   private:
    std::unique_ptr<std::ofstream> m_outStream;  //!< The output file
};

///////////////////////////////////////////////////////////////////////////////
/// \brief Log policy outputting into the standard output
///////////////////////////////////////////////////////////////////////////////
class mStdcoutLogPolicy final : public mILogPolicy
{
   public:
    ///////////////////////////////////////////////////////////////////////////
    /// \brief Constructs the std::cout logging policy.
    ///////////////////////////////////////////////////////////////////////////
    mStdcoutLogPolicy() = default;
    ///////////////////////////////////////////////////////////////////////////
    //! Destroys a std::cout logging policy.
    ///////////////////////////////////////////////////////////////////////////
    ~mStdcoutLogPolicy() final = default;
    ///////////////////////////////////////////////////////////////////////////
    /// \brief Does nothing
    ///////////////////////////////////////////////////////////////////////////
    void open_ostream(const std::string& a_name) override{};
    ///////////////////////////////////////////////////////////////////////////
    /// \brief Does nothing
    ///////////////////////////////////////////////////////////////////////////
    void close_ostream() override{};
    ///////////////////////////////////////////////////////////////////////////
    /// \brief Write into the standard output
    ///
    /// \param a_msg The message to output
    ///////////////////////////////////////////////////////////////////////////
    void write(const std::string& a_msg) override;
};

}  // namespace m::logging
///////////////////////////////////////////////////////////////////////////////
/// \}
///////////////////////////////////////////////////////////////////////////////