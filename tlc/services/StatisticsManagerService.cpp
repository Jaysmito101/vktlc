#pragma once

#include "services/StatisticsManager.hpp"
#include "utils/Utils.hpp" 

#include "imgui.h"

#ifdef TLC_ENABLE_STATISTICS
namespace tlc {

    void StatisticsManager::Setup() {
    }

    void StatisticsManager::OnStart() {
        
    }

    void StatisticsManager::OnEnd() {
    }

    void StatisticsManager::OnSceneChange() { 
    }

    void StatisticsManager::OnEvent(const String& event, const String& eventParams) {
        (void)event;
        (void)eventParams;
    }

    void StatisticsManager::NewFrame() {
        m_CurrentFrameStats.clear();
    }

    void StatisticsManager::EndFrame() {
        for (auto& [statKey, statRB] : m_StatsHistory) {
            auto it = m_CurrentFrameStats.find(statKey);
            if (it != m_CurrentFrameStats.end()) {
                statRB.Push(it->second);
                m_CurrentFrameStats.erase(statKey);
            }
            else {
                statRB.Push(0.0);
            }
        }

        // For the new items that have been added in the current frame
        if (m_CurrentFrameStats.size() > 0) {
            for (auto& [statKey, statValue] : m_CurrentFrameStats) {
                m_StatsHistory[statKey].Fill(0.0, m_FrameCount % k_MaxFrames);
                m_StatsHistory[statKey].Push(statValue);
            }
        }

        m_FrameCount++;
    }

    void StatisticsManager::ClearStats() {
        m_CurrentFrameStats.clear();
        m_StatsHistory.clear();
    }

    void StatisticsManager::SetStat(const String& statName, F32 value) {
        m_CurrentFrameStats[statName] = value;
    }

    void StatisticsManager::ShowDebugUI(Raw<Bool> windowOpen) {
        ImGui::Begin("Statistics", windowOpen);

        // General Info Section
        ImGui::Text("General Info");
        ImGui::Separator();
        ImGui::Text("Current Frame: %llu", m_FrameCount);
        ImGui::Text("Frame Time: %.2f ms", ImGui::GetIO().DeltaTime * 1000.0f);
        ImGui::Text("Frame Rate: %.1f FPS", ImGui::GetIO().Framerate);
        static float avgFramerate = 0.0f;
        avgFramerate = (avgFramerate * (m_FrameCount - 1) + ImGui::GetIO().Framerate) / m_FrameCount;
        ImGui::Text("Average Frame Rate: %.1f FPS", avgFramerate);

        if (ImGui::BeginTabBar("StatsTabBar")) {
            if (ImGui::BeginTabItem("Per Frame Stats")) {
                static char searchBuffer[1024] = "";
                ImGui::InputTextWithHint("##Search", "Search stats...", searchBuffer, IM_ARRAYSIZE(searchBuffer));

                ImGui::PushID("PerFrameStats");
                for (const auto& [statName, statsRB] : m_StatsHistory) {
                    if (searchBuffer[0] == '\0' || utils::LevenshteinSubstringMatch(statName, searchBuffer) > 0.8f) {
                        bool isTimeStat = statName.find("Time") != String::npos;

                        auto currentValue = String();
                        if (isTimeStat) {
                            currentValue = std::format("[{:.4f} ms]", statsRB.Top() / 1000.0f);
                        }
                        else {
                            currentValue = std::format("[{:.2f}]", statsRB.Top());
                        }
                        
                        auto stateHeader = std::format("{} {}###StatItem", statName, currentValue);
                        ImGui::PushID(statName.c_str());
                        if (ImGui::CollapsingHeader(stateHeader.c_str())) {
                            static F32 statsBuffer[k_MaxFrames] = { 0 };
                            std::memset(statsBuffer, 0, sizeof(statsBuffer));
                            statsRB.CopyToArray(statsBuffer, nullptr);
                            std::reverse(statsBuffer, statsBuffer + k_MaxFrames);
                            F32 minValue = FLT_MAX, maxValue = FLT_MIN, avgValue = 0.0;
                            for (U32 i = 1; i < k_MaxFrames; i++) {
                                if (statsBuffer[i] < minValue) minValue = statsBuffer[i];
                                if (statsBuffer[i] > maxValue) maxValue = statsBuffer[i];
                                avgValue += statsBuffer[i];
                            }
                            avgValue /= k_MaxFrames;
                            for (U32 i = 1; i < k_MaxFrames; i++) {
                                statsBuffer[i] = statsBuffer[i] / maxValue;
                            }

                            if (isTimeStat) {
                                ImGui::Text("Min: %.4f ms", minValue / 1000.0f);
                                ImGui::Text("Max: %.4f ms", maxValue / 1000.0f);
                                ImGui::Text("Avg: %.4f ms", avgValue / 1000.0f);
                            }
                            else {
                                ImGui::Text("Min: %.2f", minValue);
                                ImGui::Text("Max: %.2f", maxValue);
                                ImGui::Text("Avg: %.2f", avgValue);
                            }

                            ImGui::PlotLines("##StatPlot", statsBuffer + 1, k_MaxFrames - 1, 0, nullptr, 0.0f, 1.0f, ImVec2(0, 80.0f));
                        }
                        ImGui::PopID();
                    }
                }
                ImGui::PopID();                
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Consistent Stats")) {
                ImGui::Text("Consistent stats will be displayed here.");
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }

        ImGui::End();
    }
}
#endif // TLC_ENABLE_STATISTICS