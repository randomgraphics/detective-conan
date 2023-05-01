set(sources
    detective-conan.cpp
)

add_library(detective-conan ${sources})

target_include_directories(detective-conan PUBLIC ../inc)