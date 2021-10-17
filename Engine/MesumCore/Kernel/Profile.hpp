#ifndef M_Profile
#define M_Profile
#pragma once

#include <array>
#include <chrono>
#include <numeric>

namespace m::profile
{
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
class IProfiler
{
   public:
    virtual void add_timing(std::chrono::steady_clock::duration a_timming) = 0;
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
template <UInt t_SampleCount>
class MultiSamplingProfiler : public IProfiler
{
   public:
    void add_timing(std::chrono::steady_clock::duration a_timming) override;
    template <typename t_RetType, typename t_Preiod>
    t_RetType get_average();

   private:
    std::array<std::chrono::steady_clock::duration, t_SampleCount> m_samples{};
    UInt m_currentCount{0};
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
class Timing
{
   public:
    Timing() = delete;
    Timing(IProfiler& a_parent) : m_pParent(&a_parent)
    {
        m_start = std::chrono::high_resolution_clock::now();
    }
    ~Timing()
    {
        auto end = std::chrono::high_resolution_clock::now();
        m_pParent->add_timing(end - m_start);
    }

   private:
    IProfiler*                                         m_pParent;
    std::chrono::time_point<std::chrono::steady_clock> m_start;
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
template <UInt t_SampleCount>
void MultiSamplingProfiler<t_SampleCount>::add_timing(
    std::chrono::steady_clock::duration a_timming)
{
    m_samples[m_currentCount % t_SampleCount] = a_timming;
    m_currentCount++;
}

template <UInt t_SampleCount>
template <typename t_RetType, typename t_Preiod>
t_RetType MultiSamplingProfiler<t_SampleCount>::get_average()
{
    auto allTimings = std::accumulate(m_samples.begin(), m_samples.end(), std::chrono::steady_clock::duration(0));
    return std::chrono::duration_cast<
               std::chrono::duration<t_RetType, t_Preiod>>(allTimings).count() /
               std::min(t_SampleCount, m_currentCount);
}

};  // namespace m::profile

#endif  // M_Profile