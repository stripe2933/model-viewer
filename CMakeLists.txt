cmake_minimum_required(VERSION 3.24)

project(model-viewer
    LANGUAGES CXX
    VERSION 0.1.0
)

find_package(Boost COMPONENTS container container_hash iostreams REQUIRED)
find_package(fastgltf CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)

add_executable(model-viewer WIN32 MACOSX_BUNDLE)
target_compile_features(model-viewer PRIVATE cxx_std_23)
target_link_libraries(model-viewer PRIVATE
    Boost::container
    Boost::container_hash
    Boost::iostreams
    fastgltf::fastgltf
    imgui::imgui
    glm::glm
)
target_compile_definitions(model-viewer PRIVATE
    GLM_FORCE_XYZW_ONLY
    GLM_FORCE_DEPTH_ZERO_TO_ONE
)

if (APPLE)
    enable_language(OBJCXX)
    target_sources(model-viewer PRIVATE
        main.mm
        metal/Renderer.mm
    )
    target_link_libraries(model-viewer PRIVATE
        "-framework AppKit"
        "-framework Foundation"
        "-framework GameController"
        "-framework Metal"
        "-framework MetalKit"
        "-framework QuartzCore"
        "-framework UniformTypeIdentifiers"
    )

    set_target_properties(model-viewer PROPERTIES
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.leekyoungheon.${PROJECT_NAME}"
        MACOSX_BUNDLE_BUNDLE_NAME "${PROJECT_NAME}"
        MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
        MACOSX_BUNDLE_COPYRIGHT "Copyright (c) 2026 LEE KYOUNGHEON. All rights reserved."
        MACOSX_BUNDLE_GUI_IDENTIFIER "Model Viewer"
        MACOSX_BUNDLE_LONG_VERSION_STRING "${PROJECT_VERSION}"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}"
    )
else()
    find_package(glfw3 CONFIG REQUIRED)
    find_package(vku CONFIG REQUIRED)

    target_sources(model-viewer PRIVATE
        main.cpp
        impl.cpp
        vulkan/Gpu.cpp
        vulkan/Model.cpp
        vulkan/Swapchain.cpp
    )
    target_precompile_headers(model-viewer PRIVATE pch.hpp)
    target_link_libraries(model-viewer PRIVATE
        glfw
        vku::vku
    )
    target_compile_definitions(model-viewer PRIVATE
        GLFW_INCLUDE_VULKAN
        VULKAN_HPP_NO_SMART_HANDLE
        VULKAN_HPP_NO_TO_STRING
    )

    set_source_files_properties(impl.cpp PROPERTIES SKIP_PRECOMPILE_HEADERS ON)
endif()
