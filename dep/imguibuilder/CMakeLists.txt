add_library(imgui
    ../imgui/imgui.cpp
    ../imgui/imgui_demo.cpp
    ../imgui/imgui_draw.cpp
    ../imgui/imgui_widgets.cpp
    ../imgui/imgui_tables.cpp
    ../imgui/backends/imgui_impl_glfw.cpp
    ../imgui/backends/imgui_impl_vulkan.cpp
)

target_include_directories(imgui PRIVATE
    ../glfw/include
    ../imgui
    ../imgui/backends
    ${Vulkan_INCLUDE_DIRS}
)


target_link_libraries(imgui
    glfw
    ${Vulkan_LIBRARY}
)