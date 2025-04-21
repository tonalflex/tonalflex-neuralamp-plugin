cmake_minimum_required(VERSION 3.22.1)

project(TonalFlexNeuralAmp VERSION 0.0.1)

set(INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")

option(BUILD_DESKTOP "Also build Standalone" OFF) # cmake -B build -S . -DBUILD_DESKTOP=ON
set(_plugin_formats "VST3") # Default plugin formats

if (BUILD_DESKTOP)
    # Add Standalone format for desktop test builds
    list(APPEND _plugin_formats "Standalone")
    set(VST3_MANIFEST TRUE) # Create moduleinfo.json for VST3
else()
    # Rely on installed JUCE inside x-compiler container
    find_package(JUCE CONFIG REQUIRED)
    # Disable LTO (Link-Time Optimization)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION OFF)
    # Disable VST3_AUTO_MANIFEST (Required for ElkOS cross-compilation)
    set(VST3_MANIFEST FALSE) 
endif()

# Add NeuralAmpModelerCore subdirectory
add_subdirectory(NeuralAmpModelerCore)

juce_add_plugin(${PROJECT_NAME}
    COMPANY_NAME TonalFlex
    PLUGIN_NAME "TonalFlexNeuralAmp"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    PLUGIN_MANUFACTURER_CODE MCMP
    PLUGIN_CODE EXPL
    FORMATS ${_plugin_formats}
    PRODUCT_NAME "TonalFlexNeuralAmp"
    VST3_AUTO_MANIFEST ${VST3_MANIFEST}
)

target_sources(${PROJECT_NAME}
    PRIVATE
        src/processor.cpp
        src/editor.cpp
        ${PROTO_FILES}
        ${INCLUDE_DIR}/editor.h
        ${INCLUDE_DIR}/processor.h
)

target_include_directories(NAM PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/NeuralAmpModelerCore"
    "${CMAKE_CURRENT_SOURCE_DIR}/NeuralAmpModelerCore/Dependencies/eigen"
    "${CMAKE_CURRENT_SOURCE_DIR}/NeuralAmpModelerCore/Dependencies/nlohmann"
)

target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${INCLUDE_DIR}
        ${GENERATED_PROTO_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/NeuralAmpModelerCore
        ${CMAKE_CURRENT_SOURCE_DIR}/NeuralAmpModelerCore/Dependencies/eigen
        ${CMAKE_CURRENT_SOURCE_DIR}/NeuralAmpModelerCore/Dependencies/nlohmann
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        juce::juce_core
        juce::juce_dsp
        juce::juce_events
        juce::juce_audio_utils
        juce::juce_audio_devices
        juce::juce_audio_processors
        juce::juce_gui_basics
        juce::juce_gui_extra
        stdc++fs  # For GCC with libstdc++ / comment out on macOS
        NAM
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

target_compile_definitions(${PROJECT_NAME}
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif()

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/..)