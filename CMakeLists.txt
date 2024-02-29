cmake_minimum_required(VERSION 3.12)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

project(tlc C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

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

include_directories(
    ./tlc
    ./dep/glfw/include
    ./dep/glm
    PUBLIC "$ENV{VULKAN_SDK}/include"
)

if (MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -pedantic -Werror)
endif()

add_subdirectory(./dep/glfw)

add_executable(tlc
# core
    ./tlc/core/Logger.cpp
    ./tlc/core/Utils.cpp
    ./tlc/core/Window.cpp
    ./tlc/core/Application.cpp
# vulkan
    ./tlc/vulkan/VulkanContext.cpp
    ./tlc/vulkan/VulkanDevice.cpp
    ./tlc/vulkan/VulkanSwapchain.cpp
    ./tlc/vulkan/VulkanBuffer.cpp
    ./tlc/vulkan/VulkanShader.cpp
    ./tlc/vulkan/VulkanFramebuffer.cpp
    ./tlc/vulkan/VulkanCommandBuffer.cpp
    ./tlc/vulkan/VulkanGraphicsPipeline.cpp
# rendering
    ./tlc/rendering/Renderer.cpp
# engine
    ./tlc/engine/Scene.cpp
# game
    ./tlc/game/Game.cpp
    ./tlc/game/scenes/MainScene.cpp
    ./tlc/game/scenes/TestScene.cpp
# main
    ./tlc/Main.cpp
)

target_link_libraries(tlc
    glfw
    ${Vulkan_LIBRARY}
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