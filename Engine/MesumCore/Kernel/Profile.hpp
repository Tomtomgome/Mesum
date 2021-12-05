#pragma once

#include <array>
#include <chrono>
#include <numeric>

///////////////////////////////////////////////////////////////////////////////
/// \addtogroup Core
/// \{
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
/// \brief Namespace grouping basic profiling tools
///////////////////////////////////////////////////////////////////////////////
namespace m::profile
{
///////////////////////////////////////////////////////////////////////////////
/// \brief Interface core to profiling
///////////////////////////////////////////////////////////////////////////////
class mIProfiler
{
   public:
    ///////////////////////////////////////////////////////////////////////////
    /// \brief Add a timming data to a profiler
    ///
    /// \param a_timming The timming data for the thing we want to measure
    ///////////////////////////////////////////////////////////////////////////
    virtual void add_timing(std::chrono::steady_clock::duration a_timming) = 0;
};

///////////////////////////////////////////////////////////////////////////////
/// \brief Implementation of a profiler that averages times accross multiple
/// samples
///////////////////////////////////////////////////////////////////////////////
template <mUInt t_SampleCount>
class mProfilerMultiSampling : public mIProfiler
{
   public:
    void add_timing(std::chrono::steady_clock::duration a_timming) override;
    template <typename t_RetType, typename t_Preiod>
    t_RetType get_average();

   private:
    std::array<std::chrono::steady_clock::duration, t_SampleCount> m_samples{};
    mUInt m_currentCount{0};
};

///////////////////////////////////////////////////////////////////////////////
/// \brief RAII Timming that sends the elapsed time between its creation and
/// destruction to its parent profiler
///////////////////////////////////////////////////////////////////////////////
class mRAIITiming
{
   public:
    mRAIITiming() = delete;
    ///////////////////////////////////////////////////////////////////////////
    /// \brief Construct a Timming with its associated parent.
    ///
    /// This gets a timestamps of this call
    ///
    /// \param a_parent The parent profiler to send the data to
    ///////////////////////////////////////////////////////////////////////////
    explicit mRAIITiming(mIProfiler& a_parent);

    ///////////////////////////////////////////////////////////////////////////
    /// \brief Sends the time elapsed between this and the construction of
    /// the object to the parent profiler
    ///
    /// \pre The parent must not be nullptr
    ///////////////////////////////////////////////////////////////////////////
    ~mRAIITiming();

   private:
    ///////////////////////////////////////////////////////////////////////////
    /// \brief The profiler to send the data to
    ///////////////////////////////////////////////////////////////////////////
    mIProfiler* m_pParent = nullptr;
    ///////////////////////////////////////////////////////////////////////////
    /// \brief The starting point of the sampling data
    ///////////////////////////////////////////////////////////////////////////
    std::chrono::time_point<std::chrono::steady_clock> m_start;
};

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
template <mUInt t_SampleCount>
void mProfilerMultiSampling<t_SampleCount>::add_timing(
    std::chrono::steady_clock::duration a_timming)
{
    m_samples[m_currentCount % t_SampleCount] = a_timming;
    m_currentCount++;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
template <mUInt t_SampleCount>
template <typename t_RetType, typename t_Preiod>
t_RetType mProfilerMultiSampling<t_SampleCount>::get_average()
{
    auto allTimings = std::accumulate(m_samples.begin(), m_samples.end(),
                                      std::chrono::steady_clock::duration(0));
    return std::chrono::duration_cast<
               std::chrono::duration<t_RetType, t_Preiod>>(allTimings)
               .count() /
           std::max(1u, std::min(t_SampleCount, m_currentCount));
}

};  // namespace m::profile
