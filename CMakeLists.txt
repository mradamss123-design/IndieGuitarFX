cmake_minimum_required(VERSION 3.22)
project(IndieGuitarFX VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ================================================================
# JUCE Detection — tries local paths first, then downloads via git
# ================================================================
set(JUCE_SEARCH_PATHS
    "C:/JUCE"
    "C:/Program Files/JUCE"
    "$ENV{USERPROFILE}/JUCE"
    "$ENV{USERPROFILE}/Documents/JUCE"
    "C:/SDKs/JUCE"
    "${CMAKE_CURRENT_SOURCE_DIR}/../JUCE"
    "${CMAKE_CURRENT_SOURCE_DIR}/JUCE"
)

set(JUCE_FOUND_PATH "")
foreach(p IN LISTS JUCE_SEARCH_PATHS)
    if(EXISTS "${p}/CMakeLists.txt")
        set(JUCE_FOUND_PATH "${p}")
        break()
    endif()
endforeach()

if(JUCE_FOUND_PATH)
    message(STATUS "JUCE found at: ${JUCE_FOUND_PATH}")
    add_subdirectory("${JUCE_FOUND_PATH}" "${CMAKE_BINARY_DIR}/juce_build" EXCLUDE_FROM_ALL)
else()
    message(STATUS "JUCE not found locally — downloading via FetchContent (requires internet)...")
    include(FetchContent)
    FetchContent_Declare(
        JUCE
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG        7.0.9
        GIT_SHALLOW    TRUE
        GIT_PROGRESS   TRUE
    )
    FetchContent_MakeAvailable(JUCE)
endif()

# ================================================================
# Plugin target
# ================================================================
juce_add_plugin(IndieGuitarFX
    COMPANY_NAME                "IndieStudio"
    PLUGIN_MANUFACTURER_CODE    "IStu"
    PLUGIN_CODE                 "IGFX"
    FORMATS                     VST3 Standalone
    PRODUCT_NAME                "IndieGuitarFX"
    IS_SYNTH                    FALSE
    NEEDS_MIDI_INPUT            FALSE
    NEEDS_MIDI_OUTPUT           FALSE
    IS_MIDI_EFFECT              FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD     FALSE
    VST3_CATEGORIES             "Fx" "Distortion" "Guitar"
    PLUGIN_DESCRIPTION          "Guitar FX processor for indie rock and indie pop"
)

# Генерирует JuceHeader.h — обязательно для CMake-сборки JUCE
juce_generate_juce_header(IndieGuitarFX)

target_sources(IndieGuitarFX PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/DSP/Overdrive.cpp
    Source/DSP/Chorus.cpp
    Source/DSP/Reverb.cpp
    Source/DSP/Delay.cpp
    Source/DSP/Compressor.cpp
    Source/DSP/EQ.cpp
    Source/DSP/ToneAnalyzer.cpp
    Source/GUI/KnobLookAndFeel.cpp
    Source/GUI/EffectStrip.cpp
    Source/GUI/ToneAnalyzerPanel.cpp
    Source/GUI/SpectrumAnalyzer.cpp
)

target_compile_definitions(IndieGuitarFX PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
    JUCE_MODAL_LOOPS_PERMITTED=1
)

target_link_libraries(IndieGuitarFX
    PRIVATE
        juce::juce_audio_utils
        juce::juce_audio_processors
        juce::juce_dsp
        juce::juce_graphics
        juce::juce_gui_basics
        juce::juce_gui_extra
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

# ================================================================
# Print install path after build
# ================================================================
add_custom_command(TARGET IndieGuitarFX_VST3 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND ${CMAKE_COMMAND} -E echo "=== BUILD COMPLETE ==="
    COMMAND ${CMAKE_COMMAND} -E echo "VST3 location:"
    COMMAND ${CMAKE_COMMAND} -E echo "$<TARGET_FILE_DIR:IndieGuitarFX_VST3>"
    COMMAND ${CMAKE_COMMAND} -E echo "Copy to: C:/Program Files/Common Files/VST3/"
    COMMAND ${CMAKE_COMMAND} -E echo ""
)
