cmake_minimum_required(VERSION 3.12)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

project(tlc C CXX)

add_compile_definitions(_CRT_SECURE_NO_WARNINGS)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(SHADERC_SKIP_TESTS ON)

# Use FindVulkan module added with CMAKE 3.7
if(NOT CMAKE_VERSION VERSION_LESS 3.7.0)
    message(STATUS "Using module to find Vulkan")
    find_package(Vulkan)
endif()

IF(UNIX AND NOT APPLE)
    set(LINUX TRUE)
ENDIF()

IF(WIN32)
    IF(NOT Vulkan_FOUND)
        find_library(Vulkan_LIBRARY NAMES vulkan-1 vulkan PATHS ${CMAKE_SOURCE_DIR}/libs/vulkan)

        IF(Vulkan_LIBRARY)
            set(Vulkan_FOUND ON)
            MESSAGE("Using bundled Vulkan library version")
        ENDIF()
    ENDIF()

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DVK_USE_PLATFORM_WIN32_KHR")
ELSEIF(LINUX)
    IF(NOT Vulkan_FOUND)
        find_library(Vulkan_LIBRARY NAMES vulkan HINTS "$ENV{VULKAN_SDK}/lib" "${CMAKE_SOURCE_DIR}/libs/vulkan" REQUIRED)

        IF(Vulkan_LIBRARY)
            set(Vulkan_FOUND ON)
            MESSAGE("Using bundled Vulkan library version")
        ENDIF()
    ENDIF()
ENDIF()

IF(NOT Vulkan_FOUND)
    message(FATAL_ERROR "Could not find Vulkan library!")
ELSE()
    message(STATUS ${Vulkan_LIBRARY})
ENDIF()

add_subdirectory(./dep/glfw)
add_subdirectory(./dep/imguibuilder)



include_directories(
    ./tlc
    ./dep/glfw/include
    ./dep/glm
    ./dep/imgui
    ./dep/imgui/backends
    # ./dep/shaderc/libshaderc/include
    # PUBLIC "$ENV{VULKAN_SDK}/include"
    ${Vulkan_INCLUDE_DIRS}
)



add_executable(tlc
# core
    ./tlc/core/Uuid.cpp
    ./tlc/core/Logger.cpp
    ./tlc/core/Utils.cpp
    ./tlc/core/Window.cpp
    ./tlc/core/Application.cpp
# vulkan
    ./tlc/vulkanapi/VulkanContext.cpp
    ./tlc/vulkanapi/VulkanDevice.cpp
    ./tlc/vulkanapi/VulkanSwapchain.cpp
    ./tlc/vulkanapi/VulkanBuffer.cpp
    ./tlc/vulkanapi/VulkanShader.cpp
    ./tlc/vulkanapi/VulkanImage.cpp
    ./tlc/vulkanapi/VulkanGraphicsPipeline.cpp
# services
    ./tlc/services/Services.cpp
    ./tlc/services/ShaderCompilerService.cpp
    ./tlc/services/CacheManagerService.cpp
    ./tlc/services/assetmanager/AssetManagerService.cpp
    ./tlc/services/assetmanager/AssetBundlerService.cpp
    ./tlc/services/renderer/VulkanManagerService.cpp
    ./tlc/services/renderer/PresentationRendererService.cpp
    ./tlc/services/renderer/DebugUIManagerService.cpp
# engine
    ./tlc/engine/Scene.cpp
    # ./tlc/engine/ecs/Component.cpp
    # ./tlc/engine/ecs/Entity.cpp
    ./tlc/engine/ecs/ECSBase.cpp
# game
    ./tlc/game/Game.cpp
    ./tlc/game/RegisterAssets.cpp
    ./tlc/game/RegisterServices.cpp
    # ./tlc/game/scenes/MainScene.cpp
    # ./tlc/game/scenes/TestScene.cpp
    # ./tlc/game/components/TriangleUpDown.cpp
    # ./tlc/game/components/TriangleLeftRight.cpp
# main
    ./tlc/Main.cpp
# third party
    ./dep/imgui/backends/imgui_impl_glfw.cpp
)
target_link_libraries(tlc
    glfw
    imgui
    ${Vulkan_LIBRARY}
    debug $ENV{VULKAN_SDK}/Lib/shaderc_combinedd.lib
    optimized $ENV{VULKAN_SDK}/Lib/shaderc_combined.lib
)




if (WIN32)
    target_compile_definitions(tlc
        PUBLIC _CRT_SECURE_NO_WARNINGS
    )
endif()

if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 19.12.25835)
    set(CMAKE_CXX20_STANDARD_COMPILE_OPTION "-std:c++latest")
    set(CMAKE_CXX20_EXTENSION_COMPILE_OPTION "-std:c++latest")
endif()

if( MSVC )
    if(${CMAKE_VERSION} VERSION_LESS "3.6.0") 
        message( "\n\t[ WARNING ]\n\n\tCMake version lower than 3.6.\n\n\t - Please update CMake and rerun; OR\n\t - Manually set 'GLFW-CMake-starter' as StartUp Project in Visual Studio.\n" )
    else()
        set_property( DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT tlc )
    endif()
endif()