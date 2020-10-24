#ifndef M_APPLICATION
#define M_APPLICATION
#pragma once

#include "Types.hpp"

#include <chrono>
#include <thread>

namespace m 
{
    class ITimedApplication
    {
        virtual void init() = 0;
        virtual void destroy() = 0;
        virtual mBool step(const Double& a_deltaTime) = 0;
    public:
        void launch()
        {
            init();


            auto start = std::chrono::high_resolution_clock::now();
            Double deltaTime = 0.0;
            while(!step(deltaTime))
            {
                auto end = std::chrono::high_resolution_clock::now();
                long long timming =
                std::chrono::duration_cast<std::chrono::microseconds>(end - start)
                        .count();
                //LOG("Frame Lasted ", timming, " microseconds")
                if (timming < 16000)
                {
                    std::this_thread::sleep_for(
                        std::chrono::microseconds(16000 - timming));
                    timming = 16000;
                }
                deltaTime = timming * 0.000001;
                start = std::chrono::high_resolution_clock::now();
            }

            destroy();
        }
    };

}
#endif //M_APPLICATION