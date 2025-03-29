#include "game/Game.hpp"

int main()
{
    tlc::Logger::Init();
    tlc::Logger::Get()->SetLogLevelFilter(tlc::LogLevel::InfoP);
    tlc::log::AttachFile(tlc::utils::GetExecutableDirectory() + "/log.txt");

    (void)tlc::GameApplication::Create();

    auto app = tlc::Application::Get();
    app->Run();
    tlc::Application::Shutdown();

    tlc::Logger::Shutdown();
    return 0;
}