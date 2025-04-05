#pragma once

#include "core/Core.hpp"
#include "containers/RingBuffer.hpp"
#include "services/Services.hpp"

// TODO: Move this to the the CMakefile
#define TLC_ENABLE_STATISTICS

#ifdef TLC_ENABLE_STATISTICS
namespace tlc {
    class StatisticsManager : public IService {
    public:
        void Setup();

        void OnStart() override;
        void OnEnd() override;
        void OnSceneChange() override;
        void OnEvent(const String& event, const String& eventParams) override;

        void NewFrame();
        void EndFrame();
        void ClearStats();
        void SetStat(const String& statName, F32 value);
        
        void ShowDebugUI(Raw<Bool> windowOpen);

    private:
        static const Size k_MaxFrames = 300;
        
        Size m_FrameCount = 0;

        UnorderedMap<String, F32> m_CurrentFrameStats;
        UnorderedMap<String, RingBuffer<F32, k_MaxFrames>> m_StatsHistory;
    };

    namespace internal {
        struct StaticticsManagerScopeTimer {
            StaticticsManagerScopeTimer(const String& name) : m_Name(name) {
                m_StartTime = std::chrono::high_resolution_clock::now();
            }

            ~StaticticsManagerScopeTimer() {
                auto endTime = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - m_StartTime).count();
                Services::Get<StatisticsManager>()->SetStat(m_Name, static_cast<F32>(duration));
            }

        private:
            std::chrono::high_resolution_clock::time_point m_StartTime;
            String m_Name;
        };
    }
}
#endif


#ifdef TLC_ENABLE_STATISTICS

#define TLC_STATISTICS_PER_FRAME_TIME_SCOPE(name) tlc::internal::StaticticsManagerScopeTimer __statisticsscope##__LINE__(name)
#define TLC_STATISTICS_PER_FRAME_CUSTOM_TIME_SCOPE(name, value) tlc::Services::Get<tlc::StatisticsManager>()->SetStat(name, value)
#define TLC_STATISTICS_PER_FRAME(name, code) \
{ \
    TLC_STATISTICS_PER_FRAME_TIME_SCOPE(name); \
    code; \
}

#else

#define TLC_STATISTICS_PER_FRAME_TIME_SCOPE(name) (void)0
#define TLC_STATISTICS_PER_FRAME_CUSTOM_TIME_SCOPE(name, value) (void)0
#define TLC_STATISTICS_PER_FRAME(name, code) { code; }

#endif