#include <Application.hpp>

namespace m
{
	namespace application
	{
		void ITimedLoopApplication::launch()
		{
			init();

			auto start = std::chrono::high_resolution_clock::now();
			Double deltaTime = 0.0;
			while (step(deltaTime))
			{
				auto end = std::chrono::high_resolution_clock::now();
				I64 timming =
					std::chrono::duration_cast<std::chrono::duration<I64, std::micro>>(end - start)
					.count();
				//LOG("Frame Lasted ", timming, " microseconds")
				if (timming < m_limitMicroSecondsPerFrame)
				{
					std::this_thread::sleep_for(
						std::chrono::microseconds(m_limitMicroSecondsPerFrame - timming));
					timming = m_limitMicroSecondsPerFrame;
				}
				deltaTime = timming * 0.000001;
				start = std::chrono::high_resolution_clock::now();
			}

			destroy();
		}

		void ILoopApplication::launch()
		{
			init();

			while (!step())
			{
				//Nothing needed here
			}

			destroy();
		}


		void IPlatformAppBase::init()
		{
			configure();
		}
	};
};