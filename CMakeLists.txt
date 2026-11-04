cmake_minimum_required(VERSION 3.22)
project(KWBuilderV4 VERSION 0.4.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/JUCE/CMakeLists.txt")
 message(FATAL_ERROR "Clone JUCE into JUCE/ first. See README.md.")
endif()
add_subdirectory(JUCE)
juce_add_plugin(KWBuilderV4 COMPANY_NAME "KWWorkbench" BUNDLE_ID "org.kwworkbench.builder.v4"
 PLUGIN_MANUFACTURER_CODE KwWb PLUGIN_CODE KwB4 FORMATS VST3 Standalone
 PRODUCT_NAME "KW Builder V4 PreCut PostTilt" IS_SYNTH FALSE NEEDS_MIDI_INPUT FALSE NEEDS_MIDI_OUTPUT FALSE
 IS_MIDI_EFFECT FALSE COPY_PLUGIN_AFTER_BUILD FALSE)
juce_generate_juce_header(KWBuilderV4)
target_sources(KWBuilderV4 PRIVATE Source/Plugin.cpp Source/DSP.h)
target_compile_definitions(KWBuilderV4 PUBLIC JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0)
target_link_libraries(KWBuilderV4 PRIVATE juce::juce_audio_utils juce::juce_dsp
 PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_warning_flags)
enable_testing()
add_executable(KWCoreTests tests/tests.cpp)
add_test(NAME KWCoreTests COMMAND KWCoreTests)
