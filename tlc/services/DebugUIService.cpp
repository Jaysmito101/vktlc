#include "services/DebugUIManager.hpp"
#include "core/Window.hpp"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"


#include "services/renderer/VulkanManager.hpp"


namespace tlc {

    static void CheckVkResult(VkResult err)
    {
        if (err == 0) return;
        log::Fatal("VkResult: %d", (U64)err);
    }

    void DebugUIManager::Setup() {}

    void DebugUIManager::OnStart() {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;        // Enable Gamepad Controls

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();  

        SetupForVulkan();
    }

    void DebugUIManager::OnEnd() {

    }

    void DebugUIManager::OnSceneChange() {

    }

    void DebugUIManager::OnEvent(const String& event, const String& eventParams) {

    }

    void DebugUIManager::SetupForVulkan() {
        // ImGui_ImplGlfw_InitForVulkan(Window::Get()->GetHandle(), true);

        // auto vulkan = Services::GetService<VulkanManager>();

        // ImGui_ImplVulkan_InitInfo init_info = {};

        // init_info.Instance = vulkan->GetContext()->GetInstance();
        // init_info.PhysicalDevice = vulkan->GetPhysicalDevice();
        // init_info.Device = vulkan->GetDevice()->GetDevice();
        // init_info.QueueFamily = vulkan->GetDevice()->GetGraphicsQueueFamilyIndex();
        // init_info.Queue = vulkan->GetDevice()->GetQueue(VulkanQueueType::Graphics);
        // init_info.PipelineCache = VK_NULL_HANDLE;
        // init_info.DescriptorPool = vulkan->GetDevice()->GetDescriptorPool();
        // init_info.RenderPass = wd->RenderPass;
        // init_info.Subpass = 0;
        // init_info.MinImageCount = 2;
        // init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
        // init_info.Allocator = nullptr;
        // init_info.CheckVkResultFn = CheckVkResult;

        // ImGui_ImplVulkan_Init(&init_info);
    }

}