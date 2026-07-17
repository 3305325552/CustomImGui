cmake_minimum_required(VERSION 3.24)

project(CustomImGui VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(CUSTOM_IMGUI_BUILD_TESTS "Build CustomImGui test applications" ${PROJECT_IS_TOP_LEVEL})
option(CUSTOM_IMGUI_BUILD_IMANIM_DEMOS "Compile the large upstream ImAnim demo windows" OFF)

set(CUSTOM_IMGUI_IMGUI_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party/imgui")
set(CUSTOM_IMGUI_IMANIM_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party/ImAnim")

add_library(custom_imgui STATIC
    ${CUSTOM_IMGUI_IMGUI_DIR}/imgui.cpp
    ${CUSTOM_IMGUI_IMGUI_DIR}/imgui_draw.cpp
    ${CUSTOM_IMGUI_IMGUI_DIR}/imgui_tables.cpp
    ${CUSTOM_IMGUI_IMGUI_DIR}/imgui_widgets.cpp
    ${CUSTOM_IMGUI_IMGUI_DIR}/misc/cpp/imgui_stdlib.cpp
    ${CUSTOM_IMGUI_IMGUI_DIR}/backends/imgui_impl_dx11.cpp
    ${CUSTOM_IMGUI_IMGUI_DIR}/backends/imgui_impl_win32.cpp
    ${CUSTOM_IMGUI_IMANIM_DIR}/im_anim.cpp
)

target_include_directories(custom_imgui PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CUSTOM_IMGUI_IMGUI_DIR}
    ${CUSTOM_IMGUI_IMGUI_DIR}/backends
    ${CUSTOM_IMGUI_IMGUI_DIR}/misc/cpp
    ${CUSTOM_IMGUI_IMANIM_DIR}
)

target_compile_definitions(custom_imgui PUBLIC
    IMGUI_DEFINE_MATH_OPERATORS
    NOMINMAX
    WIN32_LEAN_AND_MEAN
)

target_link_libraries(custom_imgui PUBLIC
    d3d11
    d3dcompiler
    dxgi
    dwmapi
)

add_library(CustomImGui::custom_imgui ALIAS custom_imgui)
add_library(imgui ALIAS custom_imgui)

if(CUSTOM_IMGUI_BUILD_IMANIM_DEMOS)
    target_sources(custom_imgui PRIVATE
        ${CUSTOM_IMGUI_IMANIM_DIR}/im_anim_demo.cpp
        ${CUSTOM_IMGUI_IMANIM_DIR}/im_anim_doc.cpp
        ${CUSTOM_IMGUI_IMANIM_DIR}/im_anim_usecase.cpp
    )
    target_compile_definitions(custom_imgui PUBLIC CUSTOM_IMGUI_HAS_IMANIM_DEMOS=1)
endif()

if(CUSTOM_IMGUI_BUILD_TESTS)
    add_executable(custom_imgui_dx11_test WIN32
        examples/win32_dx11/main.cpp
    )
    target_link_libraries(custom_imgui_dx11_test PRIVATE CustomImGui::custom_imgui)
endif()
