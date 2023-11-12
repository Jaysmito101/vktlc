#include "core/Core.hpp"
#include "core/Application.hpp"

int main()
{
    tlc::Logger::Init();

    auto app = tlc::Application::Get();
    app->Run();
    tlc::Application::Shutdown();

    tlc::Logger::Shutdown();
    return 0;
}