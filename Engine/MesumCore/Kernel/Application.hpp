#pragma once

#include <Kernel.hpp>
#include <Types.hpp>

/*! \addtogroup Core
 *@{
 */
//! Namespace grouping basic application structures
namespace m::application
{
//! Interface of basic applications
/*!
 * \brief It can be launched !
 */
class mIBasicApplication
{
   public:
    //! Launch the application
    /*!
     * \param a_cmdLine A reference to a parsed cmdLine
     * \param a_appData A pointer to a nameless structure containing application
     *  data (might bu nullptr)
     */
    virtual void launch(mCmdLine const& a_cmdLine, void* a_appData) = 0;
};

//! Interface for a loop based application
/*!
 * \brief It has a three step life cycle : init called first, a step called in a
 * loop, and finally a destroy when exiting
 */
class mILoopApplication : public mIBasicApplication
{
   public:
    //! Launch the application
    /*!
     * \param a_cmdLine A reference to a parsed cmdLine
     * \param a_appData A pointer to a nameless structure containing application
     *  data (might be nullptr)
     */
    void launch(mCmdLine const& a_cmdLine, void* a_appData) final;

   private:
    //! Virtual that should implement initialization
    /*!
     * \param a_cmdLine A reference to a parsed cmdLine
     * \param a_appData A pointer to a nameless structure containing application
     *  data (might be nullptr)
     */
    virtual void init(mCmdLine const& a_cmdLine, void* a_appData) = 0;
    //! Virtual that should implement the application exits
    virtual void destroy() = 0;
    //! Virtual that should implement each step
    /*!
     * \return should return true for the application to continue, false to stop
     */
    virtual Bool step() = 0;
};

//! Interface for a loop based application with timmed steps
/*!
 * \brief It has a three step life cycle : init called first, a step called in a
 * loop, and finally a destroy when exiting
 */
class mITimedLoopApplication : public mIBasicApplication
{
   public:
    //! Launch the application
    /*!
     * \param a_cmdLine A reference to a parsed cmdLine
     * \param a_appData A pointer to a nameless structure containing application
     *  data (might bu nullptr)
     */
    void launch(mCmdLine const& a_cmdLine, void* a_appData) final;
    //! Set the minimal duration of a step
    /*!
     * \param a_minStepDuration The target minimal duration
     */
    inline void set_minimalStepDuration(
        std::chrono::steady_clock::duration a_minStepDuration)
    {
        m_minStepDuration = a_minStepDuration;
    }
    //! Get the minimal duration of a step
    /*!
     * \return The target minimal duration
     */
    inline std::chrono::steady_clock::duration get_minimalStepDuration() const
    {
        return m_minStepDuration;
    }

   private:
    //! Virtual that should implement initialization
    /*!
     * \param a_cmdLine A reference to a parsed cmdLine
     * \param a_appData A pointer to a nameless structure containing application
     *  data (might be nullptr)
     */
    virtual void init(mCmdLine const& a_cmdLine, void* a_appData) = 0;
    //! Virtual that should implement the application exits
    virtual void destroy() = 0;
    //! Virtual that should implement each step
    /*!
     * \param a_deltaTime The time elapsed since last step in microseconds
     * \return should return true for the application to continue, false to stop
     */
    virtual Bool step(
        std::chrono::steady_clock::duration const& a_deltaTime) = 0;

    /*! the minimal duration a step can take*/
    std::chrono::steady_clock::duration m_minStepDuration{0};
};

}  // namespace m::application

/*!
 * @}
 * */