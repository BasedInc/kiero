include(FetchContent)
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG        6d910d5487d11ca567b61c7824b0c78c569d62f0 # v1.92.5
)
FetchContent_MakeAvailable(imgui)

add_library(imgui STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_dx9.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_dx10.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_dx11.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_dx12.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_win32.cpp
)
target_include_directories(imgui PUBLIC
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)

add_library(kiero_imgui_demo SHARED
    imgui/main.cpp
    imgui/impl/d3d9_impl.cpp
    imgui/impl/d3d10_impl.cpp
    imgui/impl/d3d11_impl.cpp
    imgui/impl/d3d12_impl.cpp
    imgui/impl/opengl_impl.cpp
    imgui/impl/win32_impl.cpp
)
target_link_libraries(kiero_imgui_demo PRIVATE
    kiero::kiero
    imgui

    RuntimeObject.lib
    dxgi.lib
    opengl32.lib
)
