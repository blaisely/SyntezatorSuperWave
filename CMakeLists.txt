cmake_minimum_required(VERSION 3.15)

project(SuperSawSynth VERSION 0.5)
# Add JUCE subdirectory
add_subdirectory(JUCE)

# Plugin project configuration
juce_add_plugin(SuperSawSynth
        COMPANY_NAME "blaisely"
        IS_SYNTH TRUE
        NEEDS_MIDI_INPUT TRUE
        NEEDS_MIDI_OUTPUT TRUE
        IS_MIDI_EFFECT FALSE
        EDITOR_WANTS_KEYBOARD_FOCUS TRUE
        COPY_PLUGIN_AFTER_BUILD TRUE
        PLUGIN_MANUFACTURER_CODE Blas
        PLUGIN_CODE Ssaw
        FORMATS VST3 Standalone
        PRODUCT_NAME "SuperSawSynthesizer")

target_include_directories(SuperSawSynth PRIVATE JuceLibraryCode)

file(GLOB_RECURSE SOURCE_FILES
        ${CMAKE_SOURCE_DIR}/Source/*.cpp
        ${CMAKE_SOURCE_DIR}/Source/*.h
)
file(GLOB_RECURSE JUCE_FILES
        ${CMAKE_SOURCE_DIR}/JuceLibraryCode/*.cpp
        ${CMAKE_SOURCE_DIR}/JuceLibraryCode/*.h
)
target_sources(SuperSawSynth PRIVATE
        ${SOURCE_FILES}
)
source_group(TREE ${CMAKE_SOURCE_DIR}/Source PREFIX "Source Files" FILES
        ${SOURCE_FILES}
)
juce_add_binary_data(BinaryData SOURCES
                  Source/graphics/SUPERWAVE_logo.png
)
target_compile_definitions(SuperSawSynth
        PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        CMAKE_BUILD_TYPE="${CMAKE_BUILD_TYPE}"
        JUCE_VST3_CAN_REPLACE_VST2=0)

target_link_libraries(SuperSawSynth
        PRIVATE
        BinaryData
        juce::juce_gui_extra
        juce::juce_audio_basics
        juce::juce_audio_devices
        juce::juce_audio_formats
        juce::juce_audio_processors
        juce::juce_audio_utils
        juce::juce_core
        juce::juce_data_structures
        juce::juce_events
        juce::juce_graphics
        juce::juce_gui_basics
        juce::juce_audio_plugin_client
        juce::juce_dsp
        PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)

set_target_properties(SuperSawSynth PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO
)

