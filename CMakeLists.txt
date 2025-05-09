cmake_minimum_required(VERSION 3.19.0)

project(wincrowd-audio_session_controller)

add_subdirectory(external)

# library
add_library(audio_session_controller STATIC
    include/wincrowd/audio_session_controller/audio_session_controller.h
    source/audio_session_controller.cpp
)

target_include_directories(audio_session_controller
    PUBLIC
        include
    PRIVATE
        source
)

target_compile_features(audio_session_controller 
    PRIVATE 
        cxx_std_17
)

# executable
add_executable(audio_session_controller-app WIN32
    source/main.cpp
)

set_property(TARGET audio_session_controller PROPERTY COMPILE_FLAGS "/DUNICODE /D_UNICODE")
set_property(TARGET audio_session_controller-app PROPERTY COMPILE_FLAGS "/DUNICODE /D_UNICODE")

target_link_libraries(audio_session_controller-app
    PRIVATE
        audio_session_controller
)

target_compile_features(audio_session_controller-app
    PRIVATE 
        cxx_std_17
)
