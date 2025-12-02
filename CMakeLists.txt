cmake_minimum_required(VERSION 3.12...3.29)

option(KIERO_INCLUDE_D3D9 "Enable support for D3D9" OFF)
option(KIERO_INCLUDE_D3D10 "Enable support for D3D10" OFF)
option(KIERO_INCLUDE_D3D11 "Enable support for D3D11" OFF)
option(KIERO_INCLUDE_D3D12 "Enable support for D3D12" OFF)
option(KIERO_INCLUDE_OPENGL "Enable support for OpenGL" OFF)
option(KIERO_INCLUDE_VULKAN "Enable support for Vulkan" OFF)

option(KIERO_BUILD_MINHOOK "Builds the MinHook implementation for \"kiero::bind\"" OFF)
option(KIERO_MINHOOK_EXTERNAL "Use an externally provided MinHook instead of the bundled" OFF)

option(KIERO_BUILD_SAFETYHOOK "Builds the SafetyHook implementation for \"kiero::bind\"" OFF)
option(KIERO_SAFETYHOOK_EXTERNAL "Use an externally provided SafetyHook instead of the bundled" OFF)

option(KIERO_BUILD_EXAMPLES "Build example targets" ${PROJECT_IS_TOP_LEVEL})

project(kiero VERSION 1.2.12)

add_library(kiero-core STATIC
    src/kiero.cpp
)
add_library(kiero::kiero-core ALIAS kiero-core)
target_compile_definitions(kiero-core PUBLIC KIERO_VERSION=${PROJECT_VERSION})
target_compile_features(kiero-core PUBLIC cxx_std_20)

if (MSVC)
    if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        target_compile_options(kiero-core PRIVATE
            /clang:-Wall
            /clang:-Wextra
            /clang:-Wcast-align
            /clang:-Wconversion
            /clang:-Wsign-conversion
            /clang:-Wold-style-cast
            /clang:-Wimplicit-fallthrough
        )
    else()
        target_compile_options(kiero-core PRIVATE /WX /W4)
    endif()
else()
    message(WARNING "Non Microsoft compatible compiler detected")
endif()

if (NOT (KIERO_INCLUDE_D3D9 OR KIERO_INCLUDE_D3D10 OR KIERO_INCLUDE_D3D11 OR KIERO_INCLUDE_D3D12 OR KIERO_INCLUDE_OPENGL OR KIERO_INCLUDE_VULKAN))
    message(FATAL_ERROR "At least one graphics API must be specified")
endif()

if (KIERO_INCLUDE_D3D9)
    target_compile_definitions(kiero-core PUBLIC KIERO_INCLUDE_D3D9)
endif()

if (KIERO_INCLUDE_D3D10)
    target_compile_definitions(kiero-core PUBLIC KIERO_INCLUDE_D3D10)
endif()

if (KIERO_INCLUDE_D3D11)
    target_compile_definitions(kiero-core PUBLIC KIERO_INCLUDE_D3D11)
endif()

if (KIERO_INCLUDE_D3D12)
    target_compile_definitions(kiero-core PUBLIC KIERO_INCLUDE_D3D12)
endif()

if (KIERO_INCLUDE_OPENGL)
    find_package(OpenGL REQUIRED)
    target_link_libraries(kiero-core PUBLIC OpenGL::GL)
    target_compile_definitions(kiero-core PUBLIC KIERO_INCLUDE_OPENGL)
endif()

if (KIERO_INCLUDE_VULKAN)
    find_package(Vulkan REQUIRED)
    target_link_libraries(kiero-core PUBLIC Vulkan::Headers)
    target_compile_definitions(kiero-core PUBLIC KIERO_INCLUDE_VULKAN)
endif()

add_library(kiero STATIC src/hook/kiero_nobind.cpp)
target_link_libraries(kiero PUBLIC kiero::kiero-core)
add_library(kiero::kiero ALIAS kiero)

if (KIERO_BUILD_MINHOOK)
    if (NOT KIERO_MINHOOK_EXTERNAL)
        include(FetchContent)
        FetchContent_Declare(
            minhook
            GIT_REPOSITORY https://github.com/TsudaKageyu/minhook.git
            GIT_TAG        f5485b8454544c2f034c78f8f127c1d03dea3636
        )
        FetchContent_MakeAvailable(minhook)
    elseif (NOT TARGET minhook)
        message(FATAL_ERROR "minhook target not present")
    endif()
    add_library(kiero-minhook STATIC src/hook/kiero_minhook.cpp)
    target_link_libraries(kiero-minhook
        PUBLIC kiero::kiero-core
        PRIVATE minhook
    )
    add_library(kiero::kiero-minhook ALIAS kiero-minhook)
endif()

if (KIERO_BUILD_SAFETYHOOK)
    if (NOT KIERO_SAFETYHOOK_EXTERNAL)
        include(FetchContent)
        FetchContent_Declare(
            safetyhook
            GIT_REPOSITORY https://github.com/cursey/safetyhook.git
            GIT_TAG        c3f3f306a0f12d1811c0b713ad2ed2a8ddc6cf55 # v0.6.9
        )
        FetchContent_MakeAvailable(safetyhook)
    elseif (NOT TARGET safetyhook)
        message(FATAL_ERROR "safetyhook target not present")
    endif()
    add_library(kiero-safetyhook STATIC src/hook/kiero_safetyhook.cpp)
    target_link_libraries(kiero-safetyhook
        PUBLIC kiero::kiero-core
        PRIVATE safetyhook
    )
    add_library(kiero::kiero-safetyhook ALIAS kiero-safetyhook)
endif()

target_include_directories(kiero-core PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/>
    $<INSTALL_INTERFACE:include>
)

if (KIERO_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
